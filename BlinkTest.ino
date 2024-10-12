#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

// Define pins for the RFID module
#define SS_PIN 21
#define RST_PIN 22

// Define pins for the ultrasonic sensors and buzzers
const int trigPin1 = 15;
const int echoPin1 = 2;
const int buzzerPin1 = 13;

const int trigPin2 = 33;
const int echoPin2 = 32;
const int buzzerPin2 = 14;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// AWS IoT topics
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

// AWS IoT client objects
WiFiClientSecure net;
PubSubClient client(net);

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(String tagUID, float distance1, float distance2) {
  StaticJsonDocument<200> doc;
  doc["tagUID"] = tagUID;
  doc["distance1"] = distance1;
  doc["distance2"] = distance2;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);  // Initialize serial communications
  while (!Serial); // Wait for serial port to be available
  Serial.println("Starting...");

  // Initialize SPI bus
  Serial.println("Initializing SPI...");
  SPI.begin();  
  Serial.println("SPI Initialized");

  // Initialize MFRC522
  Serial.println("Initializing RFID reader...");
  mfrc522.PCD_Init();
  Serial.println("RFID Reader is ready. Scan a tag...");

  // Initialize pins for ultrasonic sensors and buzzers
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(buzzerPin1, OUTPUT);
  digitalWrite(buzzerPin1, HIGH); // Ensure buzzer is off initially

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(buzzerPin2, OUTPUT);
  digitalWrite(buzzerPin2, HIGH); // Ensure buzzer is off initially

  // Connect to AWS
  connectAWS();
}

void loop() {
  // Separate the RFID and ultrasonic checks for clarity
  String tagUID = checkRFID();
  float distance1 = checkUltrasonic(trigPin1, echoPin1, buzzerPin1);
  float distance2 = checkUltrasonic(trigPin2, echoPin2, buzzerPin2);

  // Publish sensor data to AWS IoT
  publishMessage(tagUID, distance1, distance2);

  client.loop();
  delay(1000); // Delay between measurements
}

float checkUltrasonic(int trigPin, int echoPin, int buzzerPin) {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Set the trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin, pulseIn() returns the duration (length of the pulse) in microseconds
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  float distance = duration * 0.0344 / 2;

  // Print the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check buzzer behavior
  if (distance < 5) {
    Serial.println("Object detected, buzzer ON\n");
    digitalWrite(buzzerPin, LOW); // Turn on the buzzer
  } else {
    Serial.println("No object detected, buzzer OFF\n");
    digitalWrite(buzzerPin, HIGH); // Turn off the buzzer
  }

  return distance;
}

String checkRFID() {
  String tagUID = "No Tag";
  
  // Check if a new tag has been detected
  if (!mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("No card present.");
    return tagUID;
  }

  // Select the scanned tag
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Error reading card.");
    return tagUID;
  }

  // Print UID of the detected tag
  Serial.print("Tag UID: ");
  tagUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    tagUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") + String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Check the tag type
  if (tagUID.equalsIgnoreCase("03BD4414")) {
    Serial.println("Recyclable tag detected.\n");
  } else if (tagUID.equalsIgnoreCase("8318E70C")) {
    Serial.println("Non-recyclable tag detected.\n");
  } else {
    Serial.println("Unknown tag.");
  }

  // Halt PICC
  mfrc522.PICC_HaltA();
  
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  return tagUID;
}
