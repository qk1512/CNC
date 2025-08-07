#ifndef ESPNOW_H
#define ESPNOW_H

#include <esp_now.h>
#include <WiFi.h>
#include "../sensor/soilMoisture.h"

void initEspNow();
void sendSensorDataTask(void *pvParameters);

#endif
