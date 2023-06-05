/**
 * @file    data_scraping.h
 * @brief   Establish SSL/TLS connection and extract frequency data from HTTP response
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#pragma once

#include "config_macros.h"

esp_err_t data_scraping_init(void);
esp_err_t data_scraping_get_freq(float* freq);
