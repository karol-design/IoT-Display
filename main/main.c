/**
 * @file    main.c
 * @brief   Main application file
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "http_drv.h"
#include "nvs_flash.h"
#include "wifi_drv.h"

#define TAG "app"

void app_main(void) {
    esp_err_t err = ESP_OK;

    err = nvs_flash_init();  // Initialize NVS
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());  // Erase NVS flash memory
        err = nvs_flash_init();              // And try initialising it again
    }

    ESP_ERROR_CHECK(wifi_drv_init());        // Initialise WiFi
    while (wifi_drv_connected() == false) {  // Wait for the device to connect to the AP
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "WiFi RSSI: %d", wifi_drv_get_rssi());
    vTaskDelay(500 / portTICK_PERIOD_MS);

    https_drv_init();

    while (true) {
        if (wifi_drv_connected() == false) {
            ESP_LOGE(TAG, "WiFi drv fault: %d", wifi_drv_connected());
            esp_restart();  // Reboot the microcontroller
        }
    }
}
