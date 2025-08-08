#include <M5Atom.h>
#include <ArduinoModbus.h>
#include <ArduinoRS485.h>
//#include "ATOM_DTU_CAT1.h"

#include "./atom_lite/sensor/soilMoisture.h"
#include "./atom_lite/esp_now/atomNow.h"
#include "./atom_lite/mqtt/mqtt_client.h"

RS485Class RS485(Serial2, ATOM_DTU_RS485_RX, ATOM_DTU_RS485_TX, -1, -1);

TaskHandle_t readTaskHandle, espnowTaskHandle, mqttTaskHandle;

char payload[256];

void prepareSoilMoistureJson()
{
  StaticJsonDocument<256> doc;

  doc["temperature"] = SM_sensor.temperature;
  doc["ph"] = SM_sensor.PH_values;
  doc["moisture"] = SM_sensor.soil_moisture;
  doc["conductivity"] = SM_sensor.soil_conductivity;
  doc["nitrogen"] = SM_sensor.soil_nitrogen;
  doc["phosphorus"] = SM_sensor.soil_phosphorus;
  doc["potassium"] = SM_sensor.soil_potassium;

  serializeJson(doc,payload);
}

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

  //ModbusRTUClient.begin(9600);
  //ModbusRTUClient.setTimeout(2000);
  //SerialAT.begin(SIM7680_BAUDRATE, SERIAL_8N1, ATOM_DTU_SIM7680_RX, ATOM_DTU_SIM7680_TX);
  delay(1000);

  
  //sensorSemaphore = xSemaphoreCreateMutex();
  

  //delay(1000);
  //initEspNow();
  mqttInit();
  /* if (!ModbusRTUClient.begin(9600, SERIAL_8N1))
  {
    Serial.println("Failed to start Modbus RTU Client!");
    while (1)
      ;
  } */

  //xTaskCreatePinnedToCore(readSensorTask, "ReadSensor", 4096, NULL , 1 , &readTaskHandle, 1);
  //xTaskCreatePinnedToCore(sendSensorDataTask, "SendESPNow", 4096, NULL, 1, &espnowTaskHandle, 1);
  //xTaskCreatePinnedToCore(mqttTask, "MQTTTask", 4096, NULL, 1, &mqttTaskHandle, 1);
  
  Serial.println("Modbus Client ready.");
}

void loop()
{
  
}