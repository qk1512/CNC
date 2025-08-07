/* #include "mqtt_client.h"
//#define TINY_GSM_MODEM_SIM7600

TinyGsm modem(SerialAT);
TinyGsmClient tcpClient(modem);
PubSubClient mqttClient(tcpClient);

const char apn[] = "v-internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    Serial.println("Message arrived [" + String(topic) + "]: ");
    Serial.write(payload, len);
    Serial.println();
}

void mqttInit()
{
    SerialAT.begin(SIM7680_BAUDRATE, SERIAL_8N1, ATOM_DTU_SIM7680_RX, ATOM_DTU_SIM7680_TX);
    modem.init();

    InitNetwork();

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

bool mqttConnect()
{
    Serial.println("Connecting to MQTT...");
    String clientId = "MQTTID_" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), ACCESS_TOKEN, ""))
    {
        Serial.println("MQTT connected");
        //mqttClient.publish(MQTT_PUB_TOPIC, "CATM MQTT CLIENT ONLINE");
        return true;
    }
    else
    {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(mqttClient.state());
        return false;
    }
}

void mqttLoop()
{
    mqttClient.loop();
}

bool mqttPublish(const char *topic, const char *payload)
{
    return mqttClient.publish(topic, payload);
}

bool mqttIsConnected()
{
    return mqttClient.connected();
}

void InitNetwork(void)
{
    unsigned long start = millis();
    SerialMon.println("Initializing modem...");
    while (!modem.init())
    {
        SerialMon.println("waiting...." + String((millis() - start) / 1000) +
                          "s");
    };

    start = millis();
    SerialMon.println("Waiting for network...");
    while (!modem.waitForNetwork())
    {
        SerialMon.println("waiting...." + String((millis() - start) / 1000) +
                          "s");
    }

    SerialMon.println("Waiting for GPRS connect...");
    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        SerialMon.println("waiting...." + String((millis() - start) / 1000) +
                          "s");
    }
    SerialMon.println("success");
} */