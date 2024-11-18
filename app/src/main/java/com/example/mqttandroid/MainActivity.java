package com.example.mqttandroid;

import androidx.activity.OnBackPressedDispatcher;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity  {
    private EditText patientNameEditText;
    private EditText macAddressEditText;
    private EditText numOfRemindersEditText;


    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);



        patientNameEditText  = findViewById(R.id.patientNameEditText);
        macAddressEditText = findViewById(R.id.macAddressEditText);
        numOfRemindersEditText = findViewById(R.id.numOfRemindersEditText);
        numOfRemindersEditText = findViewById(R.id.numOfRemindersEditText);

        Button button2 = findViewById(R.id.button2);
        button2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Create an instance of your custom class

                // Create an intent to start the target activity
                Intent intent = new Intent(MainActivity.this, ActivityREM.class);

                // Put the custom class object as an extra to the intent

                // Start the target activity
                startActivity(intent);
            }
        });

        Button button6 = findViewById(R.id.button6);
        button6.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Create an instance of your custom class

                // Create an intent to start the target activity
                Intent intent = new Intent(MainActivity.this, ActivityREM.class);

                // Put the custom class object as an extra to the intent

                // Start the target activity
                startActivity(intent);
            }
        });

        Button button7 = findViewById(R.id.button7);
        button7.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Create an instance of your custom class

                // Create an intent to start the target activity
                Intent intent = new Intent(MainActivity.this, ActivityREM.class);

                // Put the custom class object as an extra to the intent

                // Start the target activity
                startActivity(intent);
            }
        });




        /*Button exportDeleteEditButton = findViewById(R.id.export_delete_edit_button);
        exportDeleteEditButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String patientName = patientNameEditText.getText().toString();
                String macAddress = macAddressEditText.getText().toString();
                int numOfReminders = Integer.parseInt(numOfRemindersEditText.getText().toString());

            }*/

           /* Button button2=findViewById(R.id.button2);
            Button button6=findViewById(R.id.button6);
            Button button7=findViewById(R.id.button7);*/



        //  @Override
            /*public boolean onCreateOptionsMenu(Menu menu) {
                getMenuInflater().inflate(R.menu.toolbar_menu, menu);
                return true;
            }*/


    }


}


