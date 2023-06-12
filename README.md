## any-clock | IoT Clock-like display capable of extracting and dipsplaying data from any website

The any-clock is not your ordinary desk clock. Instead of a boring and ubiquitous time display, it shows the real-time data extracted from any http/https website.

### CI Workflow results

![ESP-IDF CI Build](https://github.com/karol-design/IoT-Display/actions/workflows/esp-idf.yml/badge.svg)


## About 
The any-clock utilizes the ESP32 microcontroller with built-in Wi-Fi. Once provisioned, it can connect to any available Wi-Fi access point (AP) within range. The device operates exclusively with 2.4 GHz Wi-Fi APs and should be placed no further than 15 meters from the AP.

## Contributing (Firmware)
Commit to the main only the code that compile without any warnings or errors.
To test, compile or flash the code use ESP-IDF 4.4.1

## How to use

1) Setting up a provisioning device:
Download the "ESP BLE Provisioning" app from the App Store or Google Play (requires iOS/Android device).
Enable Bluetooth on your iOS/Android device and ensure that the "allow new connections" (or similar) setting is turned on if available.

2) Powering up the any-clock:
Connect the USB cable to a mains USB charger or any USB socket (e.g., on your laptop). The any-clock will display a startup animation followed by the “on" and "Wi-Fi" messages.

3) Provisioning:
Open the "ESP BLE Provisioning" app and follow these instructions:
Scan the QR Code from the device, choose the Wi-Fi AP, and enter the password.
Alternatively, search manually for the device, enter the proof-of-possession (PoP) code [0000 for this device], select the Wi-Fi network, and enter the password.

4) any-clock running:
The any-clock should now display the "conn" message, and within a few seconds, the data should be extracted and displayed. In case of a restart, the any-clock will attempt to connect to the same Wi-Fi AP used during the last provisioning. You can uninstall the "ESP BLE Provisioning" app now.

### Reprovisioning

If you need to change the Wi-Fi credentials, follow these steps:
- Turn off the any-clock by disconnecting the USB cable.
- Press and hold the reset button on the any-clock.
- Plug the power back in while keeping the reset button pressed.
- Release the reset button when you see the provisioning ("Prov") message.
- Repeat steps 3 and 4 from the "Provisioning" section in the instruction above.

