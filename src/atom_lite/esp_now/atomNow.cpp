#include "atomNow.h"

uint8_t peerAddress[] = {0xDC, 0x54, 0x75, 0xCE, 0xAE, 0x50}; // MAC đích
                                                              // uint8_t peerAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

SemaphoreHandle_t xEspNowDoneSemaphore;

void initEspNow()
{
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ESP-NOW Init Failed");
        return;
    }
    else
    {
        Serial.println("ESP-NOW Init Successful");
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    esp_now_add_peer(&peerInfo);
    xEspNowDoneSemaphore = xSemaphoreCreateBinary();
}

void formatSMData(char *buffer, size_t bufferSize)
{
    snprintf(buffer, bufferSize,
             "%.2f_%d_%.2f_%.2f_%d_%d_%d",
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
    Serial.println("ESP is sending");
    while (1)
    {
        if (xSemaphoreTake(sensorSemaphore, pdMS_TO_TICKS(100)))
        {
            if (SM_sensor.data_updated)
            {
                char dataBuffer[128]; // đủ lớn để chứa JSON
                formatSMData(dataBuffer, sizeof(dataBuffer));
                //esp_now_send(peerAddress, (uint8_t *)dataBuffer, strlen(dataBuffer));
                esp_err_t result = esp_now_send(peerAddress, (uint8_t *)dataBuffer, strlen(dataBuffer));
                if (result == ESP_OK)
                {
                    Serial.println("ESP-NOW send success");
                }
                else
                {
                    Serial.printf("ESP-NOW send failed: %d\n", result);
                }

                //Serial.print("ESP Now send: ");
                //Serial.printf(dataBuffer,sizeof(dataBuffer));
                //Serial.println();
                
                SM_sensor.data_updated = false;
                Serial.printf("ESP Now is sent\n");
            }
            xSemaphoreGive(sensorSemaphore); // Trả lại semaphore ở đây
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // Gửi mỗi 10 giây
    }
}
