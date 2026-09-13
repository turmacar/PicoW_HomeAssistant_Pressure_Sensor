#ifndef SRC_CREDENTIALS
#define SRC_CREDENTIALS

// Local credentials (gitignored) are included first so they override the
// defaults below. Any value not set there falls back to the default.
#if __has_include("Credentials.local.h")
#include "Credentials.local.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "your_wifi_ssid_here"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_wifi_password_here"
#endif
#ifndef MQTT_LOGIN
#define MQTT_LOGIN "your_mqtt_login_here"
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD "your_mqtt_password_here"
#endif
#ifndef MQTT_BROKER
#define MQTT_BROKER IPAddress(192,168,0,42)
#endif

#endif // SRC_CREDENTIALS
