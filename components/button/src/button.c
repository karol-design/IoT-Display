/**
 * @file    button.c
 * @brief   Initialise GPIO for the button, read its current state
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#include "button.h"

#include <stdio.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "button"
#define GPIO_BUTTON_SEL (1ULL << GPIO_BUTTON)

/**
 * @brief Get the debounced level of a button.
 * 
 * @param gpio The GPIO pin number of the button.
 * @return The debounced level of the button (0 or 1).
 */
static int button_get_level_debounce(gpio_num_t gpio) {
    uint16_t stable_output_count = 0;
    int gpio_level, gpio_level_last = -1;

    while(stable_output_count < BUTTON_DEBOUNCE_MIN_COUNT){
        gpio_level = gpio_get_level(gpio);

        if(gpio_level == gpio_level_last) { // If GPIO level is unchanged since the last reading...
            stable_output_count++;          // ...increase stable output counter
        } else {
            stable_output_count = 0;        // Otherwise: reset stable output counter
        }

        gpio_level_last = gpio_level; // Save last GPIO reading
        vTaskDelay(10 / portTICK_PERIOD_MS); // Get the new reading every 10 ms
    }

    return gpio_level;
}

int button_get_level(void) {
    return button_get_level_debounce(GPIO_BUTTON);
}

esp_err_t button_init(void) {
    // Initialize the config structure for the GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = GPIO_BUTTON_SEL,  // bit mask of the pins to be used
        .mode = GPIO_MODE_INPUT,          // set as input
        .pull_up_en = 1,                  // enable pull-up
        .pull_down_en = 0,                // disable pull-down mode
        .intr_type = GPIO_INTR_DISABLE,   // disable interrupt
    };

    esp_err_t err = gpio_config(&io_conf);  // configure GPIO with the given settings

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Initialising GPIO unsuccessful");
    } else {
        ESP_LOGI(TAG, "Initialising GPIO successful");
    }

    return err;
}
