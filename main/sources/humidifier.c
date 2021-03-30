#include <stdio.h>
#include "humidifier.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define TAG "HUMIDIFIER"

#define CONTROL_PIN 23

static bool initialized = false;

static void init_gpio()
{
    if (initialized == true)
        return;

    ESP_LOGI(TAG, "initializing pin...");
    initialized = true;

    gpio_pad_select_gpio(CONTROL_PIN);
    gpio_set_direction(CONTROL_PIN, GPIO_MODE_OUTPUT);
}

void humidifier_set_status(bool status)
{
    init_gpio();

    ESP_LOGI(TAG, "humidifier_set_status called status = %d", status);
    gpio_set_level(CONTROL_PIN, status);

    humidifier_status = status;
}