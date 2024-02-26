float mmToInches(float value_mm)
{
  return 0.0393701 * value_mm;
}

float hPaToInHg(float value_hPa)
{
  return 0.02953 * value_hPa;
}

//#########################################################################################
void convertReadingsToImperial() {
  WxConditions[0].Pressure = hPaToInHg(WxConditions[0].Pressure);
  WxForecast[1].Rainfall   = mmToInches(WxForecast[1].Rainfall);
  WxForecast[1].Snowfall   = mmToInches(WxForecast[1].Snowfall);
}

//#########################################################################################
// Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
bool decodeWeather(WiFiClient& json, String Type) {
  Serial.print(F("\nCreating object...and "));
  // allocate the JsonDocument
  DynamicJsonDocument doc(35 * 1024);
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  // convert it to a JsonObject
  JsonObject root = doc.as<JsonObject>();
  Serial.println(" Decoding " + Type + " data");
  if (Type == "weather") {
    // All Serial.println statements are for diagnostic purposes and not required, remove if not needed
    WxConditions[0].lon         = root["coord"]["lon"].as<float>();
    WxConditions[0].lat         = root["coord"]["lat"].as<float>();
    WxConditions[0].Main0       = root["weather"][0]["main"].as<const char*>();
    WxConditions[0].Forecast0   = root["weather"][0]["description"].as<const char*>();
    WxConditions[0].Forecast1   = root["weather"][1]["description"].as<const char*>();
    WxConditions[0].Forecast2   = root["weather"][2]["description"].as<const char*>();
    WxConditions[0].Icon        = root["weather"][0]["icon"].as<const char*>();
    WxConditions[0].Temperature = root["main"]["temp"].as<float>();
    WxConditions[0].Pressure    = root["main"]["pressure"].as<float>();
    WxConditions[0].Humidity    = root["main"]["humidity"].as<float>();
    WxConditions[0].Low         = root["main"]["temp_min"].as<float>();
    WxConditions[0].High        = root["main"]["temp_max"].as<float>();
    WxConditions[0].Windspeed   = root["wind"]["speed"].as<float>();
    WxConditions[0].Winddir     = root["wind"]["deg"].as<float>();
    WxConditions[0].Cloudcover  = root["clouds"]["all"].as<int>();
    WxConditions[0].Visibility  = root["visibility"].as<int>();
    WxConditions[0].Rainfall    = root["rain"]["1h"].as<float>();
    WxConditions[0].Snowfall    = root["snow"]["1h"].as<float>();
    WxConditions[0].Country     = root["sys"]["country"].as<const char*>();
    WxConditions[0].Sunrise     = root["sys"]["sunrise"].as<int>();
    WxConditions[0].Sunset      = root["sys"]["sunset"].as<int>();
    WxConditions[0].Timezone    = root["timezone"].as<int>();
  }
  if (Type == "forecast") {
    JsonArray list                    = root["list"];
    for (byte r = 0; r < max_readings; r++) {
      WxForecast[r].Dt                = list[r]["dt"].as<int>();
      WxForecast[r].Temperature       = list[r]["main"]["temp"].as<float>();
      WxForecast[r].Low               = list[r]["main"]["temp_min"].as<float>();
      WxForecast[r].High              = list[r]["main"]["temp_max"].as<float>();
      WxForecast[r].Pressure          = list[r]["main"]["pressure"].as<float>();
      WxForecast[r].Humidity          = list[r]["main"]["humidity"].as<float>();
      WxForecast[r].Forecast0         = list[r]["weather"][0]["main"].as<const char*>();
      WxForecast[r].Forecast1         = list[r]["weather"][1]["main"].as<const char*>();
      WxForecast[r].Forecast2         = list[r]["weather"][2]["main"].as<const char*>();
      WxForecast[r].Icon              = list[r]["weather"][0]["icon"].as<const char*>();
      WxForecast[r].Description       = list[r]["weather"][0]["description"].as<const char*>();
      WxForecast[r].Cloudcover        = list[r]["clouds"]["all"].as<int>();
      WxForecast[r].Windspeed         = list[r]["wind"]["speed"].as<float>();
      WxForecast[r].Winddir           = list[r]["wind"]["deg"].as<float>();
      WxForecast[r].Rainfall          = list[r]["rain"]["3h"].as<float>();
      WxForecast[r].Snowfall          = list[r]["snow"]["3h"].as<float>();
      WxForecast[r].Pop               = list[r]["pop"].as<float>();
      WxForecast[r].Period            = list[r]["dt_txt"].as<const char*>();
    }
    //------------------------------------------
    float pressure_trend = WxForecast[2].Pressure - WxForecast[0].Pressure; // Measure pressure slope between ~now and later
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0; // Remove any small variations less than 0.1
    WxConditions[0].Trend = "0";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";

    if (Units == "I") convertReadingsToImperial();
  }
  return true;
}
//#########################################################################################
String convertUnixTime(int unix_time) {
  // Returns either '21:12  ' or ' 09:12pm' depending on Units mode
  time_t tm = unix_time;
  struct tm *now_tm = gmtime(&tm);
  char output[40];
  if (Units == "M") {
    strftime(output, sizeof(output), "%H:%M %d/%m/%y", now_tm);
  }
  else {
    strftime(output, sizeof(output), "%I:%M%P %m/%d/%y", now_tm);
  }
  return output;
}
//#########################################################################################
