
<?php
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $medicine = $_POST['medicine'];
    $timing = $_POST['timing'];

    $data = "$timing: $medicine\n";

    if (file_put_contents('/var/www/html/medicine.txt', $data)) {
        echo "Medicine for $timing sent to ESP32.";
    } else {
        echo "Failed to write to file.";
    }
}
?>
