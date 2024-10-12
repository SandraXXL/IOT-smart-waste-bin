# Smart Waste Bin using IoT and AWS

## Required Components

### Hardware

<img width="778" alt="hardware" src="https://github.com/user-attachments/assets/4baaf9ce-e58a-46d5-a19e-7177c55162a8">


ESP32 CP2012 *1

HC-SRO4 Ultrasonic Sensor *2

Buzzer *2

RFID Reader RC522

RFID Tags (13.56MHz) *3

Breadboard

Jumper Wires

### Software

AWS IoT Core

AWS IoT Analytics

Amazon QuickSight

### Hardware Connection

<img width="648" alt="connection" src="https://github.com/user-attachments/assets/c42623d6-ff59-4da7-a156-a91cf254238c">

### Software Intgeration

AWS IoT Core Configuration: 

The ESP32 is programmed to connect to AWS IoT Core using MQTT protocol. Device certificates and endpoint configurations should be set up to enable secure communication.

AWS IoT Analytics Setup: 

Data from AWS IoT Core is routed to AWS IoT Analytics, where it is processed and analyzed. Data streams should be configured to handle sensor data and waste classification information.

Amazon QuickSight Visualization:

AWS IoT Analytics data is visualized using Amazon QuickSight. Dashboards should be created to display waste management metrics, trends, and insights.

## Main Functions

1. Waste type classification
   
2. Waste level detection (when bin is full, alarm rings)

## Workflow Diagram

<img width="780" alt="workflow" src="https://github.com/user-attachments/assets/141c23e5-9f0b-4cb9-9e51-59899c0fc91b">







