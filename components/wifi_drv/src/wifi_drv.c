/**
 * @file    wifi_drv.c
 * @brief   Initialize, configure and start Wi-Fi in a station mode, handle WiFi and IP events
 * @author  Karol Wojslaw (karol.wojslaw@student.manchester.ac.uk)
 */

#include "wifi_drv.h"

#define TAG "wifi_drv"

static uint8_t wifi_connected = false;  // IP assigned by the AP flag

/**
 * @brief WIFI and IP events handler
 */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGW(TAG, "Device connected to the AP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGW(TAG, "Device disconnected from the AP");
        wifi_connected = false;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
        ESP_LOGW(TAG, "Lost IP Event");
        wifi_connected = false;
    }
}

/**
 * @brief Test whether an IP adress has been successfully assigned to the device by the AP
 * @return got_ip flag value
 */
uint8_t wifi_drv_connected() {
    return wifi_connected;
}

/**
 * @brief Get the RSSI of the WiFi Access Point
 * @return rssi_value
 */
int8_t wifi_drv_get_rssi() {
    wifi_ap_record_t wifi_ap_info;
    ESP_RETURN_ON_ERROR(esp_wifi_sta_get_ap_info(&wifi_ap_info), TAG, "Failed to get AP info (RSSI)");
    return (wifi_ap_info.rssi);
}

/**
 * @brief Initialize, configure and start Wi-Fi in a station mode
 * @return Error code
 */
esp_err_t wifi_drv_init() {
    // Create an LwIP core task and initialize LwIP-related work
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "Failed to initialise LwIP (NETIF)");
    // Create an event loop to handle events from the WiFi task
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "Failed to create a default event loop");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  // Use default configuration parameters
    // Initialize WiFi, allocate resource, start WiFi task
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to initialise WiFi");
    ESP_LOGI(TAG, "WiFi initialised sucessfully");

    // Register an event handler for WIFI and IP events
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL), TAG, "Failed to register an event handler for WiFi");
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL), TAG, "Failed to register an event handler for IP events");

    // Initialize default station as network interface instance (esp-netif)
    esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // Define WiFi Station configuration
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .scan_method = WIFI_FAST_SCAN,             // Use fast scan, i.e. end after find SSID match AP
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,  // Sort AP by RSSI
            .threshold.rssi = (int8_t)(-127),          // Weakest RSSI to be considered
            .threshold.authmode = WIFI_AUTH_OPEN,      // Weakest authentication mode (no security)
        },
    };

    // Set mode to station and set WiFi configuration
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set the WiFi mode to STA");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "Failed to set WiFi config");
    ESP_LOGI(TAG, "WiFi configured sucessfully");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start the WiFi");  // Start WiFi
    ESP_LOGI(TAG, "WiFi started sucessfully");

    return ESP_OK;
}
