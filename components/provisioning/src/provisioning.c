/**
 * @file    provisioning.c
 * @brief   Configure WiFi with ESP Provisioning Manager, connect to the AP, handle WiFi and IP events
 * @author  Karol Wojslaw (karol.wojslaw@student.manchester.ac.uk)
 */

#include "provisioning.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "qrcode.h"
#include "stdio.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

#define TAG "provisioning"

#define EXAMPLE_PROV_MGR_MAX_RETRY_CNT 5

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_SOFTAP "softap"
#define PROV_TRANSPORT_BLE "ble"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"

/* Event handler for catching system events */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    static int retries;
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG,
                         "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *)wifi_sta_cfg->ssid,
                         (const char *)wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG,
                         "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                retries++;
                if (retries >= EXAMPLE_PROV_MGR_MAX_RETRY_CNT) {
                    ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                    wifi_prov_mgr_reset_sm_state_on_failure();
                    retries = 0;
                }
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                retries = 0;
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
    }
}

/**
 * @brief Initializes Wi-Fi in station mode.
 *
 * @note Prior to calling this function, the necessary Wi-Fi configuration and credentials should be
 *       set using the appropriate API functions.
 */
static void wifi_init_sta(void) {
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief Generates a service name for the device based on the Wi-Fi MAC address.
 *
 * @param service_name Pointer to a buffer where the generated service name will be stored.
 * @param max          The maximum size of the service name buffer.
 */
static void get_device_service_name(char *service_name, size_t max) {
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

/**
 * @brief Generates and prints a QR code for provisioning.
 *
 * @param name      The device name for the QR code payload.
 * @param pop       The proof of possession (pop) for the QR code payload.
 * @param transport The transport type for the QR code payload.
 */
static void wifi_prov_print_qr(const char *name, const char *pop, const char *transport) {
    if (!name || !transport || !pop) {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};

    snprintf(payload, sizeof(payload),
            "{\"ver\":\"%s\",\"name\":\"%s\""
            ",\"pop\":\"%s\",\"transport\":\"%s\"}",
            PROV_QR_VERSION, name, pop, transport);
    
    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");

    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    esp_err_t qrcode_err = esp_qrcode_generate(&cfg, payload);
    if (qrcode_err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to generate QR code. Error code: %d", qrcode_err);
        return;
    }

    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}

/**
 * @brief Initializes provisioning and Wi-Fi connection.
 *
 * @param reset_provisioning Flag to reset Wi-Fi provisioning configuration if true.
 * @return ESP_OK if initialization is successful.
 */
esp_err_t provisioning_init(bool reset_provisioning) {
    ESP_ERROR_CHECK(esp_netif_init());  // Initialise TCP/IP Stack

    wifi_event_group = xEventGroupCreate();

    /* Register event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,                                       // Provision over BLE
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM  // Free BT Dual Mode mem after prov
    };

    /* Initialize provisioning manager with the configuration parameters set above */
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    /* If selected, reset Wi-Fi prov config (restore WiFi stack persistent settings to defaults) */
    if (reset_provisioning) {
        wifi_prov_mgr_reset_provisioning();
        ESP_LOGI(TAG, "Reseting WiFi provisioning configuration");
    }

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        ESP_LOGI(TAG, "Starting WiFi provisioning");

        char service_name[12];  // What is the Device Service Name (BLE device name)
        get_device_service_name(service_name, sizeof(service_name));

        /*  Use secure communication which consists of secure handshake using X25519 key exchange
         *  and proof of possession (pop) and AES-CTR for encryption/decryption of messages.
         */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        const char *pop = "0000";       // Proof-of-possession for security
        const char *service_key = NULL; // Ignored for provisioning over BLE

        // Set a custom 128 bit GATT service UUID (LSB <-> MSB)
        uint8_t custom_service_uuid[] = {
            0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);

        /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key));

        /* Print QR code for provisioning */
        wifi_prov_print_qr(service_name, pop, PROV_TRANSPORT_BLE);
    } else {
        ESP_LOGI(TAG, "WiFi already provisioned, starting Wi-Fi STA");
        wifi_prov_mgr_deinit();  // Release Prov manager resources, as it's not needed
        wifi_init_sta();         // Start Wi-Fi station
    }

    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
    return ESP_OK;
}
