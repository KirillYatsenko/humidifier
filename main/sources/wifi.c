#include <stdio.h>
#include <tcpip_adapter.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "cJSON.h"

#define SSID CONFIG_SSID
#define PASSWORD CONFIG_PASSWORD

#define TAG "WIFI"

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "connecting...\n");
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "connected\n");
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip\n");
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "disconnected\n");
        break;

    default:
        break;
    }
    return ESP_OK;
}

// Wi-Fi Access Point
esp_err_t wifi_start_ap()
{
    ESP_LOGI(TAG, "wifi_start_ap called");
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(NULL, NULL));

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    // ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi ssid = %s", SSID);
    ESP_LOGI(TAG, "wifi password = %s", PASSWORD);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = SSID,
            .password = PASSWORD,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK}};
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);

    esp_err_t res = esp_wifi_start();

    // IP address.
    tcpip_adapter_ip_info_t ipInfo;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
    ESP_LOGI(TAG, "My IP: " IPSTR, IP2STR(&ipInfo.ip));

    return res;
}

// Wi-Fi Connect
esp_err_t wifi_connect_ap(void)
{
    ESP_LOGI(TAG, "wifi_connect_ap called");
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    ESP_LOGI(TAG, "wifi ssid = %s", SSID);
    ESP_LOGI(TAG, "wifi password = %s", PASSWORD);

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_config =
        {
            .sta = {
                .ssid = SSID,
                .password = PASSWORD}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    esp_err_t res = esp_wifi_start();
    // IP address.
    tcpip_adapter_ip_info_t ipInfo;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ipInfo);
    ESP_LOGI(TAG, "My IP: " IPSTR, IP2STR(&ipInfo.ip));

    return res;
}