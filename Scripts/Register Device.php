/* 
   registers the esp device in database with its mac adress and updtaes it with respective ip address.
   updates the medicine status for respective mac and med name.
*/

<?php
// Enable error reporting
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

// Database connection details
$host = "localhost";
$user = "Suni";
$password = "Suni@1704";
$dbname = "esp_devices";

// Create database connection
$conn = new mysqli($host, $user, $password, $dbname);
if ($conn->connect_error) {
    http_response_code(500);
    die(json_encode(["status" => "error", "message" => "Database connection failed: " . $conn->connect_error]));
}

// Retrieve and decode JSON payload
$data = json_decode(file_get_contents('php://input'), true);
if (!$data) {
    http_response_code(400);
    die(json_encode([
        "status" => "error",
        "message" => "Failed to decode JSON payload",
        "raw_input" => file_get_contents('php://input')
    ]));
}

// Check if it's a device registration or medicine status update
if (isset($data['mac']) && isset($data['ip']) && !isset($data['medicine'])) {
    // Device registration/update logic
    $mac = $conn->real_escape_string($data['mac']);
    $ip = $conn->real_escape_string($data['ip']);

    // SQL query to insert or update device details
    $sql = "INSERT INTO devices (mac_address, ip_address) 
            VALUES ('$mac', '$ip') 
            ON DUPLICATE KEY UPDATE ip_address = '$ip'";

    if ($conn->query($sql) === TRUE) {
        echo json_encode(["status" => "success", "message" => "Device registered/updated successfully"]);
    } else {
        http_response_code(500);
        echo json_encode(["status" => "error", "message" => "Database query failed: " . $conn->error]);
    }
} elseif (isset($data['mac']) && isset($data['medicine']) && isset($data['status'])) {
    // Medicine status update logic
    $mac = $conn->real_escape_string($data['mac']);
    $medicine = $conn->real_escape_string($data['medicine']);
    $status = $conn->real_escape_string($data['status']);
    $timestamp = date('Y-m-d H:i:s'); // Current timestamp

    // SQL query to update status
    $sql = "UPDATE medicine_schedule 
            SET status = '$status', time = '$timestamp' 
            WHERE mac_address = '$mac' AND medicine_name = '$medicine'";

    if ($conn->query($sql) === TRUE) {
        if ($conn->affected_rows > 0) {
            echo json_encode(["status" => "success", "message" => "Medicine status updated successfully"]);
        } else {
            http_response_code(404);
            echo json_encode(["status" => "error", "message" => "No matching record found for the given MAC and medicine"]);
        }
    } else {
        http_response_code(500);
        echo json_encode(["status" => "error", "message" => "Database query failed: " . $conn->error]);
    }
} else {
    http_response_code(400);
    echo json_encode(["status" => "error", "message" => "Invalid input data. Required fields missing."]);
}

$conn->close();
?>
