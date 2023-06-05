/**
 * @file    button.h
 * @brief   Initialise GPIO for the button, read its current state
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#pragma once

#include "config_macros.h"

/**
 * @brief Get the current level of a button.
 * @return The level of the button (0 or 1).
 */
int button_get_level(void);

/**
 * @brief Initialize the button GPIO.
 * @return ESP_OK if the GPIO initialization is successful, otherwise an error code.
*/
esp_err_t button_init(void);