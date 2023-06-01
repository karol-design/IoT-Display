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
#define PIN 1

/* WiFi */
#define WIFI_SSID "iPhone (Karol)"
#define WIFI_PASS "karol1234"

/* HTTPS Frequency Data Source (API) */
#define WEB_SERVER "extranet.nationalgrid.com"
#define WEB_PORT "443"
#define WEB_URL "https://extranet.nationalgrid.com/Realtime/Home/SystemData"
#define HTML_BUFFER_SIZE 2048
#define TEMP_BUFFER_SIZE 30

/* WiFi Provisioning */
#define PROV_MGR_MAX_RETRY_CNT 5
#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_BLE "ble"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"