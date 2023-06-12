/**
 * @file    data_scraping.c
 * @brief   Establish SSL/TLS connection and extract frequency data from HTTP response
 * @author  Karol Wojslaw (wojslaw.tech@gmail.com)
 */

#include "data_scraping.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_crt_bundle.h"
#include "freertos/FreeRTOS.h"
#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"

#define TAG "data_scraping"

mbedtls_ssl_context ssl;            // SSL/TLS context
mbedtls_net_context server_fd;      // Network context
mbedtls_entropy_context entropy;    // Context for entropy source
mbedtls_ctr_drbg_context ctr_drbg;  // Context for deterministic random bit generator
mbedtls_x509_crt cacert;            // Certificate structure
mbedtls_ssl_config conf;            // SSL/TLS configuration structure

static const char *REQUEST = "GET " WEB_URL
                             " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER
                             "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

/**
 * @brief Extracts frequency data from a response string.
 *
 * @param response       The response string to search for the frequency data.
 * @param response_size  The size of the response buffer.
 * @param freq           Pointer to a float variable where the extracted frequency will be stored.
 *
 * @return ESP_OK if the frequency data is successfully extracted.
 * ESP_ERR_INVALID_ARG if `response` is NULL or `buffer_size` is 0.
 */
static esp_err_t extract_freq_data(char *response, size_t response_size, float* freq) {
    char temp_buff[TEMP_BUFFER_SIZE];
    if (response == NULL || freq == NULL) {
        return ESP_ERR_INVALID_ARG;
    } else if (response_size == 0) {
        return ESP_ERR_INVALID_SIZE;
    }

    char* r = response;
    for(int i = 0; i < (response_size-11-TEMP_BUFFER_SIZE); i++){
        if((*(r+i) == 'F') && (*(r+i+1) == 'r' ) && (*(r+i+2) == 'e') && (*(r+i+3) == 'q')) {
            memcpy(temp_buff, (r+i+11), (size_t) (TEMP_BUFFER_SIZE-1));
            ESP_LOGD(TAG, "Freq data coppied to temp buffer: %s", temp_buff);
            sscanf(temp_buff, " %f", freq);
            break;
        }
    }
    return ESP_OK;
}

/**
 * @brief Reset the mbedtls context and free network resources.
 */
static void mbedtls_reset(void) {
    mbedtls_ssl_session_reset(&ssl);
    mbedtls_net_free(&server_fd);
}

/**
 * @brief Get frequency data by establishing an SSL/TLS connection with the server and sending an HTTP request.
 */
esp_err_t data_scraping_get_freq(float* freq) {
    esp_err_t err = ESP_OK;
    int ret, flags, len;
    char buf[HTTP_BUFFER_SIZE];

    mbedtls_net_init(&server_fd);   // Initialize network context
    ESP_LOGI(TAG, "Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                   WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0) {
        ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
        mbedtls_reset();
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "Connected.");
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
            mbedtls_reset();
            return ESP_FAIL;
        }
    }

    ESP_LOGI(TAG, "Verifying peer X.509 certificate...");
    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0) {
        ESP_LOGW(TAG, "Failed to verify peer certificate!");
    } else {
        ESP_LOGI(TAG, "Certificate verified.");
    }

    ESP_LOGI(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));
    ESP_LOGI(TAG, "Writing HTTP request...");

    size_t written_bytes = 0;
    do {
        ret = mbedtls_ssl_write(&ssl,
                                (const unsigned char *)REQUEST + written_bytes,
                                strlen(REQUEST) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
            ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
            mbedtls_reset();
            return ESP_FAIL;
        }
    } while (written_bytes < strlen(REQUEST));

    ESP_LOGI(TAG, "Reading HTTP response...");

    while(true) {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        } else if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
            ret = 0;
            break;
        } else if (ret < 0) {
            ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
            break;
        } else if (ret == 0) {
            ESP_LOGI(TAG, "connection closed");
            break;
        }

        len = ret;
        ESP_LOGI(TAG, "%d bytes read", len);

        err = extract_freq_data(buf, len, freq);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error 0x%d encountered when extracting frequency data", ret);
        } else {
            ESP_LOGI(TAG, "Frequency data extracted sucessfully");
        }
    }

    mbedtls_ssl_close_notify(&ssl);
    mbedtls_reset(); 
    return err; 
}

/**
 * @brief Initialize the data scraping functionality.
 *
 * @return ESP_OK if the initialization is successful. ESP_FAIL otherwise
 */
esp_err_t data_scraping_init(void) {
    int ret;

    mbedtls_ssl_init(&ssl);             // Initialize SSL/TLS context
    mbedtls_x509_crt_init(&cacert);     // Initialize certificate structure
    mbedtls_ctr_drbg_init(&ctr_drbg);   // Initialize deterministic random bit generator
    ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf); // Initialize SSL/TLS configuration
    
    mbedtls_entropy_init(&entropy); // Initialize entropy source
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Attaching the certificate bundle...");
    ESP_ERROR_CHECK(esp_crt_bundle_attach(&conf));  // Attach the certificate bundle

    if (ret < 0) {
        ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
        return ESP_FAIL;
    }

    /* Set Hostname matching CN in server certificate */
    ESP_LOGI(TAG, "Setting hostname for TLS session...");
    if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        mbedtls_reset();
        return ESP_FAIL;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);      // Set authentication mode
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);                    // Set CA chain
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);    // Set random number generator

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        mbedtls_reset();
        return ESP_FAIL;
    }
    return ESP_OK; 
}
