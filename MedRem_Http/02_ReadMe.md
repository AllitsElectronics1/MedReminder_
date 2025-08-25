
Workflow (With Polling)

1.Startup & Initialization

Initialize GPIO pins for morning, afternoon, evening LEDs, buzzer, and lid switch.

Initialize OLED display for scrolling medicine names.

Connect to Wi-Fi using WiFiManager.

Obtain ESP32 MAC & IP address and register with backend server.

2.Polling for Schedule

ESP32 polls the backend server (check-schedule API) at startup and periodically.

Server response includes:

scheduled → whether a medicine reminder is active.

label → timing (morning/afternoon/evening).

message → medicine name.

minutesUntilNextSchedule → used to calculate deep sleep duration if no reminder.

If no schedule, ESP32 enters deep sleep for specified duration.

3.Alert Handling

Activate corresponding LED and buzzer when a scheduled reminder is received.

Scroll medicine name on OLED display.

Alert continues until lid is opened or threshold time (1 minute here) expires.

Random timer (1–10s) distinguishes short vs long lid openings:

Lid closed before timer → not_taken

Lid closed after timer → taken

If lid left open beyond threshold → display “Lid still open” and send status.

4.Lid Monitoring

Lid switch interrupt updates flags (LidOpened, SwitchPressed, randomTimerStarted).

Alert and LED/Buzzer states are updated dynamically.

Status updates sent to backend using sendMedicineStatus.

5.Status Updates

HTTP POST requests send MAC, medicine name, and status (taken / not_taken).

Device IP is updated on server during registration.

6.Continuous Operation

Handle incoming HTTP requests (server.handleClient()).

Monitor lid activity and alert timing.

Update OLED scrolling messages.

Poll server periodically for new schedules.
