#include <stdio.h>
#include "esp_log.h"
#include "sensor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MAIN"

void sensor_evt_task(void *arg)
{
  while (1)
  {
    sensor_event_t evt;
    xQueueReceive(sensor_queue, &evt, portMAX_DELAY);

    ESP_LOGI(TAG, "humidity = %.2f%%", evt.humidity);
    ESP_LOGI(TAG, "temperature = %.2f*C", evt.temperature);
  }
}

void app_main(void)
{
  sensor_start_polling();
  xTaskCreate(sensor_evt_task, "sensor_evt_task", 2048, NULL, 5, NULL);

}
