/**
 * @file    ui.c
 * @brief   Configure and run user interface (LED Display, Button)
 * @author  Karol Wojslaw (karol.wojslaw@student.manchester.ac.uk)
 */

#include "ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button.h"

#define TAG "ui"

/* 7-segment display ASCI digits lookup table */
const unsigned char seven_seg_digits_decode_gfedcba[75]= {
/*  0     1     2     3     4     5     6     7     8     9     :     ;     */
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x00, 0x00, 
/*  <     =     >     ?     @     A     B     C     D     E     F     G     */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 
/*  H     I     J     K     L     M     N     O     P     Q     R     S     */
    0x76, 0x30, 0x1E, 0x75, 0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x6D, 
/*  T     U     V     W     X     Y     Z     [     \     ]     ^     _     */
    0x78, 0x3E, 0x1C, 0x1D, 0x64, 0x6E, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 
/*  `     a     b     c     d     e     f     g     h     i     j     k     */
    0x00, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x10, 0x1E, 0x75, 
/*  l     m     n     o     p     q     r     s     t     u     v     w     */
    0x38, 0x55, 0x54, 0x5C, 0x73, 0x67, 0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x1D, 
/*  x     y     z     */
    0x64, 0x6E, 0x5B
};

/**
 * @brief Decode a character to its corresponding 7-segment display segment representation.
 *
 * @param ch The character to decode.
 * @return The 7-segment display segment representation of the character, or 0x00 if the character is not a digit.
 */
static uint8_t ui_decode_7seg(unsigned char ch) {
    if (ch > (unsigned char)'z') {
        return 0x00;
    }
	return seven_seg_digits_decode_gfedcba[ch - '0'];
}

/**
 * @brief Display a string on the user interface.
 *
 * @param str Pointer to a null-terminated string to display on the user interface. Must not be NULL.
 * @param ui Pointer to a ui_config_t structure representing the user interface configuration. Must not be NULL.
 * @return `ESP_OK` if the string was displayed successfully, otherwise an error code.
 * 
 * @note String has to be 4 characters long!
 */
static esp_err_t ui_display_string(char* str, const ui_config_t *ui) {
    if(str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    tm1637_set_brightness((ui->led), UI_LED_MAX_BRIGHT);
    
    for(int i = 0; i < 4; i++){
        uint8_t data = ui_decode_7seg(*(str+i));
        tm1637_set_segment_raw((ui->led), i, data);
    }

    return ESP_OK;
}

esp_err_t ui_init(ui_config_t *ui) {
    (ui->led) = tm1637_init(PIN_TM1637_CLK, PIN_TM1637_DIO);
    ESP_LOGI(TAG, "tm1637 initialised");
    
    ESP_ERROR_CHECK(button_init());
    ESP_LOGI(TAG, "button initialised");

    return ESP_OK;
}

esp_err_t ui_startup_animation(const ui_config_t *ui) {
    ESP_LOGI(TAG, "Run startup animation");
    for (int x = 0; x < 7; x++) {
        tm1637_set_brightness((ui->led), x);
        tm1637_set_number((ui->led), 8888);
        vTaskDelay(250 / portTICK_RATE_MS);
    }
    return ESP_OK;
}

esp_err_t ui_display_freq(const ui_config_t *ui, const float freq_float, const bool dots) {
    ESP_LOGD(TAG, "Display frequency");
    uint16_t freq_int = (uint16_t)(freq_float * 100);

    tm1637_set_brightness((ui->led), UI_LED_MAX_BRIGHT);
    tm1637_set_number_lead_dot((ui->led), freq_int, true, dots ? 0xFF : 0x00);
    
    return ESP_OK;
}

esp_err_t ui_display_message(const ui_config_t *ui, const ui_message_t message) {  
    if (ui == NULL) {
        return ESP_ERR_INVALID_ARG;
    } else {
        ESP_LOGI(TAG, "Display message");
    }

    switch(message){
        case UI_MESSAGE_ERROR:
            ESP_ERROR_CHECK(ui_display_string("ERR_", ui));
            break;
        case UI_MESSAGE_PROV:
            ESP_ERROR_CHECK(ui_display_string("PROV", ui));
            break;
        case UI_MESSAGE_CONNECTED:
            ESP_ERROR_CHECK(ui_display_string("Conn", ui));
            break;
        case UI_MESSAGE_RUNNING:
            ESP_ERROR_CHECK(ui_display_string("On__", ui));
            break;
        case UI_MESSAGE_WIFI:
            ESP_ERROR_CHECK(ui_display_string("UiFi", ui));
            break;
        default:
            ESP_ERROR_CHECK(ui_display_string("inv-", ui));
            break;
    }

    return ESP_OK;
}

int ui_get_button_level(ui_config_t *ui) {
    return button_get_level();
}
