""" 
   fetches the alert data from "csv file" such as ip address ,med name, time, and label(morning, afternoon, eveining) 
   sends alert to respective ip when time fetched from csv file matches the current time
   
 """

import time
import csv
import requests
from datetime import datetime

# Path to the reminders file
REMINDER_FILE = "/var/www/html/esp_1/reminders.csv"

# Function to send message to ESP32
def send_message(ip_address, message,label):
    try:
        # Replace with your ESP32 endpoint
        url = f"http://{ip_address}/notify"
        print(f"Sending message to {url} with message: {message} and label: {label}")  # Debug: print message and label
        # Send POST request with message and label
        response = requests.post(url, json={"message": message, "label": label})

        # Debug: Print response from ESP32
        if response.status_code == 200:
            print(f"Message sent to {ip_address}: {message}")
            print(f"Response: {response.text}")
        else:
            print(f"Failed to send message to {ip_address}: {response.status_code} - {response.text}")
    except Exception as e:
        print(f"Failed to send message to {ip_address}: {e}")

# Main loop
while True:
    try:
        # Get current time
        now = datetime.now().strftime("%H:%M")  # Get local time

        print(f"Current time: {now}")  # Debug: print current time

        # Check reminders
        with open(REMINDER_FILE, "r") as file:
            reader = csv.reader(file)
            reminders = list(reader)

        updated_reminders = []
        for reminder in reminders:
            # Extract details from the reminder
            try:
                ip, medicine, label, time_set = reminder
                print(f"Checking reminder for {ip} at {time_set} with label {label}")  # Debug: print reminder check
            except ValueError:
                print(f"Invalid reminder format: {reminder}")
                updated_reminders.append(reminder)
                continue

            if time_set == now:
                # Prepare and send the message to the ESP32
                message = f"{medicine}"
                print(f"Sending message: {message}")  # Debug: message send trigger
                send_message(ip, message,label)
            else:
                # Keep reminders that are not yet triggered
                updated_reminders.append(reminder)

        # Write back remaining reminders
        with open(REMINDER_FILE, "w") as file:
            writer = csv.writer(file)
            writer.writerows(updated_reminders)

    except Exception as e:
        print(f"Error: {e}")
    
    # Sleep for a minute before checking again
    time.sleep(60)

