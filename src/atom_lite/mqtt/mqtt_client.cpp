#include "mqtt_client.h"
//#define TINY_GSM_MODEM_SIM7600
uint32_t lastReconnectAttempt = 0;

TaskHandle_t taskModemHandle = NULL;
TaskHandle_t taskMQTTHandle = NULL;
SemaphoreHandle_t xMQTTReadySemaphore;

TinyGsm modem(SerialAT);
TinyGsmClient tcpClient(modem);
PubSubClient mqttClient(MQTT_BROKER,MQTT_PORT,tcpClient);

char payload[256];

void prepareSoilMoistureJson()
{
    StaticJsonDocument<256> doc;
    char buffer[16];

    snprintf(buffer,sizeof(buffer), "%.2f", SM_sensor.temperature);
    doc["temperature"] = buffer;

    snprintf(buffer,sizeof(buffer), "%.2f", SM_sensor.PH_values);
    doc["ph"] = buffer;

    snprintf(buffer,sizeof(buffer), "%.2f", SM_sensor.soil_moisture);
    doc["moisture"] = buffer;

    doc["conductivity"] = SM_sensor.soil_conductivity;
    doc["nitrogen"] = SM_sensor.soil_nitrogen;
    doc["phosphorus"] = SM_sensor.soil_phosphorus;
    doc["potassium"] = SM_sensor.soil_potassium;

    memset(payload, 0, sizeof(payload));
    size_t len = serializeJson(doc,payload,sizeof(payload));
    //serializeJson(doc, payload);
    Serial.printf("Json length: %u bytes\n", (unsigned)len);
}

const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    char info[len + 1];
    memcpy(info, payload, len);
    info[len] = '\0';
    Serial.println("Message arrived[" + String(topic) + "]:" + info);
}

bool mqttConnect(void)
{
    //Serial.println("Connecting to MQTT: " + String(MQTT_BROKER));
    String mqttid = "MQTTID_" + String(random(65536));

    if(!mqttClient.connect(mqttid.c_str(), ACCESS_TOKEN, "")){
        Serial.println("MQTT CONNECT FAILED");
        return false;
    }

    //Serial.println("MQTT CONNECTED!");
    mqttClient.setCallback(mqttCallback);
    //mqttClient.publish(MQTT_PUB_TOPIC, "CATM MQTT CLIENT ONLINE");
    mqttClient.subscribe(MQTT_SUB_TOPIC);
    return true;
}

void TaskModemInit(void *pvParameters)
{
    (void)pvParameters;

    Serial.println(">> Initializing modem...");
    while(!modem.init())
    {
        Serial.println("...retrying modem.init()");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    Serial.println(">> Waiting for network...");
    while(!modem.waitForNetwork())
    {
        Serial.println("...waiting network");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    Serial.println(">> Connecting GPRS...");
    while(!modem.gprsConnect(apn,gprsUser,gprsPass))
    {
        Serial.println("...retrying GPRS");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }


    Serial.println(">> GPRS connected.");

    xSemaphoreGive(xMQTTReadySemaphore);
    vTaskDelete(NULL);
}

void TaskMQTTLoop(void *pvParameters)
{
    (void)pvParameters;

    if (xSemaphoreTake(xMQTTReadySemaphore, portMAX_DELAY) == pdTRUE)
    {
        Serial.println(">> MQTT Task Starting...");
    }

    uint32_t lastReconnectAttempt = 0;
    uint32_t lastPublish = 0;

    while (true)
    {
        if (!mqttClient.connected())
        {
            uint32_t now = millis();
            if (now - lastReconnectAttempt > 3000)
            {
                lastReconnectAttempt = now;
                if (mqttConnect())
                {
                    lastReconnectAttempt = 0;
                }
            }
        }
        else
        {
            mqttClient.loop();

            uint32_t now = millis();
            if (now - lastPublish >= UPLOAD_INTERVAL)
            {
                lastPublish = now;

                // Tạo JSON dữ liệu
                prepareSoilMoistureJson();

                // Publish dữ liệu
                if (mqttClient.publish(MQTT_PUB_TOPIC, payload))
                {
                    Serial.println("[MQTT] Data published: " + String(payload));
                }
                else
                {
                    Serial.println("[MQTT] Publish failed!");
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void mqttInit()
{
    SerialAT.begin(SIM7680_BAUDRATE, SERIAL_8N1, ATOM_DTU_SIM7680_RX, ATOM_DTU_SIM7680_TX);

    xMQTTReadySemaphore = xSemaphoreCreateBinary();
    if (xMQTTReadySemaphore == NULL)
    {
        SerialMon.println("Failed to create semaphore!");
        while (1)
            ;
    }

    xTaskCreatePinnedToCore(TaskModemInit, "TaskModemInit", 4096, NULL, 1, &taskModemHandle, 1);

    xTaskCreatePinnedToCore(TaskMQTTLoop, "TaskMQTTLoop", 4096, NULL, 1, &taskMQTTHandle, 0);
}