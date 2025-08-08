/* #include <M5Atom.h>
#include "ATOM_DTU_CAT1.h"
#include <PubSubClient.h>
#include <TinyGsmClient.h>
#include <time.h>
#include <sys/time.h>

// MQTT cấu hình
#define MQTT_BROKER "mqtt.m5stack.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "ATOM_DTU_CAT1"
#define MQTT_PASSWORD "ATOM_DTU_CAT1_PWD"
#define MQTT_D_TOPIC "ATOM_DTU_CAT1/D"
#define MQTT_U_TOPIC "ATOM_DTU_CAT1/U"
#define UPLOAD_INTERVAL 10000

// GPRS cấu hình
const char apn[] = "YourAPN"; // <-- Đổi APN đúng theo SIM bạn dùng
const char gprsUser[] = "";
const char gprsPass[] = "";

// Đối tượng modem và MQTT
TinyGsm modem(SerialAT);
TinyGsmClient tcpClient(modem);
PubSubClient mqttClient(MQTT_BROKER, MQTT_PORT, tcpClient);

// Task handle
TaskHandle_t taskModemHandle = NULL;
TaskHandle_t taskMQTTHandle = NULL;

// Semaphore để đồng bộ giữa các task
SemaphoreHandle_t xMQTTReadySemaphore;

// MQTT callback
void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    char info[len + 1];
    memcpy(info, payload, len);
    info[len] = '\0';
    SerialMon.println("Message arrived [" + String(topic) + "]: " + info);
}

// MQTT kết nối
bool mqttConnect(void)
{
    SerialMon.println("Connecting to MQTT: " + String(MQTT_BROKER));
    String mqttid = "MQTTID_" + String(random(65536));

    if (!mqttClient.connect(mqttid.c_str(), MQTT_USERNAME, MQTT_PASSWORD))
    {
        SerialMon.println("MQTT CONNECT FAILED");
        return false;
    }

    SerialMon.println("MQTT CONNECTED!");
    mqttClient.setCallback(mqttCallback);
    mqttClient.publish(MQTT_U_TOPIC, "CATM MQTT CLIENT ONLINE");
    mqttClient.subscribe(MQTT_D_TOPIC);
    return true;
}

// Task khởi tạo modem + mạng
void TaskModemInit(void *pvParameters)
{
    (void)pvParameters;

    SerialMon.println(">> Initializing modem...");
    while (!modem.init())
    {
        SerialMon.println("...retrying modem.init()");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    SerialMon.println(">> Waiting for network...");
    while (!modem.waitForNetwork())
    {
        SerialMon.println("...waiting network");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    SerialMon.println(">> Connecting GPRS...");
    while (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        SerialMon.println("...retrying GPRS");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    SerialMon.println(">> GPRS connected.");

    // Cho phép MQTT task hoạt động
    xSemaphoreGive(xMQTTReadySemaphore);

    vTaskDelete(NULL);
}

// Task xử lý MQTT loop
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
                mqttClient.publish(MQTT_U_TOPIC, "Hello from FreeRTOS + Semaphore");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Setup hệ thống
void setup()
{
    M5.begin();
    SerialAT.begin(SIM7680_BAUDRATE, SERIAL_8N1, ATOM_DTU_SIM7680_RX, ATOM_DTU_SIM7680_TX);
    SerialMon.begin(115200);

    SerialMon.println("System Start...");

    // Tạo binary semaphore
    xMQTTReadySemaphore = xSemaphoreCreateBinary();
    if (xMQTTReadySemaphore == NULL)
    {
        SerialMon.println("Failed to create semaphore!");
        while (1)
            ;
    }

    // Tạo task modem init
    xTaskCreatePinnedToCore(TaskModemInit, "TaskModemInit", 4096, NULL, 1, &taskModemHandle, 1);

    // Tạo task MQTT loop
    xTaskCreatePinnedToCore(TaskMQTTLoop, "TaskMQTTLoop", 4096, NULL, 1, &taskMQTTHandle, 1);
}

void loop()
{
    // Không cần code gì ở đây vì mọi thứ dùng FreeRTOS task
}
 */