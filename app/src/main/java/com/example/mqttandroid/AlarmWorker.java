package com.example.mqttandroid;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.NonNull;
import androidx.core.app.NotificationCompat;
import androidx.work.Worker;
import androidx.work.WorkerParameters;
import androidx.work.PeriodicWorkRequest;
import androidx.work.WorkManager;
import java.util.concurrent.TimeUnit;

import java.util.Calendar;

public class AlarmWorker extends Worker {

    private String medicineName;
    private Long medicineTime;

    private MediaPlayer mediaPlayer;

    public AlarmWorker(@NonNull Context context, @NonNull WorkerParameters params) {
        super(context, params);
    }

    @NonNull
    @Override
    public Result doWork() {
        // This method will be called when the work is scheduled to run
        // Receive data from input Data object
        medicineName = getInputData().getString("medicineName");
        medicineTime = getInputData().getLong("medicineTime", 0);
        playAlarmSound(getApplicationContext());
        triggerNotification(getApplicationContext());
        connectToMQTTServer();
        //schedulePeriodicWork();
        return Result.success();
    }

    /*private void schedulePeriodicWork() {
        // Define the periodic interval
        long repeatInterval = 20; // Repeat every 2 minutes
        TimeUnit timeUnit = TimeUnit.MINUTES;

        // Create the periodic work request
        PeriodicWorkRequest.Builder periodicWorkRequestBuilder =
                new PeriodicWorkRequest.Builder(AlarmWorker.class, repeatInterval, timeUnit);

        // Optionally, you can set additional constraints or input data here

        // Build and enqueue the periodic work request
        PeriodicWorkRequest periodicWorkRequest = periodicWorkRequestBuilder.build();
        WorkManager.getInstance(getApplicationContext()).enqueue(periodicWorkRequest);
    }*/

    private void connectToMQTTServer() {
        String daystatus = getdaystatus(medicineTime);
        MqttPublishers mqttPublisher  = new MqttPublishers("tcp://192.168.1.14:1883", "android_client",getApplicationContext());
        mqttPublisher.connect();
        mqttPublisher.publish(daystatus, medicineName);
        mqttPublisher.subscribe("alert");
    }

    private String getdaystatus(Long medicineTime) {
        // Create a Calendar instance and set its time based on the provided milliseconds
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(medicineTime);

        // Get the hour of the day
        int hourOfDay = calendar.get(Calendar.HOUR_OF_DAY);

        // Classify the time of day
        if (hourOfDay < 12) {
            return "Morning";
        } else if (hourOfDay < 18) {
            return "Afternoon";
        } else {
            return "Evening";
        }
    }

    private void triggerNotification(Context context) {
        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        // Create notification channel if the SDK version is Oreo or higher
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = "Medicine Reminder Channel";
            String description = "Channel for Medicine Reminder Notifications";
            int importance = NotificationManager.IMPORTANCE_HIGH;
            NotificationChannel channel = new NotificationChannel("medicine_reminder_channel", name, importance);
            channel.setDescription(description);
            notificationManager.createNotificationChannel(channel);
        }

        // Create the notification
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, "medicine_reminder_channel")
                .setSmallIcon(R.drawable.ic_notification)
                .setContentTitle("Time to take medicine")
                .setContentText("It's time to take " + medicineName)
                .setPriority(NotificationCompat.PRIORITY_MAX);

        // Display the notification
        notificationManager.notify(1, builder.build());
    }

    private void playAlarmSound(Context context) {

        mediaPlayer = MediaPlayer.create(context, R.raw.demo);
        mediaPlayer.setLooping(false); // Set looping
        mediaPlayer.start(); // Start playing the alarm sound
    }

}

