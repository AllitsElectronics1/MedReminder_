#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "HOME";
const char* password = "suni@1704";

// Replace with your server's IP address and message file path
const char* serverUrl = "http://192.168.0.105/medicine.txt";  // Update to your server's IP

// Define GPIO pins for each LED
const int morningLED = 4;    // Replace with actual GPIO pin for morning LED
const int afternoonLED = 5;  // Replace with actual GPIO pin for afternoon LED
const int eveningLED = 14;    // Replace with actual GPIO pin for evening LED

void setup() {
  Serial.begin(115200);
  
  // Initialize the LED pins as outputs
  pinMode(morningLED, OUTPUT);
  pinMode(afternoonLED, OUTPUT);  
  pinMode(eveningLED, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Optional: Print the ESP32 IP address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {  // Check if Wi-Fi is connected
    HTTPClient http;
    http.begin(serverUrl);  // Initialize HTTP connection to the server

    // Send HTTP GET request
    int httpCode = http.GET();

    // Check for the response status
    if (httpCode > 0) { 
      if (httpCode == HTTP_CODE_OK) {  // If the request is successful
        String payload = http.getString();  // Get the response as a string
        Serial.println("Message from server: " + payload);  // Display the message

        // Check message and activate the corresponding LED
        if (payload.indexOf("morning") >= 0) {
          digitalWrite(morningLED, HIGH);
          delay(5000); // Turn on for 5 seconds
          digitalWrite(morningLED, LOW);
        } else if (payload.indexOf("afternoon") >= 0) {
          digitalWrite(afternoonLED, HIGH);
          delay(5000); // Turn on for 5 seconds
          digitalWrite(afternoonLED, LOW);
        } else if (payload.indexOf("evening") >= 0) {
          digitalWrite(eveningLED, HIGH);
          delay(5000); // Turn on for 5 seconds
          digitalWrite(eveningLED, LOW);
        }
      }
    } else {
      Serial.println("Error in HTTP request");  // Print an error message if the request failed
    }
    
    http.end();  // Close the connection
  } else {
    Serial.println("WiFi Disconnected");
  }
  
  delay(60000);  // Check every 60 seconds
}

