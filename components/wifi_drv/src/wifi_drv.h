/**
 * @file    wifi_drv.h
 * @brief   Initialize, configure and start Wi-Fi in a station mode, handle WiFi and IP events
 * @author  Karol Wojslaw (karol.wojslaw@student.manchester.ac.uk)
 */

#pragma once

#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "config_macros.h"

esp_err_t wifi_drv_init();
uint8_t wifi_drv_connected();
int8_t wifi_drv_get_rssi();
