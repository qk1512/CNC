#include "atomNow.h"

uint8_t peerAddress[] = {0xDC, 0x54, 0x75, 0xCE, 0xAC, 0xD8}; // MAC đích

void initEspNow()
{
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW Init Failed");
        return;
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    esp_now_add_peer(&peerInfo);
}

void formatSMData(char *buffer, size_t bufferSize)
{
    snprintf(buffer, bufferSize,
             "%.2f_%.d_%.2f_%.2f_%.d_%.d_%.d",
             SM_sensor.PH_values,
             SM_sensor.soil_conductivity,
             SM_sensor.temperature,
             SM_sensor.soil_moisture,
             SM_sensor.soil_nitrogen,
             SM_sensor.soil_phosphorus,
             SM_sensor.soil_potassium);
}

void sendSensorDataTask(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(sensorSemaphore, pdMS_TO_TICKS(100)))
        {
            if (SM_sensor.data_updated)
            {
                char dataBuffer[128]; // đủ lớn để chứa JSON
                formatSMData(dataBuffer, sizeof(dataBuffer));
                esp_now_send(peerAddress, (uint8_t *)dataBuffer, strlen(dataBuffer));
                SM_sensor.data_updated = false;
            }
            xSemaphoreGive(sensorSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Gửi mỗi 2 giây
    }
}
