Working of the Code (Esp_apache.ino)

This project uses an ESP32 to get medicine reminders from a PHP server and turn on LEDs.

ESP32 connects to Wi-Fi and reads a text file (medicine.txt) from the server.

Depending on the message (morning, afternoon, evening), it lights up the corresponding LED for 5 seconds.

PHP Script (send_medicine.php)
The PHP script allows updating the medicine schedule via a POST request, which updates medicine.txt.

ESP32 checks the server every 60 seconds to display the latest reminder.
