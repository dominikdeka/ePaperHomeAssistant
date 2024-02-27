#include "collectWheatherDataFunctions.h"

void readVoltage() {
  int analogValue = analogRead(34);
  float analogVolts = (analogValue*3.3*2)/4096;
  applicationState.voltage = analogVolts;
}

boolean mqttDataCollected() {
  boolean collected = true;
  byte topicsNumber = mqttTopics.size();
  for(unsigned i = 0; i < topicsNumber; i++) {
    if (mqttTopics[i].value == "") {
      collected = false;
      break;
    }
  }
  return collected;
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
  byte topicsNumber = mqttTopics.size();
  for(unsigned i = 0; i < topicsNumber; i++) {
    if (mqttTopics[i].topic == topic) {
      mqttTopics[i].value = tmp;
    }
  }
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

bool readWheatherData(WiFiClient& client, const String& RequestType) {
  const String units = (Units == "M" ? "metric" : "imperial");
  client.stop(); // close connection before sending a new request
  HTTPClient http;
  String uri = "/data/2.5/" + RequestType + "?q=" + City + "," + Country + "&APPID=" + apikey + "&mode=json&units=" + units + "&lang=" + Language;
  if(RequestType != "weather")
  {
    uri += "&cnt=" + String(max_readings);
  }
  //http.begin(uri,test_root_ca); //HTTPS example connection
  http.begin(client, server, 80, uri);
  int httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    if (!decodeWeather(http.getStream(), RequestType)) return false;
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}