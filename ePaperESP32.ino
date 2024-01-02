//TODO -  start wifi, daty do struktury, poziom baterii, refactoring, README
//DONE - refresh co godzinę, rezygnacja po x - próbach, obsługa trybów, git
#define ENABLE_GxEPD2_GFX 0
#define TIME_TO_SLEEP 1800       /* Time ESP32 will go to sleep (in seconds, max value 1800) */
#define WAKEUP_SKIP 2 /* Skip every n wakups to save battery */ 
#define MAX_ATTEMPTS 4

#include "credentials.h"
#include "settings.h"
#include "wheather_lang_pl.h"                  // Localisation (Polish)

#include <GxEPD2_BW.h> // v1.5.3
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold24pt7b.h>

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection.h"

#include <WiFi.h>
#include "time.h"                     // Built-in

#include <PubSubClient.h> // v2.8

#include <HTTPClient.h> // v2.2.0
#include <ArduinoJson.h> // v6.21.3

#include "wheather_forecast_record.h"
#define max_readings 24
Forecast_record_type  WxConditions[1];
Forecast_record_type  WxForecast[max_readings];

#include "wheather_common.h"


typedef struct {
    String start;
    std::vector<String> events;
} calDate;
std::vector<calDate> calEvents;

String month = "";
String weekDay = "";
String date = "";  
String lastUpdate = "";

//--------------------------------------
// globals
//--------------------------------------
typedef struct {
    const String tempName;
    const String humName;
    const String label;
    String tempValue;
    String humValue;
} mqttTopic;

mqttTopic mqttTopics[6] = {
  {"dominikdeka@gmail.com/temperature/taras", "dominikdeka@gmail.com/hummidity/taras", "piwnica", "", ""},
  {"dominikdeka@gmail.com/temperature/loundry", "dominikdeka@gmail.com/hummidity/loundry", "pralnia", "", ""},
  {"dominikdeka@gmail.com/temperature/front", "dominikdeka@gmail.com/hummidity/front", "zewnetrzna", "", ""},
  {"dominikdeka@gmail.com/temperature/salon", "dominikdeka@gmail.com/hummidity/salon", "parter", "", ""},
  {"dominikdeka@gmail.com/temperature/justyna", "dominikdeka@gmail.com/hummidity/justyna", "pietro", "", ""},
  {"dominikdeka@gmail.com/temperature/bedroom", "dominikdeka@gmail.com/hummidity/bedroom", "poddasze", "", ""},
};

struct ApplicationState {
  byte wakeupCount;
  byte currentPhase;
  int currentPhaseStart;
  byte viewMode;
};
RTC_DATA_ATTR struct ApplicationState applicationState = {0, 0, 0, 0};

String modes[] = { "kompaktowy", "kalendarz", "pogoda"};
String phases[] = { "  ...", " laczenie wifi...", " pobieranie pomiarow...", " pobieranie kalendarza...",  " pobieranie prognozy...", " drukowanie...", "" };

#include "wheather_drawing.h"
#include "compact_drawing.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

#include <ezButton.h>
#define DEBOUNCE_TIME 100 // the debounce time in millisecond, increase this time if it still chatters

#define GPIO_BIT_MASK ((1ULL << GPIO_NUM_32) | (1ULL << GPIO_NUM_35))
ezButton button1(33); // create ezButton object that attach to pin GPIO33;
ezButton button2(32); // create ezButton object that attach to pin GPIO32;
ezButton button3(35); // create ezButton object that attach to pin GPIO35;

void setup()
{
  Serial.begin(115200);
  while(!Serial){delay(100);}

  setStateOnWakeup();
  delay(500);
  // *** special handling for Waveshare ESP32 Driver board *** //
  // ********************************************************* //
  hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  // *** end of special handling for Waveshare ESP32 Driver board *** //
  // **************************************************************** //

  display.init(115200);

  // setup buttons
  button1.setDebounceTime(DEBOUNCE_TIME);
  button2.setDebounceTime(DEBOUNCE_TIME);
  button3.setDebounceTime(DEBOUNCE_TIME);

  //setup mqtt
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(mqttCallback);

  // init application state
  setApplicationPhase(0);
}

void loop()
{
  if (applicationState.currentPhase == 0) { // drawing phase
    clearScreen();
    // setApplicationPhase(6);
    // displayCurrentState();

    nextPhase();
  }
  // if (applicationState.currentPhase == 1) { // listen to buttons to set mode
  //   button1.loop();
  //   if (button1.isPressed()) {
  //     button1.loop(); // reset button object state
  //     Serial.println(button1.getCount());
  //     changeViewMode();
  //     printCurrentState();
  //   }
  //   int secondsFromBoot = millis() / 1000;
  //   if (secondsFromBoot - applicationState.currentPhaseStart > 5) {
  //     nextPhase();
  //   }

  // }
  if (applicationState.currentPhase == 1) { // connecting wifi
    // We start by connecting to a WiFi network

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    nextPhase();
    readDateTime();
  }
  if (applicationState.currentPhase == 2) { // collecting mqtt data
    byte Attempts = 1;
    if (!mqttClient.connected()) {
      mqttReconnect();
      Attempts++;
    }
    mqttClient.loop();
    if(mqttDataCollected() || Attempts > MAX_ATTEMPTS){
      nextPhase();
    }
  }
  if (applicationState.currentPhase == 3) { // collecting GCal events
    byte Attempts = 1;
    while (calEvents.empty() && Attempts <= MAX_ATTEMPTS) {
      readCalendarEvents();
      Attempts++;
    };
    nextPhase();
  }
  if (applicationState.currentPhase == 4) { // collecting wheather data
    byte Attempts = 1;
    bool RxWeather = false, RxForecast = false;
    while ((RxWeather == false || RxForecast == false) && Attempts <= MAX_ATTEMPTS) { // Try up-to 2 time for Weather and Forecast data
      if (RxWeather  == false) RxWeather  = obtain_wx_data(wifiClient, "weather");
      if (RxForecast == false) RxForecast = obtain_wx_data(wifiClient, "forecast");
      Attempts++;
    }    
    StopWiFi();
    nextPhase();
  }
  if (applicationState.currentPhase == 5) { // draw data on screen
    int leftOffset = 0;    
    if (applicationState.viewMode == 0 || applicationState.viewMode == 1) {
      leftOffset = displayCalendarData();
    }
    if (applicationState.viewMode == 0 || applicationState.viewMode == 2) {
      int topOffset = displayWeather(leftOffset);
      // displayWeatherData(leftOffset, topOffset);
    }
    nextPhase();
    delay(2000);
  }
  if (applicationState.currentPhase == 6) { // going deep sleep phase
    deepSleep();
  }
}

void changeViewMode(bool up = true){
  int secondsFromBoot = millis() / 1000;
  applicationState.currentPhaseStart = secondsFromBoot;
  if (up) {
    applicationState.viewMode = (applicationState.viewMode + 1) % (sizeof(modes)/sizeof(String));
  } else {
    applicationState.viewMode = (applicationState.viewMode + (sizeof(modes)/sizeof(String)) - 1) % (sizeof(modes)/sizeof(String));
  }
  Serial.print("change mode: ");
  Serial.println(applicationState.viewMode);
}

void setStateOnWakeup() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :
      Serial.println("Wakeup caused by external signal using RESET");
      applicationState.wakeupCount = 0;
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : 
      if (digitalRead(GPIO_NUM_32) == HIGH) {
        changeViewMode();
        Serial.println("GPIO_NUM_32 was triggered");
      }
      if (digitalRead(GPIO_NUM_35) == HIGH) {
        changeViewMode(false);
        Serial.println("GPIO_NUM_35 was triggered");
      }
      applicationState.wakeupCount = 0;
      break;
    case ESP_SLEEP_WAKEUP_TIMER :
      Serial.println("Wakeup caused by timer");
      applicationState.wakeupCount = applicationState.wakeupCount + 1;
      if ((applicationState.wakeupCount + 1) % WAKEUP_SKIP == 0) {
        deepSleep();  
      }
      break;
    default :
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      applicationState.wakeupCount = 0;
      break;
  }
}

void nextPhase() {
  setApplicationPhase(applicationState.currentPhase + 1);
  displayCurrentState();
}

void setApplicationPhase(byte phase) {
  int secondsFromBoot = millis() / 1000;
  applicationState.currentPhase = phase;
  applicationState.currentPhaseStart = secondsFromBoot;
  Serial.print("change phase: ");
  Serial.println(applicationState.currentPhase);
}

boolean mqttDataCollected() {
  boolean collected = true;
  for(unsigned i = 0; i < sizeof(mqttTopics) / sizeof(mqttTopic); i++) {
    if ((mqttTopics[i].tempValue == "") || (mqttTopics[i].humValue == "")) {
      collected = false;
      break;
    }
  }
  // if (message.equals("")) {
  //   collected = false;
  // }
  return collected;
  // if (collected) {
  //   Serial.println("Collected");
  //   // Serial.println(message);
  //   displayMqttData();
  // }
}
void deepSleep() {
  int uS_TO_S_FACTOR = 1000000;
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);
  esp_sleep_enable_ext1_wakeup(GPIO_BIT_MASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  
  Serial.println("Going to sleep now");
  // delay(1000);
  Serial.flush();
  display.hibernate();
  display.end(); // not sure if it is needed?
  esp_deep_sleep_start();
}

void clearScreen()
{
  String messsage = phases[applicationState.currentPhase];
  display.setRotation(0);
  display.setFont(&FreeSerifBold12pt7b);
  display.setTextColor(GxEPD_BLACK);

  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(messsage, 0, 0, &tbx, &tby, &tbw, &tbh);

  uint16_t x = 0 - tbx;
  uint16_t y = 0 - tby;

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(messsage);
  }
  while (display.nextPage());
}

void mqttReconnect() {
  // Loop until we're reconnected
  byte topicsNumber = sizeof(mqttTopics) / sizeof(mqttTopic);
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqttClientId, mqttUser, mqttPassword)) {      
      Serial.println("connected");
      for(byte i = 0; i < topicsNumber; i++) {
        mqttClient.subscribe(mqttTopics[i].tempName.c_str());
        mqttClient.subscribe(mqttTopics[i].humName.c_str());
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//--------------------------------------
// function callback called everytime 
// if a mqtt message arrives from the broker
//--------------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: '");
  Serial.println(topic);
  char tmp[length+1];
  for (unsigned int i = 0; i < length; i++) {
    tmp[i] = (char)payload[i];
  }
  tmp[length] = 0;

  for(unsigned i = 0; i < sizeof(mqttTopics) / sizeof(mqttTopic); i++) {
    if (mqttTopics[i].tempName == topic) {
      mqttTopics[i].tempValue = tmp;
    }
    if (mqttTopics[i].humName == topic) {
      mqttTopics[i].humValue = tmp;
    }
  }
  // String myCurrentTime = timeClient.getFormattedTime();
  // mqttClient.publish(mqttTopicOut,("ESP8266: Cedalo Mosquitto is awesome. ESP8266-Time: " + myCurrentTime).c_str());
}
void readDateTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov"); //(gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", Timezone, 1);  //setenv()adds the "TZ" variable to the environment with a value TimeZone, only used if set to 1, 0 means no change
  tzset(); // Set the TZ environment variable
  delay(100);
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo, 10000)) { // Wait for 10-sec for time to synchronise
    Serial.println("Failed to obtain time");
    // return false;
  }

  month = month_M[timeinfo.tm_mon];
  weekDay = weekday_D[timeinfo.tm_wday];
  date = String(timeinfo.tm_mday);
  char time_output[30], update_time[30];
  strftime(update_time, sizeof(update_time), "%H:%M:%S", &timeinfo);  // Creates: '14:05:49'
  sprintf(time_output, "%s %s", TXT_UPDATED, update_time);
  lastUpdate = String(time_output);
  phases[6] = lastUpdate;  
  Serial.println("Week: " + weekDay);  
  Serial.println("Month: " + month);  
  Serial.println("Date: " + date);  
  Serial.println("Last update: " + lastUpdate);  
}
void readCalendarEvents(void)
{
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec";
    http.begin(url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    Serial.println(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print("DeserializationError: ");
          Serial.println(error.c_str());
        } else {
          if (doc.is<JsonArray>() && doc.size() > 0) {
            byte size = doc.size();
            for (byte i = 0; i < size; i++) {
              String start = doc[i]["start"];
              calDate date;
              date.start = start;
              byte eventsSize = doc[i]["events"].size();
              // std::vector<String> events;
              for (byte j = 0; j < eventsSize; j++) {
                  date.events.push_back(doc[i]["events"][j]); // (*it)[0] == vec[i][0]
              }

              calEvents.push_back(date);
              // [i].start = start;
              // calEvents[i].events = events;
            }
          } else {
            Serial.println("Error: Array expected");
          }
        }
      } else {
        Serial.print("HTTP response error ");
        Serial.println(httpCode);
      }
    } else {
      Serial.print("HTTP request error ");
      Serial.println(httpCode);
    }

    http.end();
  }

}

//TODO #########################################################################################
// uint8_t StartWiFi() {
//   Serial.print("\r\nConnecting to: "); Serial.println(String(ssid));
//   IPAddress dns(8, 8, 8, 8); // Google DNS
//   WiFi.disconnect();
//   WiFi.mode(WIFI_STA); // switch off AP
//   WiFi.setAutoConnect(true);
//   WiFi.setAutoReconnect(true);
//   WiFi.begin(ssid, password);
//   unsigned long start = millis();
//   uint8_t connectionStatus;
//   bool AttemptConnection = true;
//   while (AttemptConnection) {
//     connectionStatus = WiFi.status();
//     if (millis() > start + 15000) { // Wait 15-secs maximum
//       AttemptConnection = false;
//     }
//     if (connectionStatus == WL_CONNECTED || connectionStatus == WL_CONNECT_FAILED) {
//       AttemptConnection = false;
//     }
//     delay(50);
//   }
//   if (connectionStatus == WL_CONNECTED) {
//     wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
//     Serial.println("WiFi connected at: " + WiFi.localIP().toString());
//   }
//   else Serial.println("WiFi connection *** FAILED ***");
//   return connectionStatus;
// }
// //#########################################################################################
void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}