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
#include "provisioning.h"

#define TAG "app"

void app_main(void) {
    esp_err_t err = ESP_OK;

    err = nvs_flash_init();  // Initialize NVS
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Error when initialising NVS. Erasing NVS and retrying...");
        ESP_ERROR_CHECK(nvs_flash_erase());  // Erase NVS flash memory
        err = nvs_flash_init();              // And try initialising it again
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Initialize the event loop

    bool perform_reprovisioning = true;
    ESP_ERROR_CHECK(provisioning_init(perform_reprovisioning)); // Run provisioning and establish WiFi connection

    https_drv_init();

    while (true) {
        int8_t rssi;
        ESP_ERROR_CHECK(provisioning_get_rssi(&rssi));
        ESP_LOGI(TAG, "WiFi RSSI: %d dBm", rssi);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
