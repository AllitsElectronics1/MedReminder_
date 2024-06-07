#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

// WiFi credentials
const char* ssid ="ETC_413";
const char* password = "99551122";

// MQTT broker details
const char* mqttServer = "172.19.229.158";
const int mqttPort = 1883;

// MQTT topics
const char* mqttSubtopics[] = {"Morning", "Afternoon", "Evening"};

// Hardware pins
const int ledPins[] = {4, 5, 14}; // Replace with your desired pin numbers
const int buzzerPin = 15;
const int switchPin = 12; // Limit switch pin

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// MQTT client setup
WiFiClient espClient;
PubSubClient client(espClient);

const int ThresholdTime = 60000; //15min
bool randomTimerStarted = false; // To indicate if the random timer has started
unsigned long randomTimerDuration = 0; // Holds the random timer value (1-10 seconds in ms)
unsigned long randomTimerStartTime = 0;
bool TimerStarted  = false;
unsigned long TimerStartTime = 0;
bool SwitchPressed;
bool LidOpened;

bool LidMessagePrinted = false;
bool TimerMessagePrinted = false;

bool buzzerOn = false;
unsigned long buzzerStartTime = 0;
const unsigned long buzzerDuration = 30000;
char lastTopic[32]; 

void IRAM_ATTR isr() {
 SwitchPressed = true;
  LidOpened = true;
  randomTimerStarted = true;
  randomTimerDuration = random(1000, 10000); // Random timer (1-10 seconds)
  randomTimerStartTime = millis(); // Record when the random timer started
  Serial.print("Lid opened, starting random timer: ");
  Serial.print(randomTimerDuration / 1000); // Display in seconds
  Serial.println(" seconds");
}

void setup() {
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(buzzerPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP); // Assuming LOW means closed
  attachInterrupt(switchPin, isr , RISING);


  // Start the serial communication
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  client.setKeepAlive(60); // Set keep-alive interval to 60 seconds
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
      for (int i = 0; i < 3; i++) {
        client.subscribe(mqttSubtopics[i]);
      }
       client.loop();
    } 
    else {
      Serial.println("Failed to connect to MQTT broker. Retrying...");
      delay(2000);
    }

  }
   lcd.init();
  lcd.backlight();
  lcd.clear();
}


int subtopicIndex = -1;

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to a string
  payload[length] = '\0';
  String message = String((char*)payload);

  strcpy(lastTopic, topic); // Store the last topic

  int subtopicIndex = -1; // Find the subtopic index
  for (int i = 0; i < 3; i++) {
    if (strcmp(topic, mqttSubtopics[i]) == 0) {
      subtopicIndex = i;
      break;
    }
  }

  if (subtopicIndex != -1) {
    // Parse the message to extract time and medicine name
    int separatorIndex = message.indexOf(',');
    String time = message.substring(separatorIndex + 1); // Get the medicine time
    String medicineName = message.substring(0, separatorIndex); // Get the medicine name

    // Display the medicine name on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(subtopicIndex + 1) + ": " + medicineName);
    Serial.println(String(subtopicIndex + 1) + ": " + medicineName);

    // Turn on the LED and buzzer for the corresponding subtopic
    digitalWrite(ledPins[subtopicIndex], HIGH);
    tone(buzzerPin, 1000);

    buzzerOn = true;
    buzzerStartTime = millis();

    TimerStarted = true;
    TimerStartTime = millis();
  }
}

void loop()
{

  if (!client.connected()) {
    reconnect();
  }

  client.loop(); // Keep MQTT communication alive
   
 for (int i = 0; i < 3; i++) {
    if (strcmp(lastTopic, mqttSubtopics[i]) == 0) {
      subtopicIndex = i;
      break;
    }
  }

  if((millis() - TimerStartTime) < ThresholdTime){
    if(SwitchPressed){
     if (randomTimerStarted && (millis() - randomTimerStartTime) <= randomTimerDuration) {
       if (digitalRead(switchPin) == LOW) { // Lid closed within random timer
         Serial.println("Lid closed within the random timer (Medicine NOT taken)");
         LidOpened=false;
         client.publish("alert", "Medicine NOT taken");
         lcd.clear();
         lcd.setCursor(0, 0);
         lcd.print("Medicine NOT taken");
         resetState(); // Reset everything and exit
         return;
        }
      } 

     // If the random timer expired but lid closed within 1 minute, consider medicine taken
     if (randomTimerStarted && (millis() - randomTimerStartTime) > randomTimerDuration) 
     {
        if (digitalRead(switchPin) == LOW) { // Lid closed within 1 minute after random timer
         Serial.println("Lid closed after random timer but within 1 minute (Medicine TAKEN)");
         LidOpened=false;
         client.publish("alert", "Medicine TAKEN");
         resetState(); // Reset the state and exit
         return;
        }
      }
    }   
}
  
 else
  {
    if(((millis() - TimerStartTime) > ThresholdTime) && (LidOpened) && (!LidMessagePrinted)){
      
      Serial.println("Lid still opened");
      LidMessagePrinted= true;
      tone(buzzerPin, 1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lid still opened");
      if(digitalRead(switchPin) == LOW){
         resetState();
      }
    }
    if(((millis() - TimerStartTime) > ThresholdTime) && (!SwitchPressed)  && (!TimerMessagePrinted)){
    Serial.println("15-minute timer expired");
    digitalWrite(ledPins[subtopicIndex], HIGH);
    tone(buzzerPin, 1000);
    delay(3000);
    client.publish("alert", "Medicine NOT TAKEN");
    resetState();
    TimerMessagePrinted = true; // Reset the state and exit
    return;
    }
  }
  if (buzzerOn && (millis() - buzzerStartTime) >= buzzerDuration) {
    tone(buzzerPin, 0); // Turn off the buzzer
    buzzerOn = false;
  }
}

void resetState() {
  // Reset all flags and indicators
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  tone(buzzerPin, 0); // Stop the buzzer
  lcd.clear(); // Clear the LCD screen
  // Reset timer flags
  TimerStarted = false;
  randomTimerStarted = false;
}

void reconnect() {
  // Attempt to reconnect to MQTT
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client")) { // Adjust the client ID as needed
      Serial.println("Connected to MQTT broker");
      for (int i = 0; i < 3; i++) {
        client.subscribe(mqttSubtopics[i]);
      }
    } else {
      Serial.println("Failed to connect to MQTT broker. Retrying...");
      delay(2000); // Wait before retrying
    }
  }
}


