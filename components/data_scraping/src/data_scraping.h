/**
 * @file    data_scraping.h
 * @brief   ...
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_macros.h"
#include "esp_crt_bundle.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"

void data_scraping_init(void);
