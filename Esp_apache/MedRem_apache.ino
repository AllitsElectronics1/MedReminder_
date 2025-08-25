
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// Pin Definitions
const int morningLED = 4;         // GPIO pin for morning LED
const int afternoonLED = 5;      // GPIO pin for afternoon LED
const int eveningLED = 14;        // GPIO pin for evening LED
const int buzzerPin = 15;         // GPIO pin for buzzer
const int switchPin = 12;      // GPIO pin for lid switch (interrupt)

String macAddress;  // Store the MAC address globally
String message; 

const char* serverUrl = "http://192.168.1.7/esp/register_device.php";

// OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String scrollMessage = "";    // Message to scroll
int scrollPosition = SCREEN_WIDTH; // Initial position
bool isScrolling = false;    // Flag to control scrolling
unsigned long lastScrollTime = 0; // For timing the scroll updates
const unsigned long scrollInterval = 10; 

int textWidth;
const int textSize = 4;

// HTTP Server
WebServer server(80);

// Flags and Timers
const int ThresholdTime = 60000; // 15min
bool randomTimerStarted = false; // flag to indicate if the random timer has started
unsigned long randomTimerDuration = 0; // Holds the random timer value (1-10 seconds in ms)
unsigned long randomTimerStartTime = 0;
bool TimerStarted = false;
unsigned long TimerStartTime = 0; //  time when switch is pressed
bool SwitchPressed = false; //Flag to indicate status of the switch
bool LidOpened = false;    // Time when lid was opened

bool LidMessagePrinted = false;
bool TimerMessagePrinted = false;
bool alertStarted = false;

bool morningActive = false;
bool afternoonActive = false;
bool eveningActive = false;


// Functions
void IRAM_ATTR isr() {
  // Lid opened
  int switchState = digitalRead(switchPin);
  Serial.print("Interrupt triggered, switch state: ");
  Serial.println(switchState);
  SwitchPressed = true;
  LidOpened = true;
  randomTimerStarted = true;
  randomTimerDuration = random(1000, 10000); // Random timer (1-10 seconds)
  randomTimerStartTime = millis();
  Serial.println("Lid opened, starting random timer.");
  Serial.print(randomTimerDuration / 1000); // Display in seconds
  Serial.println(" seconds");   
}

void displayMessage(String message) {
  scrollMessage = message; // Set the message
    scrollPosition = SCREEN_WIDTH; // Reset position
    isScrolling = true; 
}

void handleScrolling() {
  if (isScrolling && (millis() - lastScrollTime >= scrollInterval)) {
    lastScrollTime = millis(); // Update the last scroll time

    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(scrollPosition, 0);
    display.print(scrollMessage);
    display.display();

    scrollPosition--; // Move text left

   // Reset position if the message has scrolled off the screen
   int textWidth = textSize * scrollMessage.length() * 6;
   if (scrollPosition < -textWidth) {
     scrollPosition = SCREEN_WIDTH;
    }
  }
}


void startAlerts(String message, int ledPin) {
  TimerStartTime = millis();       // Record the start time of the alert
  TimerStarted = true; // Set alert as active     
  digitalWrite(ledPin, HIGH);        // Turn on the LED, buzzer, and display the message
  tone(buzzerPin, 1000);           // Turn on buzzer with 1 kHz tone
  displayMessage(message);
  Serial.println("Alert started: " + message);
}

void stopAlerts(int ledPin) {
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);           // Turn off the buzzer
  display.clearDisplay();
  display.display();      // Reset alert state
  TimerStarted = false;
  LidOpened = false; 
  isScrolling = false; 
  if (ledPin == morningLED) morningActive = false;
  if (ledPin == afternoonLED) afternoonActive = false;
  if (ledPin == eveningLED) eveningActive = false;

  Serial.println("Alert stopped.");
}

//Function to monitor the lid status
void checkAlertStatus(int ledPin) {

  if ((millis() - TimerStartTime) < ThresholdTime)
  {
    if (LidOpened) // Lid is currently open
    { 
      if (randomTimerStarted && (millis() - randomTimerStartTime) <= randomTimerDuration) // Lid closed within random timer
      {
       if (digitalRead(switchPin) == LOW) { // Check if lid is closed
        Serial.println("Lid closed within random timer (Medicine NOT taken)");
        sendMedicineStatus(macAddress,message,"not_taken");
        stopAlerts(ledPin);
        LidOpened = false;
        randomTimerStarted = false; // Reset random timer
        return;
      }
    } 
    
    else if (randomTimerStarted && (millis() - randomTimerStartTime) > randomTimerDuration) // Lid closed after random timer
    {
      if (digitalRead(switchPin) == LOW) // Check if lid is closed
      { 
        Serial.println("Lid closed after random timer (Medicine TAKEN)");
        sendMedicineStatus(macAddress,message,"taken");
        stopAlerts(ledPin);
        randomTimerStarted = false; // Reset random timer
        LidOpened = false;
        return;
      }
    }
  }
 } 
 
  else if ((millis() - TimerStartTime) > ThresholdTime && !LidOpened )// Threshold time expired, no lid activity
  {
    Serial.println("Threshold time expired, turning alerts back on.");
    sendMedicineStatus( macAddress,message,"not_taken");
    startAlerts(message,ledPin); 
    delay(3000);
    stopAlerts(ledPin);
    return;
  } 
  else if ((millis() - TimerStartTime) > ThresholdTime && LidOpened && (!LidMessagePrinted)) // Lid left open beyond threshold time
  {
    LidMessagePrinted = true;
    Serial.println("Lid still open after threshold time.");
    display.clearDisplay();
    displayMessage("Lid still open");
    sendMedicineStatus( macAddress, message,"taken");
    delay(3000);
    stopAlerts(ledPin);
  }
}

void sendMedicineStatus(String macAddress,String message,String status) {
  HTTPClient http;
  http.begin(serverUrl); // Update your server URL
  http.addHeader("Content-Type", "application/json");

  // Prepare JSON payload
  String payload = "{\"mac\":\"" + macAddress + "\",\"medicine\":\"" + message + "\",\"status\":\"" + status + "\"}";
  Serial.println("Sending Status Payload: " + payload);

  // Send the POST request
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.println("Error sending status. HTTP Response code: " + String(httpResponseCode));
  }

  http.end();  // Close the connection
}


//Function to Register mac address with its respective ip 
bool registerDevice(String macAddress, String ipAddress) {
  HTTPClient http;
  http.begin(serverUrl); // Initialize HTTP client with server URL
  http.addHeader("Content-Type", "application/json"); // Set content type

  // Create JSON payload
  String payload = "{\"mac\":\"" + macAddress + "\",\"ip\":\"" + ipAddress + "\"}";
  Serial.println("Payload: " + payload); // Debug: print payload

  // Send POST request
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString(); // Read server response
    Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
    Serial.println("Response: " + response);
    http.end();
    return httpResponseCode == 200; // Return true only for HTTP 200
  } else {
    Serial.printf("Failed to send HTTP POST. Response code: %d\n", httpResponseCode);
    http.end();
    return false;
  }
}

//Function to handle the messges from website
void handleNotify() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Received payload: " + body);

    StaticJsonDocument<200> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, body);

    if (!error) {
      message = jsonDoc["message"].as<String>();
      String label = jsonDoc["label"].as<String>();
      Serial.println("Parsed message: " + message);
      Serial.println("Parsed label: " + label);

      // Start alerts based on the label
      if (label == "morning") {
       startAlerts(message, morningLED);  // Directly call checkAlertStatus
       morningActive = true;
      } else if (label == "afternoon") {
       startAlerts(message, afternoonLED);
        afternoonActive = true;
      } else if (label == "evening") {
       startAlerts(message, eveningLED);
        eveningActive = true; 
      }else {
        Serial.println("Unknown label received: " + label);
      }
    } else {
      Serial.println("Failed to parse JSON");
    }

    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No payload received\"}");
  }
}

void setup() {
  Serial.begin(115200);

  // GPIO setup
  pinMode(morningLED, OUTPUT);
  pinMode(afternoonLED, OUTPUT);
  pinMode(eveningLED, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(switchPin, INPUT); 

  attachInterrupt(switchPin, isr, RISING);

  // OLED initialization
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  } 
  display.clearDisplay();
  display.setTextSize(textSize);       // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color to white
  display.setCursor(0, 0); 
  delay(1000);                        
 

  // WiFiManager setup
  WiFiManager wm;
  Serial.println("Starting WiFiManager...");
  if (!wm.autoConnect("ESP32_AP")) {
    Serial.println("Failed to connect to Wi-Fi. Restarting...");
    ESP.restart();
  }

  // Get MAC and IP address
   macAddress = WiFi.macAddress();
  Serial.println("ESP32 MAC Address: " + macAddress);
  String ipAddress = WiFi.localIP().toString();
  Serial.println("ESP32 IP Address: " + ipAddress);

  // Register the device
  if (registerDevice(macAddress, ipAddress)) {
    Serial.println("Device registered successfully!");
  } else {
    Serial.println("Failed to register device.");
  }

  // HTTP Server setup
  server.on("/notify", HTTP_POST, handleNotify);
  server.begin();
  Serial.println("Server started!");
}

void loop() {
  server.handleClient();
  if (TimerStarted){
  if (morningActive)
  {
    checkAlertStatus(morningLED);
  }
  if (afternoonActive)
  {
    checkAlertStatus(afternoonLED);
  }
  if (eveningActive)
  {
    checkAlertStatus(eveningLED);
  }
 }
  handleScrolling();
}
