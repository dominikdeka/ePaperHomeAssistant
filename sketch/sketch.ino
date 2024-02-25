//TODOS - read/draw refactor, fix mqtt structure, refactoring...
#define ENABLE_GxEPD2_GFX 0
#define TIME_TO_SLEEP 1800  /* Time ESP32 will go to sleep (in seconds, max value 1800 = 30m) */
#define WAKEUP_SKIP 2 /* Skip every n wakups to save battery */ 
#define MAX_ATTEMPTS 4 /* Give up after MAX_ATTEMPTS try to connect any service */ 
#define MINIMAL_VOLTAGE 4.5 /* Below this voltage ESP goes deep sleep forever */ 
#define GPIO_BIT_MASK ((1ULL << GPIO_NUM_32) | (1ULL << GPIO_NUM_35))

#define STATUS_AREA_HEIGH 25 /* top partial window height (max: 480) */
#define CALENDAR_AREA_WIDTH 420 /* left bottom partial window width (max: 800) */


#include "credentials.h"
#include "settings.h"
#include "lang_pl.h"                  // Localisation (Polish)
#include "images.h"

#include <GxEPD2_BW.h> // v1.5.3
#include <U8g2_for_Adafruit_GFX.h> // v1.8.0

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection.h"

#include <WiFi.h> // Built-in
#include "time.h" // Built-in

#include <PubSubClient.h> // v2.8

#include <HTTPClient.h> // v2.2.0
#include <ArduinoJson.h> // v6.21.3

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// global variables
#include "structures.h"
#define max_readings 24
ForecastRecord WxConditions[1];
ForecastRecord WxForecast[max_readings];
std::vector<CalendarData> calEvents;

//--------------------------------------
// globals
//--------------------------------------

MqttTopic mqttTopics[6] = {
  {"dominikdeka@gmail.com/temperature/taras", "dominikdeka@gmail.com/hummidity/taras", "piwnica", "", ""},
  {"dominikdeka@gmail.com/temperature/loundry", "dominikdeka@gmail.com/hummidity/loundry", "pralnia", "", ""},
  {"dominikdeka@gmail.com/temperature/front", "dominikdeka@gmail.com/hummidity/front", "zewnetrzna", "", ""},
  {"dominikdeka@gmail.com/temperature/salon", "dominikdeka@gmail.com/hummidity/salon", "parter", "", ""},
  {"dominikdeka@gmail.com/temperature/justyna", "dominikdeka@gmail.com/hummidity/justyna", "pietro", "", ""},
  {"dominikdeka@gmail.com/temperature/bedroom", "dominikdeka@gmail.com/hummidity/bedroom", "poddasze", "", ""},
};

RTC_DATA_ATTR ApplicationState applicationState = {0, 0, 0, 0};

#include "wheather_common.h"
#include "drawing_common.h"
#include "drawing.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup()
{
  Serial.begin(115200);
  while(!Serial){delay(100);}

  setStateOnWakeup();
  delay(500);

  //setup mqtt
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(mqttCallback);

  if (connectWiFi() == true) {
    setApplicationPhase(0);
  } else {
    phases[6] = "WiFi connection failed";
    setApplicationPhase(6);
  }
}

void loop()
{
  if (applicationState.currentPhase == 0) { // initialize ePaper
    // *** special handling for Waveshare ESP32 Driver board *** //
    // ********************************************************* //
    hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    // *** end of special handling for Waveshare ESP32 Driver board *** //
    // **************************************************************** //

    display.init(false);
    u8g2Fonts.begin(display); // connect u8g2 procedures to Adafruit GFX
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    nextPhase();
  }
  if (applicationState.currentPhase == 1) { // reading time and battery voltage
    readDateTime();
    readVoltage();
    nextPhase();
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
    if (applicationState.viewMode == 0 || applicationState.viewMode == 1) {
      displayCalendarData();
    }
    if (applicationState.viewMode == 0 || applicationState.viewMode == 2) {
      displayWeather();
    }
    nextPhase();
    delay(2000);
  }
  if (applicationState.currentPhase == 6) { // going deep sleep phase
    deepSleep();
  }
}
void readVoltage() {
  int analogValue = analogRead(34);
  float analogVolts = (analogValue*3.3*2)/4096;
  applicationState.voltage = analogVolts;
}
void changeViewMode(bool up = true){
  int secondsFromBoot = millis() / 1000;
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
  Serial.print("change phase: ");
  Serial.println(applicationState.currentPhase);
}

boolean mqttDataCollected() {
  boolean collected = true;
  for(unsigned i = 0; i < sizeof(mqttTopics) / sizeof(MqttTopic); i++) {
    if ((mqttTopics[i].tempValue == "") || (mqttTopics[i].humValue == "")) {
      collected = false;
      break;
    }
  }
  return collected;
}

void deepSleep() {
  int uS_TO_S_FACTOR = 1000000;
  if (applicationState.voltage > MINIMAL_VOLTAGE) { // otherwise sleep forever
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  }
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);
  pinMode(GPIO_NUM_33, INPUT_PULLUP);

  esp_sleep_enable_ext1_wakeup(GPIO_BIT_MASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  
  Serial.println("Going to sleep now");
  // delay(1000);
  Serial.flush();
  display.hibernate();
  display.end(); // not sure if it is needed?
  esp_deep_sleep_start();
}

void mqttReconnect() {
  // Loop until we're reconnected
  byte topicsNumber = sizeof(mqttTopics) / sizeof(MqttTopic);
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
  char tmp[length+1];
  for (unsigned int i = 0; i < length; i++) {
    tmp[i] = (char)payload[i];
  }
  tmp[length] = 0;

  for(unsigned i = 0; i < sizeof(mqttTopics) / sizeof(MqttTopic); i++) {
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
boolean readDateTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov"); //(gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", Timezone, 1);  //setenv()adds the "TZ" variable to the environment with a value TimeZone, only used if set to 1, 0 means no change
  tzset(); // Set the TZ environment variable
  delay(100);
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo, 10000)) { // Wait for 10-sec for time to synchronise
    return false;
  }

  applicationState.month = month_M[timeinfo.tm_mon];
  applicationState.weekDay = weekday_D[timeinfo.tm_wday];
  applicationState.date = String(timeinfo.tm_mday);
  char time_output[30], update_time[30];
  strftime(update_time, sizeof(update_time), "%H:%M:%S", &timeinfo);  // Creates: '14:05:49'
  sprintf(time_output, "%s %s", TXT_UPDATED, update_time);
  applicationState.lastUpdate = time_output;
  return true;
}
void readCalendarEvents(void)
{
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec";
  http.begin(url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  Serial.println(url);
  delay(100);

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
            CalendarData date;
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

uint8_t connectWiFi() {
  IPAddress dns(8, 8, 8, 8); // Google DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  uint8_t connectionStatus;
  bool AttemptConnection = true;
  while (AttemptConnection) {
    connectionStatus = WiFi.status();
    if (millis() > start + 15000) { // Wait 15-secs maximum
      AttemptConnection = false;
    }
    if (connectionStatus == WL_CONNECTED || connectionStatus == WL_CONNECT_FAILED) {
      AttemptConnection = false;
    }
    delay(50);
  }
  if (connectionStatus == WL_CONNECTED) {
    Serial.println("WiFi connected at: " + WiFi.localIP().toString());
    return true;
  }
  else {
    Serial.println("WiFi connection *** FAILED ***");
    return false;
  } 
}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}