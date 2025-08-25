Workflow

1.Startup & Initialization

Initialize GPIO pins for morning, afternoon, evening LEDs, buzzer, and lid switch.

Initialize OLED display for scrolling medicine names.

Connect to Wi-Fi using WiFiManager.

Obtain ESP32 MAC & IP address and register with backend server.

Start HTTP server to receive notifications from web interface.

2.Receiving Reminders

Web interface sends JSON payload to /notify endpoint.

Fields: message (medicine name), label (timing: morning/afternoon/evening).

ESP32 parses JSON and triggers the corresponding alert.

3.Alert Handling

Activate corresponding LED and buzzer.

Scroll medicine name on OLED display.

Alert continues until lid is opened or threshold time (15 minutes) expires.

Random timer (1–10s) distinguishes short vs long lid openings:

Lid closed before timer → not_taken

Lid closed after timer → taken

Lid Monitoring

Lid switch interrupt updates lid status flags.

Alerts, LED, and buzzer are updated in real-time.

Medicine status is sent to the backend server.

4.Status Updates

HTTP POST requests to backend include: MAC address, medicine name, status (taken or not_taken).

Device IP updated on server during registration.

5.Continuous Operation

Handle incoming HTTP requests.

Monitor lid activity and alert timing.

Update OLED scrolling messages.
