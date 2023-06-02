/**
 * @file    main.c
 * @brief   Main application file
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#include <stdio.h>
#include <string.h>

#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <driver/gpio.h>

#include "data_scraping.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "provisioning.h"
#include "tm1637.h"

#define TAG "app"

void lcd_tm1637_task(void * arg)
{
	tm1637_led_t * lcd = tm1637_init(PIN_TM1637_CLK, PIN_TM1637_DIO);
    ESP_LOGI(TAG, "tm1637 initialised");

	setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
	tzset();

	while (true)
	{
        ESP_LOGI(TAG, "test segment control");
		// Test segment control
		uint8_t seg_data[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
		for (uint8_t x=0; x<32; ++x)
		{
			uint8_t v_seg_data = seg_data[x%6];
			tm1637_set_segment_raw(lcd, 0, v_seg_data);
			tm1637_set_segment_raw(lcd, 1, v_seg_data);
			tm1637_set_segment_raw(lcd, 2, v_seg_data);
			tm1637_set_segment_raw(lcd, 3, v_seg_data);
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
        
        ESP_LOGI(TAG, "test brightness");
		// Test brightness
		for (int x=0; x<7; x++) {
			tm1637_set_brightness(lcd, x);
			tm1637_set_number(lcd, 8888);
			vTaskDelay(300 / portTICK_RATE_MS);
		}

		for (uint8_t x=0; x<3; ++x)
		{
			// Set random system time
			struct timeval tm_test = {1517769863 + (x*3456), 0};
			settimeofday(&tm_test, NULL);

			// Get current system time
			time_t now = 0;
			struct tm timeinfo = { 0 };
			time(&now);
			localtime_r(&now, &timeinfo);
			int time_number = 100 * timeinfo.tm_hour + timeinfo.tm_min;

            ESP_LOGI(TAG, "display time with blinking dots");
			// Display time with blinking dots
			for (int z=0; z<5; ++z) {
				tm1637_set_number_lead_dot(lcd, time_number, true, z%2 ? 0xFF : 0x00);
				vTaskDelay(500 / portTICK_RATE_MS);
			}
		}

        ESP_LOGI(TAG, "test display numbers");
		// Test display numbers
		for (int x=0; x<16; ++x) {
			bool show_dot = x%2; // Show dot every 2nd cycle
			tm1637_set_segment_number(lcd, 0, x, show_dot);
			tm1637_set_segment_number(lcd, 1, x, show_dot); // On my display "dot" (clock symbol ":") connected only here
			tm1637_set_segment_number(lcd, 2, x, show_dot);
			tm1637_set_segment_number(lcd, 3, x, show_dot);
			vTaskDelay(100 / portTICK_RATE_MS);
		}
	}
}

void app_main(void) {
    xTaskCreate(&lcd_tm1637_task, "lcd_tm1637_task", 4096, NULL, 5, NULL);
    // esp_err_t err = ESP_OK;
    // float freq_hz = 0.0;

    // err = nvs_flash_init();  // Initialize NVS
    // if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //     ESP_LOGW(TAG, "Error when initialising NVS. Erasing NVS and retrying...");
    //     ESP_ERROR_CHECK(nvs_flash_erase());  // Erase NVS flash memory
    //     err = nvs_flash_init();              // And try initialising it again
    // }

    // ESP_ERROR_CHECK(esp_event_loop_create_default()); // Initialize the event loop

    // bool perform_reprovisioning = false;
    // ESP_ERROR_CHECK(provisioning_init(perform_reprovisioning)); // Run provisioning and establish WiFi connection
    // vTaskDelay(2000 / portTICK_PERIOD_MS);

    // data_scraping_init();

    while (true) {
        // int8_t rssi;
        // ESP_ERROR_CHECK(provisioning_get_rssi(&rssi));
        // ESP_LOGI(TAG, "WiFi RSSI: %d dBm", rssi);

        // ESP_ERROR_CHECK(data_scraping_get_freq(&freq_hz));
        // ESP_LOGI(TAG, "Frequency: %.2f Hz", freq_hz);

        ESP_LOGI(TAG, "App running...");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
