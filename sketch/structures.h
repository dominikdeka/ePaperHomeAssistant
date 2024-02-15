#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <Arduino.h>

typedef struct {
  byte wakeupCount;
  byte currentPhase;
  byte viewMode;
  float voltage;
  String month;
  String weekDay;
  String date;  
  String lastUpdate;
} ApplicationState;

typedef struct {
    String start;
    std::vector<String> events;
} CalendarData;

typedef struct {
    const String tempName;
    const String humName;
    const String label;
    String tempValue;
    String humValue;
} MqttTopic;

typedef struct {
  int      Dt;
  String   Period;
  String   Icon;
  String   Trend;
  String   Main0;
  String   Forecast0;
  String   Forecast1;
  String   Forecast2;
  String   Description;
  String   Time;
  String   Country;
  float    lat;
  float    lon;
  float    Temperature;
  float    Feelslike;
  float    Humidity;
  float    High;
  float    Low;
  float    Winddir;
  float    Windspeed;
  float    Rainfall;
  float    Snowfall;
  float    Pop;
  float    Pressure;
  int      Cloudcover;
  int      Visibility;
  int      Sunrise;
  int      Sunset;
  int      Timezone;
} ForecastRecord;

#endif /* ifndef STRUCTURES_H_ */
