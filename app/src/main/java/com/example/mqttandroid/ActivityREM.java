package com.example.mqttandroid;


import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TimePicker;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.work.Data;
import androidx.work.ExistingWorkPolicy;
import androidx.work.OneTimeWorkRequest;
import androidx.work.WorkManager;

import java.util.Calendar;
import java.util.concurrent.TimeUnit;

public class ActivityREM extends AppCompatActivity {
    EditText medicineNameEditText;
    TimePicker timePicker;
    Button setReminderButton;

    private MqttPublishers mqttPublisher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_rem);

        medicineNameEditText = findViewById(R.id.medicineNameEditText);
        timePicker = findViewById(R.id.timePicker);
        setReminderButton = findViewById(R.id.setReminderButton);


        setReminderButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                String medicineName = medicineNameEditText.getText().toString();
                int hour = timePicker.getCurrentHour();
                int minute = timePicker.getCurrentMinute();

                Calendar calendar = Calendar.getInstance();
                calendar.set(Calendar.HOUR_OF_DAY, hour);
                calendar.set(Calendar.MINUTE, minute);
                calendar.set(Calendar.SECOND, 0);

                setAlarm(calendar.getTimeInMillis(), medicineName);
            }

        });
    }


    private void showDefaultDialog(String message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Title");
        builder.setMessage(message);
        builder.setPositiveButton("OK", null); // You can add functionality here if needed
        builder.setNegativeButton("Cancel", null); // You can add functionality here if needed
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private void setAlarm(long timeInMillis, String medicineName) {
        // Get the current system time in milliseconds
        long currentTimeMillis = System.currentTimeMillis();

        if (timeInMillis >= currentTimeMillis) {
            // If the input time is in the future, schedule the alarm as before

            showDefaultDialog("Reminder set for " + medicineName);
            Toast.makeText(this, "Reminder set for " + medicineName, Toast.LENGTH_SHORT).show();

            Data inputData = new Data.Builder()
                    .putString("medicineName", medicineName)
                    .putLong("medicineTime", timeInMillis)
                    .build();
            // Schedule the work using WorkManager
            OneTimeWorkRequest alarmWorkRequest =
                    new OneTimeWorkRequest.Builder(AlarmWorker.class)
                            .setInputData(inputData)
                            .setInitialDelay(timeInMillis - System.currentTimeMillis(), TimeUnit.MILLISECONDS)
                            .build();
            WorkManager.getInstance(this).enqueueUniqueWork(
                    "SET ALARM",
                    ExistingWorkPolicy.REPLACE,
                    alarmWorkRequest
            );
        } else {
            // If the input time is in the past, notify the user that the reminder cannot be set
            showDefaultDialog("Input time is in the past");
            Toast.makeText(this, "Input time is in the past", Toast.LENGTH_SHORT).show();
        }
    }
}





