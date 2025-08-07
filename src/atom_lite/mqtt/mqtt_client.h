/* #ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H


//#include <Arduino.h>
#include <global.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <TinyGsmClient.h>


// Thông tin kết nối MQTT
#define MQTT_BROKER "app.coreiot.io"
#define MQTT_PORT 1883
#define ACCESS_TOKEN "qu3oTjv6NaFKeTiFDlst"
#define MQTT_PUB_TOPIC "v1/devices/me/telemetry"
#define MQTT_SUB_TOPIC "v1/devices/me/attributes"

#define UPLOAD_INTERVAL 10000

extern PubSubClient mqttClient;
extern TinyGsm modem;
extern TinyGsmClient tcpClient;

// Hàm khởi tạo kết nối mạng và MQTT
void mqttInit();
bool mqttConnect();
void mqttLoop();
bool mqttPublish(const char *topic, const char *payload);
bool mqttIsConnected();
void InitNetwork(void);

#endif
 */