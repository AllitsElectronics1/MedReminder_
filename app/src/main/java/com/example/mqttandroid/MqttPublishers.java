package com.example.mqttandroid;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;

import androidx.core.app.NotificationCompat;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;


public class MqttPublishers {
    private MqttClient mqttClient;
    private static final String CHANNEL_ID = "mqtt_notification_channel";
    private Context context;

    public MqttPublishers(String serverUri, String clientId, Context applicationContext) {
        context = applicationContext;
        try {
            mqttClient = new MqttClient(serverUri, clientId, new MemoryPersistence());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void connect() {
        MqttConnectOptions options = new MqttConnectOptions();
        options.setCleanSession(true);

        try {
            mqttClient.connect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void publish(String topic, String message) {
        try {
            MqttMessage mqttMessage = new MqttMessage(message.getBytes());
            mqttClient.publish(topic, mqttMessage);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    public void subscribe(String topic) {

        try {
            mqttClient.subscribe(topic);
            mqttClient.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    // Handle connection lost
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    // Handle incoming message
                    String payload = new String(message.getPayload());
                    System.out.println("Received message on topic: " + topic + ", Message: " + payload);

                    // Show notification
                    showNotification(context, topic, payload);

                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    // Handle message delivery
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void showNotification(Context context, String topic, String message) {
        NotificationManager notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            // NotificationChannel channel = new NotificationChannel(CHANNEL_ID, "MQTT Notification", NotificationManager.IMPORTANCE_DEFAULT);
            CharSequence name = "Mqtt message Channel";
            String description = "Channel for Mqtt connection";
            int importance = NotificationManager.IMPORTANCE_HIGH;
            NotificationChannel channel = new NotificationChannel("Mqtt_channel", name, importance);
            channel.setDescription(description);
            notificationManager.createNotificationChannel(channel);
        }

        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, "Mqtt_channel")
                .setSmallIcon(R.drawable.ic_notification)
                .setContentTitle(topic)
                .setContentText(message)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT);

        notificationManager.notify(0, builder.build());
    }


    public void disconnect() {
        try {
            mqttClient.disconnect();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}