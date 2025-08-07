#include <M5Atom.h>
#include "ATOM_DTU_CAT1.h"
#include <PubSubClient.h>
#include <TinyGsmClient.h>
#include <time.h>
#include <sys/time.h>

#define SerialMon Serial

#define MQTT_BROKER "app.coreiot.io"
#define MQTT_PORT 1883
#define ACCESS_TOKEN "YOUR_ACCESS_TOKEN_HERE" // <-- THAY BẰNG ACCESS TOKEN TỪ COREGATE
#define MQTT_PUB_TOPIC "v1/devices/me/telemetry"
#define MQTT_SUB_TOPIC "v1/devices/me/attributes"

#define UPLOAD_INTERVAL 10000

// GPRS (Viettel)
const char apn[] = "v-internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Modem
TinyGsm modem(SerialAT);
TinyGsmClient tcpClient(modem);
PubSubClient mqttClient(tcpClient);

uint32_t lastReconnectAttempt = 0;
char s_time[50];

// Hàm forward
void mqttCallback(char *topic, byte *payload, unsigned int len);
bool mqttConnect(void);
void InitNetwork(void);

void setup()
{
    M5.begin();
    SerialMon.begin(115200);
    SerialAT.begin(SIM7680_BAUDRATE, SERIAL_8N1, ATOM_DTU_SIM7680_RX, ATOM_DTU_SIM7680_TX);

    SerialMon.println("Starting...");

    InitNetwork();

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

void loop()
{
    static unsigned long timer = 0;

    if (!mqttClient.connected())
    {
        SerialMon.println(">> MQTT NOT CONNECTED");
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 3000L)
        {
            lastReconnectAttempt = t;
            if (mqttConnect())
            {
                lastReconnectAttempt = 0;
            }
        }
        delay(100);
        return;
    }

    mqttClient.loop();

    if (millis() > timer)
    {
        timer = millis() + UPLOAD_INTERVAL;

        // Giả lập dữ liệu JSON gửi lên
        String payload = "{\"temperature\": 26.5, \"humidity\": 60}";
        mqttClient.publish(MQTT_PUB_TOPIC, payload.c_str());
        SerialMon.println("Published: " + payload);
    }
}

bool mqttConnect(void)
{
    SerialMon.println("Connecting to MQTT Broker...");
    String clientId = "ATOMDTU_" + String(random(0xffff), HEX);

    // ACCESS_TOKEN là username, password để trống
    if (mqttClient.connect(clientId.c_str(), ACCESS_TOKEN, ""))
    {
        SerialMon.println("MQTT CONNECTED!");
        mqttClient.subscribe(MQTT_SUB_TOPIC);
        mqttClient.publish(MQTT_PUB_TOPIC, "{\"status\":\"online\"}");
        return true;
    }
    else
    {
        SerialMon.print("MQTT FAILED. State: ");
        SerialMon.println(mqttClient.state());
        return false;
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    SerialMon.print("Message arrived [");
    SerialMon.print(topic);
    SerialMon.print("]: ");

    for (unsigned int i = 0; i < len; i++)
        SerialMon.print((char)payload[i]);

    SerialMon.println();
}

void InitNetwork(void)
{
    unsigned long start = millis();
    SerialMon.println("Initializing modem...");

    while (!modem.init())
    {
        SerialMon.println("Waiting modem init..." + String((millis() - start) / 1000) + "s");
        delay(1000);
    }

    SerialMon.println("Waiting for network...");
    while (!modem.waitForNetwork())
    {
        SerialMon.println("Waiting network..." + String((millis() - start) / 1000) + "s");
        delay(1000);
    }

    SerialMon.println("Connecting to GPRS...");
    while (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        SerialMon.println("Retrying GPRS...");
        delay(2000);
    }

    SerialMon.println("GPRS Connected!");
}
