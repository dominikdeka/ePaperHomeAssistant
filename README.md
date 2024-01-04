# ePaperDisplay

* create `credentials.h` file that contains:

```
const char * ssid = "************";   // Wifi Username
const char * password = "************";  // Wifi Password

String GOOGLE_SCRIPT_ID = "************";

const char* mqtt_server = "maqiatto.com"; // at least in my case
const uint16_t mqtt_server_port = 1883;
const char* mqttUser = "************";
const char* mqttPassword = "************";
const char* mqttClientId = "ePaperDisplay-1";

// Use your own API key by signing up for a free developer account at https://openweathermap.org/
String apikey       = "************";                      // See: https://openweathermap.org/  // It's free to get an API key, but don't take more than 60 readings/minute!
const char server[] = "api.openweathermap.org";
