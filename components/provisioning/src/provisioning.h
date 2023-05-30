/**
 * @file    provisioning.h
 * @brief   Configure WiFi with ESP Provisioning Manager, connect to the AP, handle WiFi and IP events
 * @author  Karol Wojslaw (karol.wojslaw@student.manchester.ac.uk)
 */

#pragma once
#include "config_macros.h"

esp_err_t provisioning_init(bool reset_provisioning);
