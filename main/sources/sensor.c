#include "sensor.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#define TAG "SENSOR"

#define SDA 21
#define SCL 22

#define AM2320 0x5c
#define READ_CMD 0x03
#define START_REGISTER 0x00

xSemaphoreHandle onDataReadSemaphore;
static xSemaphoreHandle onPollingSemaphore;
static void (*polledCallback)(void);

static void init_i2c_bus()
{
    ESP_LOGI(TAG, "configuring i2c...");
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA,
        .scl_io_num = SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000};

    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

static uint16_t CRC16(uint8_t *ptr, uint8_t length)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    while (length--)
    {
        crc ^= *ptr++;
        for (i = 0; i < 8; i++)
            if ((crc & 0x01) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
    }
    return crc;
}

static void on_polling_timer(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "on_polling_timer called");
    xSemaphoreGive(onPollingSemaphore);
}

static void listen_for_polling(void *args)
{
    init_i2c_bus();

    while (true)
    {
        xSemaphoreTake(onPollingSemaphore, portMAX_DELAY);
        ESP_LOGI(TAG, "onPollingSemaphore took");
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (AM2320 << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, READ_CMD, true);
        i2c_master_write_byte(cmd, START_REGISTER, true);
        i2c_master_write_byte(cmd, 0x04, true);

        uint8_t raw[8];
        for (int i = 0; i < 8; i++)
            raw[i] = 0;

        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (AM2320 << 1) | I2C_MASTER_READ, true);
        i2c_master_read(cmd, raw, 8, I2C_MASTER_ACK);
        i2c_master_stop(cmd);

        i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);

        uint16_t crc = raw[7] << 8 | raw[6];
        if (crc != CRC16(raw, 6))
        {
            ESP_LOGW(TAG, "crc check failed");
        }
        else
        {
            humidity = (raw[2] << 8 | raw[3]) / 10.0;
            temperature = (raw[4] << 8 | raw[5]) / 10.0;

            if (polledCallback != NULL)
                polledCallback();
        }
    }
}

void sensor_start_polling(void (*cb)(void))
{
    ESP_LOGI(TAG, "sensor_start_polling...");
    polledCallback = cb;

    onPollingSemaphore = xSemaphoreCreateBinary();

    TimerHandle_t xTimer = xTimerCreate("polling timer", pdMS_TO_TICKS(1000), true, NULL, on_polling_timer);
    xTimerStart(xTimer, 0);

    xTaskCreate(listen_for_polling, "listen_for_polling", 2048, NULL, 2, NULL);
}
