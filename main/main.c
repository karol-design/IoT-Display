/**
 * @file    main.c
 * @brief   Main application file
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#include "data_scraping.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "provisioning.h"
#include "ui.h"

#define TAG "app"

void app_main(void) {
    esp_err_t err = ESP_OK;
    ui_config_t ui;  // User interface config struct
    float freq_hz;   // Frequency in Hz
    int8_t rssi;     // WiFi AP RSSI

    ESP_ERROR_CHECK(ui_init(&ui));               // Initialise User Interface
    ESP_ERROR_CHECK(ui_startup_animation(&ui));  // Run startup animation
    ESP_ERROR_CHECK(ui_display_message(&ui, UI_MESSAGE_RUNNING));
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    err = nvs_flash_init();  // Initialize NVS
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Error when initialising NVS. Erasing NVS and retrying...");
        ESP_ERROR_CHECK(nvs_flash_erase());  // Erase NVS flash memory
        err = nvs_flash_init();              // And try initialising it again
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());  // Initialize the event loop

    ESP_ERROR_CHECK(ui_display_message(&ui, UI_MESSAGE_PROV));

    bool perform_reprovisioning = ui_get_button_level(&ui);
    ESP_ERROR_CHECK(provisioning_init(perform_reprovisioning));  // Run provisioning and establish WiFi connection

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(ui_display_message(&ui, UI_MESSAGE_CONNECTED));

    ESP_ERROR_CHECK(data_scraping_init());  // Initialise data scraping component

    /* Main app loop */
    while (true) {
        ESP_ERROR_CHECK(provisioning_get_rssi(&rssi));
        ESP_LOGI(TAG, "WiFi RSSI: %d dBm", rssi);

        ESP_ERROR_CHECK(data_scraping_get_freq(&freq_hz));
        ESP_LOGI(TAG, "Frequency: %.2f Hz", freq_hz);

        for(int i = 0; i < 60; i++) {   // Turn the dots on & off for approximately 60 seconds
            ESP_ERROR_CHECK(ui_display_freq(&ui, freq_hz, (i%2)));
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}
