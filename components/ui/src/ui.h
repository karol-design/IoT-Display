/**
 * @file    ui.h
 * @brief   Configure and run user interface (LED Display, Button)
 * @author  Karol Wojslaw (karol.wojslaw@student.manchester.ac.uk)
 */

#pragma once
#include "config_macros.h"
#include "tm1637.h"

/* User interface config struct */
typedef struct {
	tm1637_led_t* led;
} ui_config_t;

/* User Interface message type */
typedef enum {
	UI_MESSAGE_ERROR = 0x00,
	UI_MESSAGE_PROV = 0x01,
	UI_MESSAGE_CONNECTED = 0x03,
	UI_MESSAGE_RUNNING = 0x02
} ui_message_t;

/**
 * @brief Initialize the user interface.
 *
 * @param ui Pointer to a ui_config_t structure representing the user interface configuration. Must not be NULL.
 * @return `ESP_OK` if the user interface was initialized successfully, otherwise an error code.
 */
esp_err_t ui_init(ui_config_t *ui);

/**
 * @brief Run a startup animation on the user interface.
 *
 * @param ui Pointer to a ui_config_t structure representing the user interface configuration. Must not be NULL.
 * @return `ESP_OK` if the startup animation completed successfully, otherwise an error code.
 */
esp_err_t ui_startup_animation(const ui_config_t *ui);

/**
 * @brief Display a frequency value on the user interface.
 *
 * @param ui Pointer to a ui_config_t structure representing the user interface configuration. Must not be NULL.
 * @param freq_float The frequency value to display on the user interface.
 * @param dots Flag for deciding if dots should be on or off.
 * @return `ESP_OK` if the frequency was displayed successfully, otherwise an error code.
 */
esp_err_t ui_display_freq(const ui_config_t *ui, const float freq_float, const bool dots);

/**
 * @brief Display a message on the user interface.
 *
 * @param ui Pointer to a ui_config_t structure representing the user interface configuration. Must not be NULL.
 * @param message The message to display on the user interface.
 * @return `ESP_OK` if the message was displayed successfully, otherwise an error code.
 */
esp_err_t ui_display_message(const ui_config_t *ui, const ui_message_t message);
