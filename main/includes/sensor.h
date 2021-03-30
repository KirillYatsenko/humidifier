#ifndef SENSOR_H
#define SENSOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct
{
    double temperature;
    double humidity;
} sensor_event_t;

xQueueHandle sensor_queue;

void sensor_start_polling(void);
sensor_event_t* sensor_get_latest(void);

#endif