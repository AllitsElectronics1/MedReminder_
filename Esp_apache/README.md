Overview

 ESP32, LEDs, buzzer, OLED display, a web interface, and a PHP backend.

Users can schedule medicine notifications via a web interface, and the ESP32 alerts the user with LED, buzzer, and display. The system also monitors if the medicine has been taken using a lid switch.

Components

Apache Server – Hosts PHP backend and website.

Web Interface – HTML forms to send medicine schedules to ESP32.

How It Works
1. Web Interface (MedRem.html)
Users enter medicine names for morning, afternoon, and evening.

Each form sends a POST request to send-medicine.php with medicine name and timing.

2. PHP Backend (send_medicine.php)

send-medicine.php receives medicine names and timings.

Stores the data in a file (medicine.txt) or sends it via HTTP/JSON to the ESP32.

3. ESP32

Connects to Wi-Fi via WiFiManager.

Registers itself to the server with MAC and IP address.

Runs an HTTP server to receive notifications from the web interface.

On receiving a notification:

Turns on the corresponding LED.

Sounds the buzzer.

Displays the medicine name on OLED.

Monitors lid switch to detect if medicine is taken:

Lid closed within random timer → "NOT taken" reported.

Lid closed after timer → "TAKEN" reported.

Threshold time ensures alert repeats if medicine not taken.

Example Workflow

User opens the web page.

Enters “Paracetamol” for morning and submits.

ESP32 receives the message via POST /notify.

ESP32 alerts the user:

Morning LED turns on.

Buzzer sounds.

OLED displays “Paracetamol”.

User opens the lid to take medicine. ESP32 monitors the lid and updates the server with status (taken or not_taken).

Example JSON Payload Sent to ESP32
{
  "message": "Paracetamol",
  "label": "morning"
}
