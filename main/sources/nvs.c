
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "settings.h"

#define TAG "STORAGE"

static bool initialized = false;

static void initialize(void)
{
    if (initialized)
        return;

    ESP_LOGI(TAG, "initializing...");
    ESP_ERROR_CHECK(nvs_flash_init());

    initialized = true;
}

void storage_load_settings(void)
{
    ESP_LOGI(TAG, "storage_load_settings called");
    initialize();

    nvs_handle handle;
    ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &handle));

    uint8_t autostart_enable = 0;
    uint8_t autostart_value = 1;

    esp_err_t result = nvs_get_u8(handle, "stngs_en", &autostart_enable);
    nvs_get_u8(handle, "stngs_val", &autostart_value);

    switch (result)
    {
    case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGE(TAG, "autostart settings not set yet");
        break;
    case ESP_OK:
        ESP_LOGI(TAG, "autostart_enable: %d; autostart_value: %d",
                 autostart_enable, autostart_value);

        settings_info.autostart_enable = autostart_enable;
        settings_info.autostart_value = autostart_value;

        break;
    default:
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
        break;
    }

    nvs_close(handle);
}

void storage_save_settings(void)
{
    ESP_LOGI(TAG, "storage_save_settings called");
    initialize();

    nvs_handle handle;
    ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &handle));

    // Names are short because of ESP_ERR_NVS_KEY_TOO_LONG error
    ESP_ERROR_CHECK(nvs_set_u8(handle, "stngs_en", settings_info.autostart_enable));
    ESP_ERROR_CHECK(nvs_set_u8(handle, "stngs_val", settings_info.autostart_value));

    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
}