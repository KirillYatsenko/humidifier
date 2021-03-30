#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "cJSON.h"

#include "humidifier.h"
#include "settings.h"
#include "sensor.h"
#include "storage.h"

#define TAG "SERVER"

static esp_err_t on_url_hit(httpd_req_t *req)
{
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    esp_vfs_spiffs_register(&config);

    ESP_LOGI(TAG, "url %s was hit", req->uri);
    char path[600];
    sprintf(path, "/spiffs%s", req->uri);
    if (strcmp(path, "/spiffs/") == 0)
    {
        sprintf(path, "/spiffs/%s", "index.html");
    }
    //style.css
    char *ptr = strrchr(path, '.');
    if (strcmp(ptr, ".css") == 0)
    {
        ESP_LOGI(TAG, "setting mime type to css");
        httpd_resp_set_type(req, "text/css");
    }
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        httpd_resp_send_404(req);
        return ESP_OK;
    }

    char lineRead[256];
    while (fgets(lineRead, sizeof(lineRead), file))
    {
        httpd_resp_sendstr_chunk(req, lineRead);
    }
    httpd_resp_sendstr_chunk(req, NULL);
    esp_vfs_spiffs_unregister(NULL);
    return ESP_OK;
}

static esp_err_t on_get_info(httpd_req_t *req)
{
    sensor_event_t *latest_info = sensor_get_latest();
    char message[100];
    sprintf(message, "{\"humidity\": %.1f,"
                     "\"temperature\": %.1f,"
                     "\"status\": %d,"
                     "\"autostart_enable\": %d,"
                     "\"autostart_value\": %d}",
            latest_info->humidity,
            latest_info->temperature,
            humidifier_status,
            settings_info.autostart_enable,
            settings_info.autostart_value);

    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

static esp_err_t on_humidifier_turn_on(httpd_req_t *req)
{
    humidifier_set_status(true);
    settings_info.autostart_enable = false;

    char *message = "{\"status\":\"ok\"}";

    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

static esp_err_t on_humidifier_turn_off(httpd_req_t *req)
{
    humidifier_set_status(false);
    settings_info.autostart_enable = false;

    char *message = "{\"status\":\"ok\"}";

    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

static esp_err_t on_change_autostart_settings(httpd_req_t *req)
{
    ESP_LOGI(TAG, "url %s was hit", req->uri);
    char buf[150];
    memset(&buf, 0, sizeof(buf));
    httpd_req_recv(req, buf, req->content_len);

    cJSON *payload = cJSON_Parse(buf);
    cJSON *autostartJson = cJSON_GetObjectItem(payload, "autostart_enable");
    cJSON *autostartValueJson = cJSON_GetObjectItem(payload, "autostart_value");

    bool autostart_enable = cJSON_IsTrue(autostartJson);
    int autostart_value = autostartValueJson->valueint;

    cJSON_Delete(payload);

    ESP_LOGI(TAG, "buf: %s", buf);
    ESP_LOGI(TAG, "autostart_enable: %s", autostart_enable ? "true" : "false");
    ESP_LOGI(TAG, "autostart_value: %d", autostart_value);

    if (autostart_value < 1 || autostart_value > 100)
    {
        ESP_LOGE(TAG, "autostart_value validation failed");
        return ESP_OK;
    }

    settings_info.autostart_enable = autostart_enable;
    settings_info.autostart_value = autostart_value;
    storage_save_settings();

    char *message = "{\"status\":\"ok\"}";
    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

void server_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "starting server");
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to start http server");
    }

    httpd_uri_t sensor_info_end_point_config = {
        .uri = "/api/info",
        .method = HTTP_GET,
        .handler = on_get_info};

    httpd_uri_t humidifier_turn_on_end_point_config = {
        .uri = "/api/turn_on",
        .method = HTTP_POST,
        .handler = on_humidifier_turn_on};

    httpd_uri_t humidifier_turn_off_end_point_config = {
        .uri = "/api/turn_off",
        .method = HTTP_POST,
        .handler = on_humidifier_turn_off};

    httpd_uri_t humidifier_autostart_end_point_config = {
        .uri = "/api/autostart",
        .method = HTTP_POST,
        .handler = on_change_autostart_settings};

    httpd_uri_t first_end_point_config = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = on_url_hit};

    httpd_register_uri_handler(server, &sensor_info_end_point_config);
    httpd_register_uri_handler(server, &humidifier_turn_on_end_point_config);
    httpd_register_uri_handler(server, &humidifier_turn_off_end_point_config);
    httpd_register_uri_handler(server, &humidifier_autostart_end_point_config);
    httpd_register_uri_handler(server, &first_end_point_config);
}