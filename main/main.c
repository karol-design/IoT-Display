/**
 * @file    main.c
 * @brief   Main application file
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_system.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "http_drv.h"
#include "nvs_flash.h"
#include "wifi_drv.h"

#define TAG "app"

#define MAX_BUFFER_SIZE 1024

// HTTP client event handler
esp_err_t httpClientEventHandler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Print the received data
                printf("%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}

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

    // Configure HTTP client
    esp_http_client_config_t httpConfig = {
        .url = "http://www.testingmcafeesites.com/index.html",
        .event_handler = httpClientEventHandler,
    };

    // Perform HTTP request
    esp_http_client_handle_t client = esp_http_client_init(&httpConfig);
    err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP request complete");
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);

    while (true) {
        if (wifi_drv_connected() == false) {
            ESP_LOGE(TAG, "WiFi drv fault: %d", wifi_drv_connected());
            esp_restart();  // Reboot the microcontroller
        }
    }
}
