/*
   fetches the ip address associated with the user input mac address from the registered device database.
   stores the input data (med name, time, label) by user in a csv file.
*/

<?php

$host = "localhost";
$user = "Suni";
$password = "Suni@1704";
$dbname = "esp_devices";

// Database connection
$conn = new mysqli($host, $user, $password, $dbname);

// Check for connection errors
if ($conn->connect_error) {
    http_response_code(500);
    die(json_encode(["status" => "error", "message" => "Database connection failed: " . $conn->connect_error]));
}

// Ensure the request is POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Check if required fields are present
    if (isset($_POST['mac']) && isset($_POST['medicine']) && isset($_POST['time']) && isset($_POST['label'])) {
        $mac = $conn->real_escape_string($_POST['mac']);
        $medicines = $_POST['medicine'];
        $times = $_POST['time'];
        $labels = $_POST['label'];

        // Fetch the IP address associated with the MAC address
        $sql = "SELECT ip_address FROM devices WHERE mac_address = '$mac'";
        $result = $conn->query($sql);

        if ($result->num_rows > 0) {
            $row = $result->fetch_assoc();
            $ipAddress = $row['ip_address'];

            // Process each medicine reminder
            for ($i = 0; $i < count($medicines); $i++) {
                $medicine = $conn->real_escape_string($medicines[$i]);
                $time = $conn->real_escape_string($times[$i]);
                $label = $conn->real_escape_string($labels[$i]);

                // Save to medicine_schedule table
                $insertSql = "INSERT INTO medicine_schedule (mac_address, medicine_name, schedule_time) 
                              VALUES ('$mac', '$medicine', '$time')";

                // Add entry to reminders.csv
                $data = $ipAddress . "," . $medicine . "," . $label . "," . $time . "\n";
                file_put_contents("/var/www/html/esp_1/reminders.csv", $data, FILE_APPEND);

                if (!$conn->query($insertSql)) {
                    error_log("Failed to insert reminder: " . $conn->error);
                }
            }

            // Return success message
            echo json_encode(["status" => "success", "message" => "Reminders set successfully for IP: $ipAddress"]);
        } else {
            // MAC not found in the devices table
            http_response_code(404);
            echo json_encode(["status" => "error", "message" => "MAC address not found in registered devices"]);
        }
    } else {
        // Missing required POST data
        http_response_code(400);
        echo json_encode(["status" => "error", "message" => "MAC, Medicine, Time, and Label are required"]);
    }
} else {
    // Invalid request method
    http_response_code(405);
    echo json_encode(["status" => "error", "message" => "Invalid request method. POST expected."]);
}

// Close the database connection
$conn->close();
?>
