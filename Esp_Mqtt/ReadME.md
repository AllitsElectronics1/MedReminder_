
Working of the Code

 ESP32 with an OLED display, LEDs, a buzzer, and a limit switch to remind users to take medicine.

ESP32 connects to Wi-Fi and subscribes to MQTT topics for morning, afternoon, and evening reminders.

When a message arrives, the ESP32:

Displays the medicine name on the OLED screen.

Turns on the corresponding LED.

Activates the buzzer for 30 seconds.

The limit switch detects if the lid is opened.

If the lid is closed too early, it reports "NOT taken" to MQTT.

If the lid is closed after a random timer but within 1 minute, it reports "TAKEN".

If no action occurs within 15 minutes, it reports "NOT TAKEN" and alerts again.

The ESP32 handles reconnecting to the MQTT broker automatically if disconnected.

Features

OLED scrolling text for medicine names.

LEDs and buzzer as visual and audio alerts.

Lid detection with random and threshold timers.

MQTT communication for real-time updates.
