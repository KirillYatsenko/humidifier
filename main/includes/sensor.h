#ifndef SENSOR_H
#define SENSOR_H

double temperature;
double humidity;

void sensor_start_polling(void (*cb)(void));

#endif