#include "mqtt_client.h"
//#define TINY_GSM_MODEM_SIM7600
uint32_t lastReconnectAttempt = 0;

TaskHandle_t taskModemHandle = NULL;
TaskHandle_t taskMQTTHandle = NULL;
SemaphoreHandle_t xMQTTReadySemaphore;

TinyGsm modem(SerialAT);
TinyGsmClient tcpClient(modem);
PubSubClient mqttClient(tcpClient);

const char apn[] = "v-internet";
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
    Serial.println("Connecting to MQTT: " + String(MQTT_BROKER));
    String mqttid = "MQTTID_" + String(random(65536));

    if(!mqttClient.connect(mqttid.c_str(), ACCESS_TOKEN, "")){
        Serial.println("MQTT CONNECT FAILED");
        return false;
    }

    Serial.println("MQTT CONNECTED!");
    mqttClient.setCallback(mqttCallback);
    mqttClient.publish(MQTT_PUB_TOPIC, "CATM MQTT CLIENT ONLINE");
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

    // Đợi modem init xong
    if (xSemaphoreTake(xMQTTReadySemaphore, portMAX_DELAY) == pdTRUE)
    {
        SerialMon.println(">> MQTT Task Starting...");
    }

    uint32_t timer = millis();
    uint32_t lastReconnectAttempt = 0;

    while (true)
    {
        if (!mqttClient.connected())
        {
            uint32_t now = millis();
            if (now - lastReconnectAttempt > 3000)
            {
                lastReconnectAttempt = now;
                mqttConnect();
            }
        }
        else
        {
            mqttClient.loop();

            if (millis() - timer >= UPLOAD_INTERVAL)
            {
                timer = millis();
                mqttClient.publish(MQTT_PUB_TOPIC, "Hello from FreeRTOS + Semaphore");
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

    xTaskCreatePinnedToCore(TaskMQTTLoop, "TaskMQTTLoop", 4096, NULL, 1, &taskMQTTHandle, 1);
}