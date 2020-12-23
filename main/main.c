#include <stdio.h>
#include "esp_log.h"
#include "sensor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MAIN"

void cb(void)
{
  ESP_LOGI(TAG, "humidity = %.2f%%", humidity);
  ESP_LOGI(TAG, "temperature = %.2f*C", temperature);
}

void app_main(void)
{
  sensor_start_polling(cb);
}
