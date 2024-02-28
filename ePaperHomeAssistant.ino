//TODOS - lastUpdate - date only, refactore settings, fonts in i18n...
#define ENABLE_GxEPD2_GFX 0
#define TIME_TO_SLEEP 1800  /* Time ESP32 will go to sleep (in seconds, max value 1800 = 30m) */
#define WAKEUP_SKIP 2 /* Skip every n wakups to save battery */ 
#define MAX_ATTEMPTS 4 /* Give up after MAX_ATTEMPTS try to connect any service */ 
#define MINIMAL_VOLTAGE 4.5 /* Below this voltage ESP goes deep sleep forever */ 
#define GPIO_BIT_MASK ((1ULL << GPIO_NUM_32) | (1ULL << GPIO_NUM_35))

#define STATUS_SECTION_HEIGHT 25 /* top (status) section height (max: 480) */
#define LEFT_SECTION_WIDTH 400 /* left bottom (calendar) section width (max: 800) */

#include "src/credentials.h"
#include "src/settings.h"
#include "src/i18n/pl.h"
#include "src/structures.h"

#include <GxEPD2_BW.h> // v1.5.3
#include <U8g2_for_Adafruit_GFX.h> // v1.8.0

// select the display class and display driver class
#include "src/setupHardware.h"

#include <WiFi.h> // Built-in
#include "time.h" // Built-in

#include <PubSubClient.h> // v2.8

#include <HTTPClient.h> // v2.2.0
#include <ArduinoJson.h> // v6.21.3

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// global variables
#define max_readings 24
ForecastRecord WxConditions[1];
ForecastRecord WxForecast[max_readings];

std::vector<CalendarData> calEvents;
std::vector<MqttData> mqttTopics;

RTC_DATA_ATTR ApplicationState applicationState = {0, 0, 0};

#include "src/collectData.h"
#include "src/displayMainSections.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(100); }

  setStateOnWakeup();
  delay(500);

  //setup mqtt
  mqttInit();

  if (connectWiFi() == true) {
    setApplicationPhase(0);
  } else {
    setApplicationPhase(6); // leave display unchanged and go sleep
  }
}

void loop() {
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
  if (applicationState.currentPhase == 2) { // collecting GCal events
    byte Attempts = 1;
    while (calEvents.empty() && Attempts <= MAX_ATTEMPTS) {
      readCalendarEvents();
      Attempts++;
    };
    nextPhase();
  }
  if (applicationState.currentPhase == 3) { // collecting mqtt data
    byte Attempts = 1;
    if (!mqttClient.connected()) {
      mqttReconnect();
      Attempts++;
    }
    mqttClient.loop();
    if(mqttDataCollected() || Attempts > MAX_ATTEMPTS) {
      nextPhase();
    }
  }
  if (applicationState.currentPhase == 4) { // collecting wheather data
    byte Attempts = 1;
    bool RxWeather = false, RxForecast = false;
    while ((RxWeather == false || RxForecast == false) && Attempts <= MAX_ATTEMPTS) { // Try up-to 2 time for Weather and Forecast data
      if (RxWeather  == false) RxWeather  = readWheatherData(wifiClient, "weather");
      if (RxForecast == false) RxForecast = readWheatherData(wifiClient, "forecast");
      Attempts++;
    }    
    StopWiFi();
    nextPhase();
  }
  if (applicationState.currentPhase == 5) { // draw data on screen
    if (applicationState.viewMode == 0 || applicationState.viewMode == 1) {
      displayCalendarData(applicationState.viewMode);
    }
    if (applicationState.viewMode == 0 || applicationState.viewMode == 2) {
      displayWheather();
    }
    nextPhase();
    delay(2000);
  }
  if (applicationState.currentPhase == 6) { // going deep sleep phase
    deepSleep();
  }
}
void changeViewMode(bool up = true) {
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
  byte nextPhase;
  if (applicationState.viewMode == 1 && applicationState.currentPhase == 2) { // skip reading wheather
    nextPhase = 5;
  } else if (applicationState.viewMode == 2 && applicationState.currentPhase == 1) { // skip reading calendar
    nextPhase = 3;
  } else {
    nextPhase = applicationState.currentPhase + 1;
  }
  setApplicationPhase(nextPhase);
  displayCurrentState();
}

void setApplicationPhase(byte phase) {
  int secondsFromBoot = millis() / 1000;
  applicationState.currentPhase = phase;
  Serial.print("change phase: ");
  Serial.println(applicationState.currentPhase);
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

void mqttInit() {
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(mqttCallback);
  for(byte i = 0; i < sizeof(mqttTopicNames) / sizeof(String); i++) {
    MqttData topic;
    topic.topic = mqttTopicNames[i];
    topic.value = "";
    mqttTopics.push_back(topic);
  }
}

void mqttReconnect() {
  byte topicsNumber = sizeof(mqttTopicNames) / sizeof(String);
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(mqttClientId, mqttUser, mqttPassword)) {      
      Serial.println("connected");
      for(byte i = 0; i < topicsNumber; i++) {
        mqttClient.subscribe(mqttTopicNames[i].c_str());
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
