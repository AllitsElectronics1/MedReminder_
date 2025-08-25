#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// Pin Definitions
const int morningLED = 4;
const int afternoonLED = 5;
const int eveningLED = 15;
const int buzzerPin = 14;
const int switchPin = 12;

String macAddress;
String message;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String scrollMessage = "";
int scrollPosition = SCREEN_WIDTH;
bool isScrolling = false;
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 10;

int textSize = 4;

WebServer server(80);

const int ThresholdTime = 60000;
bool randomTimerStarted = false;
unsigned long randomTimerDuration = 0;
unsigned long randomTimerStartTime = 0;
bool TimerStarted = false;
unsigned long TimerStartTime = 0;
bool SwitchPressed = false;
bool LidOpened = false;
bool LidMessagePrinted = false;
bool TimerMessagePrinted = false;
bool alertStarted = false;
bool morningActive = false;
bool afternoonActive = false;
bool eveningActive = false;

unsigned long lastPollTime = 0;
unsigned long pollInterval = 3600000; // default 1 hour

void IRAM_ATTR isr() {
  SwitchPressed = true;
  LidOpened = true;
  randomTimerStarted = true;
  randomTimerDuration = random(1000, 10000);
  randomTimerStartTime = millis();
}

void displayMessage(String message) {
  scrollMessage = message;
  scrollPosition = SCREEN_WIDTH;
  isScrolling = true;
}

void handleScrolling() {
  if (isScrolling && ((millis() - lastScrollTime >= 50))) {
    lastScrollTime = millis();
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(scrollPosition, 0);
    display.print(scrollMessage);
    display.display();
    scrollPosition--;
    int textWidth = textSize * scrollMessage.length() * 6;
    if (scrollPosition < -textWidth) {
      scrollPosition = SCREEN_WIDTH;
    }
  }
}

void startAlerts(String message, int ledPin) {
  TimerStartTime = millis();
  TimerStarted = true;
  digitalWrite(ledPin, HIGH);
  tone(buzzerPin, 1000);
  displayMessage(message);
}

void stopAlerts(int ledPin) {
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);
  display.clearDisplay();
  display.display();
  TimerStarted = false;
  LidOpened = false;
  isScrolling = false;
  if (ledPin == morningLED) morningActive = false;
  if (ledPin == afternoonLED) afternoonActive = false;
  if (ledPin == eveningLED) eveningActive = false;
  Serial.println("Alert stopped.");
}

void checkAlertStatus(int ledPin) {
  if ((millis() - TimerStartTime) < ThresholdTime) {
    if (LidOpened) {
      if (randomTimerStarted && (millis() - randomTimerStartTime) <= randomTimerDuration) {
        if (digitalRead(switchPin) == LOW) {
          sendMedicineStatus(macAddress, message, "not_taken");
          stopAlerts(ledPin);
          LidOpened = false;
          randomTimerStarted = false;
          return;
        }
      } else if (randomTimerStarted && (millis() - randomTimerStartTime) > randomTimerDuration) {
        if (digitalRead(switchPin) == LOW) {
          sendMedicineStatus(macAddress, message, "taken");
          stopAlerts(ledPin);
          randomTimerStarted = false;
          LidOpened = false;
          return;
        }
      }
    }
  } else if ((millis() - TimerStartTime) > ThresholdTime && !LidOpened) {
    sendMedicineStatus(macAddress, message, "not_taken");
    startAlerts(message, ledPin);
    delay(3000);
    stopAlerts(ledPin);
  } else if ((millis() - TimerStartTime) > ThresholdTime && LidOpened && (!LidMessagePrinted)) {
    LidMessagePrinted = true;
    display.clearDisplay();
    displayMessage("Lid still open");
    sendMedicineStatus(macAddress, message, "taken");
    delay(3000);
    stopAlerts(ledPin);
  }
}

void sendMedicineStatus(String macAddress, String message, String status) {
  HTTPClient http;
  http.begin("http://tesstmedreminder.ap-south-1.elasticbeanstalk.com:8080/api/devices/updateMedicineStatus");
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"macAddress\":\"" + macAddress + "\",\"medicineName\":\"" + message + "\",\"status\":\"" + status + "\"}";
  Serial.println("Sending Status Payload: " + payload);
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    Serial.println("Response: " + http.getString());
  } else {
    Serial.println("Error sending status. HTTP Response code: " + String(httpResponseCode));
  }
  http.end();
}

bool registerDevice(String macAddress, String ipAddress) {
  HTTPClient http;
  http.begin("http://tesstmedreminder.ap-south-1.elasticbeanstalk.com:8080/api/devices/update-ip");
  http.addHeader("Content-Type", "application/json");
  String payload = "{\"macAddress\":\"" + macAddress + "\",\"ipAddress\":\"" + ipAddress + "\"}";
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    Serial.println("Response: " + http.getString());
    http.end();
    return httpResponseCode == 200;
  } else {
    Serial.println("Failed to send HTTP POST. Response code: " + String(httpResponseCode));
    http.end();
    return false;
  }
}

bool pollServerForSchedule() {
  HTTPClient http;

    String url = "http://192.168.1.11:8080/api/medicines/check-schedule?macAddress=" + macAddress;
  Serial.println("Polling URL: " + url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Poll response: " + response);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      bool scheduled = doc["scheduled"];
      int nextMinutes = doc["minutesUntilNextSchedule"];
      String label = doc["label"] | "";
      message = doc["message"] | "";

      if (scheduled && label != "") {
        label.toLowerCase();
        if (label == "morning") {
          startAlerts(message, morningLED);
          morningActive = true;
        } else if (label == "afternoon") {
          startAlerts(message, afternoonLED);
          afternoonActive = true;
        } else if (label == "evening") {
          startAlerts(message, eveningLED);
          eveningActive = true;
        }
        http.end();
        return true; // Reminder started
      } else {
        if (nextMinutes == 0) {
          nextMinutes = 60; // Default fallback
        }
        Serial.printf("No reminder now. Sleeping for %d minutes...\n", nextMinutes);
        uint64_t sleepTimeUs = (uint64_t)nextMinutes * 60ULL * 1000000ULL;
        http.end();
        esp_sleep_enable_timer_wakeup(sleepTimeUs);
        esp_deep_sleep_start();
        return false;
      }
    } else {
      Serial.println("Failed to parse JSON");
    }
  } else {
    Serial.printf("HTTP Poll failed. Code: %d\n", httpCode);
  }

  http.end();
  return false;
}


void setup() {
  Serial.begin(115200);
  delay(1000);

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
    while (1); // Halt if OLED fails
  }
  display.clearDisplay();
  display.display();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  delay(500);

  // WiFiManager setup
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  Serial.println("Starting WiFiManager...");
  if (!wm.autoConnect("ESP32_AP", "password123")) {
    Serial.println("Failed to connect to Wi-Fi. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Wi-Fi connected!");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  // Get and print MAC address
  macAddress = WiFi.macAddress();
  Serial.println("ESP32 MAC Address: " + macAddress);
  String ipAddress = WiFi.localIP().toString();

  // Register device with server
  if (registerDevice(macAddress, ipAddress)) {
    Serial.println("Device registered successfully!");
  } else {
    Serial.println("Failed to register device.");
  }

  // Poll server once to check for schedule
  if (!pollServerForSchedule()) {
    // If no schedule found, device will already enter deep sleep from inside pollServerForSchedule
    // So nothing more to do here
    return;
  }
}


void loop() {
  server.handleClient();
  if (TimerStarted) {
    if (morningActive) checkAlertStatus(morningLED);
    if (afternoonActive) checkAlertStatus(afternoonLED);
    if (eveningActive) checkAlertStatus(eveningLED);
  }
  handleScrolling();
  if (millis() - lastPollTime >= pollInterval) {
    pollServerForSchedule();
  }
}
