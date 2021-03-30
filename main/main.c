#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor.h"
#include "wifi.h"
#include "server.h"
#include "settings.h"
#include "humidifier.h"
#include "storage.h"

#define TAG "MAIN"

void sensor_evt_task(void *arg)
{
  while (1)
  {
    sensor_event_t evt;
    xQueueReceive(sensor_queue, &evt, portMAX_DELAY);

    ESP_LOGI(TAG, "humidity = %.2f%%", evt.humidity);
    ESP_LOGI(TAG, "temperature = %.2f*C", evt.temperature);

    if (settings_info.autostart_enable)
    {
      humidifier_set_status(evt.humidity < settings_info.autostart_value);
    }
  }
}

void app_main(void)
{
  storage_load_settings();
  sensor_start_polling();
  xTaskCreate(sensor_evt_task, "sensor_evt_task", 2048, NULL, 5, NULL);

  wifi_connect_ap(); // Change to wifi_start_ap() in production
  server_start();
}
