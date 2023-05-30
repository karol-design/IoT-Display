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