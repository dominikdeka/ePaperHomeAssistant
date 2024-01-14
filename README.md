# ePaperDisplay

* create `credentials.h` file that contains:

```
const char * ssid = "************";   // Wifi Username
const char * password = "************";  // Wifi Password

// Google calendar data
String GOOGLE_SCRIPT_ID = "************";

// Sensors data
const char* mqtt_server = "maqiatto.com"; // at least in my case
const uint16_t mqtt_server_port = 1883;
const char* mqttUser = "************";
const char* mqttPassword = "************";
const char* mqttClientId = "ePaperDisplay-1";

// Use your own API key by signing up for a free developer account at https://openweathermap.org/
String apikey       = "************";
const char server[] = "api.openweathermap.org";
```
