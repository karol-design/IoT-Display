/**
 * @file    data_scraping.h
 * @brief   ...
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#pragma once

#include "config_macros.h"

// void https_get_task(void);
esp_err_t data_scraping_init(void);
esp_err_t data_scraping_get_freq(float* freq);
