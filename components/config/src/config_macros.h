/**
 * @file    config_macros.h
 * @brief   Configuration macros and common includes for use throughout the project
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#pragma once

#include <string.h>
#include "esp_check.h"
#include "esp_log.h"

/* PIN Assignment */
#define PIN_TM1637_CLK 16   // GPIO number (IOxx) for CLK pin of TM1637 display.
#define PIN_TM1637_DIO 17   // GPIO number (IOxx) for DIO pin of TM1637 display.

/* WiFi */
#define WIFI_SSID "iPhone (Karol)"  // WiFi Access Point SSID
#define WIFI_PASS "karol1234"       // WiFi Access Point Password

/* HTTPS Frequency Data Source (API) */
#define WEB_SERVER "extranet.nationalgrid.com"  // Web server with freq data
#define WEB_PORT "443"                          // Web port (443 for SSL)
#define WEB_URL "https://extranet.nationalgrid.com/Realtime/Home/SystemData"    // URL with freq data 
#define HTTP_BUFFER_SIZE 2048   // Size of the buffer for HTTP response message (with HTML file)
#define TEMP_BUFFER_SIZE 30     // Temporary buffer size (for searching Frequency data)

/* WiFi Provisioning */
#define PROV_MGR_MAX_RETRY_CNT 5    // Max number of provisioning retries before resetting Prov Mgr
#define PROV_QR_VERSION "v1"        // QR Code version
#define PROV_TRANSPORT_BLE "ble"    // Provisioning transport type
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html" // QR code base URL