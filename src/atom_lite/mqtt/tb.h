#ifndef INIT_THINGSBOARD
#define INIT_THINGSBOARD

#include "global.h"

#include "Arduino_MQTT_Client.h"
#include "ThingsBoard.h"
#include "TinyGsmClient.h"

// Thông tin kết nối MQTT
#define MQTT_BROKER "app.coreiot.io"
#define MQTT_PORT 1883
#define ACCESS_TOKEN "qu3oTjv6NaFKeTiFDlst"
#define MQTT_PUB_TOPIC "v1/devices/me/telemetry"
#define MQTT_SUB_TOPIC "v1/devices/me/attributes"

#define UPLOAD_INTERVAL 10000

void initSetUpThingBoard();

#endif