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

    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Initialize the event loop

    ESP_ERROR_CHECK(provisioning_init()); // Run provisioning and establish WiFi connection

    https_drv_init();

    while (true) {
        ESP_LOGI(TAG, "Running...");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
