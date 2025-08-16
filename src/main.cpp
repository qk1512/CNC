#include <M5Atom.h>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>
//#include "ATOM_DTU_CAT1.h"

#include "./atom_lite/sensor/soilMoisture.h"
#include "./atom_lite/esp_now/atomNow.h"
#include "./atom_lite/mqtt/mqtt_client.h"

RS485Class RS485(Serial2, ATOM_DTU_RS485_RX, ATOM_DTU_RS485_TX, -1, -1);

TaskHandle_t readTaskHandle, espnowTaskHandle;

void readSensorTask(void *pvParameters)
{
  while(1)
  {
    SMReadData(ModbusRTUClient);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void setup()
{
  M5.begin(true, false, true);
  Serial.begin(115200);

  ModbusRTUClient.begin(9600);
  /* if (!ModbusRTUClient.begin(RS485,9600))
  {
    Serial.println("Failed to start Modbus RTU Client");
  } */
  ModbusRTUClient.setTimeout(2000);
  vTaskDelay(pdMS_TO_TICKS(1000));
    
  sensorSemaphore = xSemaphoreCreateMutex();

  //vTaskDelay(pdMS_TO_TICKS(1000)); // FreeRTOS-friendly delay
  initEspNow();
  vTaskDelay(pdMS_TO_TICKS(1000)); // FreeRTOS-friendly delay
  mqttInit();

  xTaskCreatePinnedToCore(readSensorTask, "ReadSensor", 4096, NULL , 1 , &readTaskHandle, 1);
  xTaskCreatePinnedToCore(sendSensorDataTask, "SendESPNow", 4096, NULL, 1, &espnowTaskHandle, 1);
  
}

void loop()
{
  
}