#include "sensor.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "DHT.h"

#define TAG "SENSOR"

static sensor_event_t latest_info;
static xSemaphoreHandle onPollingSemaphore;

static void init_communication()
{
    ESP_LOGI(TAG, "configuring communication with DHT...");
    gpio_pad_select_gpio(GPIO_NUM_27);
    setDHTgpio(GPIO_NUM_27);

    // powering DHT
    gpio_pad_select_gpio(GPIO_NUM_12);
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_12, 1);
}

static void on_polling_timer(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "on_polling_timer called");
    xSemaphoreGive(onPollingSemaphore);
}

static void listen_for_polling(void *args)
{
    ESP_LOGI(TAG, "listen_for_polling called");

    init_communication();

    while (true)
    {
        xSemaphoreTake(onPollingSemaphore, portMAX_DELAY);
        ESP_LOGI(TAG, "onPollingSemaphore took");

        int ret = readDHT();
        errorHandler(ret);

        sensor_event_t sensor_info;

        sensor_info.humidity = getHumidity();
        sensor_info.temperature = getTemperature();
        xQueueSend(sensor_queue, &sensor_info, 0);

        latest_info = sensor_info;
    }
}

void sensor_start_polling(void)
{
    ESP_LOGI(TAG, "sensor_start_polling...");

    onPollingSemaphore = xSemaphoreCreateBinary();
    sensor_queue = xQueueCreate(10, sizeof(sensor_event_t));

    TimerHandle_t xTimer = xTimerCreate("polling timer", pdMS_TO_TICKS(3000), true, NULL, on_polling_timer);
    xTimerStart(xTimer, 0);

    xTaskCreate(listen_for_polling, "listen_for_polling", 2048, NULL, configMAX_PRIORITIES -1, NULL);
}

sensor_event_t *sensor_get_latest(void)
{
    return &latest_info;
}