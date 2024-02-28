#include "displayCommonFunctions.h"
#include "displayWheatherFunctions.h"
#include "images.h"

void displayCurrentState();
void displayWheather();
void displayCalendarData(int viewMode);

void displayMqttReadings(int leftOffset, int topOffset, int width, int height);
void displayForecastSection(int x, int y, int w);
void displayConditionsSection(int x, int y, int width, String IconName);
void displayToday(int leftOffset, int topOffset, int width, int height);

void displayCurrentState() {
  byte modesNo = sizeof(modes) / sizeof(String);

  String statusMsg = applicationState.currentPhase < 6
    ? phases[applicationState.currentPhase]
    : applicationState.lastUpdate;

  String batteryMsg = applicationState.voltage > MINIMAL_VOLTAGE  
    ? TXT_VOLTAGE + String(applicationState.voltage) + 'V'
    : TXT_CHARGE_BATTERY;

  display.setPartialWindow(0, 0, display.width(), STATUS_SECTION_HEIGHT);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_helvB14_te);
    uint16_t accumulatedWidth = 0;
    for(int i = modesNo - 1; i >= 0; i--) {    
      uint16_t w = u8g2Fonts.getUTF8Width(modes[i].c_str());
      accumulatedWidth += w + 10;
      int x = display.width() - accumulatedWidth;
      if (i == applicationState.viewMode) {
        u8g2Fonts.setForegroundColor(GxEPD_WHITE);
        u8g2Fonts.setBackgroundColor(GxEPD_BLACK);
        display.fillRect(x, 0, w + 10, STATUS_SECTION_HEIGHT, GxEPD_BLACK);
      }

      drawString(x + 5, 20, modes[i], "left", u8g2_font_helvB14_te);

      u8g2Fonts.setForegroundColor(GxEPD_BLACK);
      u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    }
    uint16_t phaseW = drawString(5, 20, statusMsg, "left", u8g2_font_helvB14_te) + 5;
    drawString((display.width() - accumulatedWidth + phaseW)/2, 20, batteryMsg, "center", u8g2_font_helvB14_te);
  }
  while (display.nextPage());

}

void displayWheather() {
  const int leftOffset = LEFT_SECTION_WIDTH;
  const int topOffset = STATUS_SECTION_HEIGHT + 10;
  const int imageHeight = 140;
  display.setPartialWindow(leftOffset, topOffset, display.width() - leftOffset, display.height() - topOffset);
  display.firstPage();
  u8g2Fonts.setFont(u8g2_font_helvB12_tf);
  do
  {
    displayToday(leftOffset + 20, topOffset, 160, imageHeight);
    displayMqttReadings(display.width() - 200, topOffset, 200, imageHeight);

    displayConditionsSection(leftOffset, topOffset + imageHeight, display.width() - leftOffset, WxConditions[0].Icon);
    displayForecastSection(leftOffset, topOffset + imageHeight + 120, display.width() - leftOffset);
  }
  while (display.nextPage());
}


void displayCalendarData(int viewMode) {
  const int topOffset = STATUS_SECTION_HEIGHT + 10;
  const int tableWidth = LEFT_SECTION_WIDTH;
  const int maxTableHeight = display.height() - topOffset;

  const int padding = 5;

  byte daysNumber = calEvents.size();


  uint16_t date_max_width;
  u8g2Fonts.setFont(u8g2_font_helvB14_te);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  for(byte i = 0; i < daysNumber; i++) {
    int16_t tw = u8g2Fonts.getUTF8Width(calEvents[i].start.c_str()); // text box width


    if (tw > date_max_width) {
      date_max_width = tw;
    }
  }

  display.setPartialWindow(0, topOffset, viewMode == 0 ? LEFT_SECTION_WIDTH : display.width(), display.height());
  display.firstPage();
  byte section = 0;
  do
  {
    int currentHeight = topOffset;
    for(byte i = 0; i < daysNumber; i++) {
      currentHeight = drawEventsDay(section, currentHeight, maxTableHeight, calEvents[i].start, date_max_width, calEvents[i].events, LEFT_SECTION_WIDTH - date_max_width);
      if (currentHeight == 0) {
        if (viewMode == 0 || viewMode == 1 && section == 1){
          break;
        } else {
          section++;
          currentHeight = topOffset;
        }
      }
    }
  }
  while (display.nextPage());
}

void displayMqttReadings(int leftOffset, int topOffset, int width, int height) {
  display.drawBitmap(display.width() - 200, topOffset, home, width, height, GxEPD_BLACK);
  drawString(leftOffset + 61, topOffset + 28, mqttTopics[5].value + "°"); //dominikdeka@gmail.com/temperature/bedroom
  drawString(leftOffset + 61, topOffset + 63, mqttTopics[4].value + "°"); //dominikdeka@gmail.com/temperature/justyna
  drawString(leftOffset + 61, topOffset + 98, mqttTopics[3].value + "°"); //dominikdeka@gmail.com/temperature/salon
  drawString(leftOffset + 61, topOffset + 133, mqttTopics[1].value + "°"); //dominikdeka@gmail.com/temperature/loundry
  drawString(leftOffset + 130, topOffset + 133, mqttTopics[0].value + "°"); //dominikdeka@gmail.com/temperature/taras
  drawString(leftOffset + 130, topOffset + 63, mqttTopics[2].value + "°"); //dominikdeka@gmail.com/temperature/front
}

void displayForecastSection(int x, int y, int width) {
  float temperature_readings[max_readings] = {0};
  float rain_readings[max_readings]        = {0};
  float snow_readings[max_readings]        = {0};

  int f = 0;
  do {
    displayForecastWeather(x, y, f, width);
    f++;
  } while (f <= 4);
  // Pre-load temporary arrays with with data - because C parses by reference
  int r = 0;
  do {
    if (Units == "I") rain_readings[r]     = WxForecast[r].Rainfall * 0.0393701; else rain_readings[r]     = WxForecast[r].Rainfall;
    if (Units == "I") snow_readings[r]     = WxForecast[r].Snowfall * 0.0393701; else snow_readings[r]     = WxForecast[r].Snowfall;
    temperature_readings[r] = WxForecast[r].Temperature;
    r++;
  } while (r < max_readings);
  int gwidth = 150, gheight = 72;
  int gx = (width - gwidth * 2) / 3 + 5;
  int gy = y + 100;
  int gap = gwidth + gx;
  drawGraph(x + gx + 0 * gap, gy, gwidth, gheight, 10, 30,    Units == "M" ? TXT_TEMPERATURE_C : TXT_TEMPERATURE_F, temperature_readings, max_readings, true, false);
  const int Rain_array_size = sizeof(rain_readings) / sizeof(float);
  const int Snow_array_size = sizeof(snow_readings) / sizeof(float);
  if (SumOfPrecip(rain_readings, Rain_array_size) >= SumOfPrecip(snow_readings, Snow_array_size))
    drawGraph(x + gx + 1 * gap + 5, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_RAINFALL_MM : TXT_RAINFALL_IN, rain_readings, Rain_array_size, true, true);
  else drawGraph(x + gx + 1 * gap + 5, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_SNOWFALL_MM : TXT_SNOWFALL_IN, snow_readings, Snow_array_size, true, true);
}
//#########################################################################################
void displayConditionsSection(int x, int y, int width, String IconName) {
  // wheather large icon
  int iconCentreX = x + 86; int iconCentreY =  y + 70;
  displayWheatherIcon(iconCentreX, iconCentreY, IconName, LargeIcon);

  // cloud cover small icon
  iconCentreX = iconCentreX + 90;
  iconCentreY = iconCentreY - 45;
  addcloud(iconCentreX - 9, iconCentreY - 3, Small * 0.5, 2); // Cloud top left
  addcloud(iconCentreX + 3, iconCentreY - 3, Small * 0.5, 2); // Cloud top right
  addcloud(iconCentreX, iconCentreY, Small * 0.5, 2); // Main cloud

  String cloudCover = String(WxConditions[0].Cloudcover) + "%";
  String humidity = TXT_HUMIDITY + ": " + String(WxConditions[0].Humidity, 0) + "%";
  uint16_t tbh = 22;
  drawString(iconCentreX + 20 + 5, iconCentreY, cloudCover + "  " + humidity, "left");

  String temperature = TXT_TEMPERATURES + ": " + String(WxConditions[0].Temperature, 1) + "°C (" + String(WxConditions[0].High, 0) + "°|" + String(WxConditions[0].Low, 0) + "°)";
  drawString(iconCentreX - 20, iconCentreY + tbh, temperature, "left");

  String slope_direction = TXT_PRESSURE_STEADY;
  if (WxConditions[0].Trend == "+") slope_direction = TXT_PRESSURE_RISING;
  if (WxConditions[0].Trend == "-") slope_direction = TXT_PRESSURE_FALLING;

  String preasure = TXT_PRESSURE + ": " + String(WxConditions[0].Pressure, 0) + "hPa (" + slope_direction + ")";
  drawString(iconCentreX - 20, iconCentreY + 2 * tbh, preasure, "left");

  String sunrise = TXT_SUNRISE + ": " + convertUnixTime(WxConditions[0].Sunrise + WxConditions[0].Timezone).substring(0, 5) + " " + TXT_SUNSET + ": " + convertUnixTime(WxConditions[0].Sunset + WxConditions[0].Timezone).substring(0, 5);
  drawString(iconCentreX - 20, iconCentreY + 3 * tbh, sunrise, "left");
}

void displayToday(int leftOffset, int topOffset, int width, int height) {
  display.drawRect(leftOffset, topOffset, width, height, GxEPD_BLACK);

  drawString(leftOffset + width / 2, topOffset + 17, applicationState.month, "center");
  drawString(leftOffset + width / 2, topOffset + height - 5, applicationState.weekDay, "center");
  drawString(leftOffset + width / 2, topOffset + height / 2 + 21, applicationState.date, "center", u8g2_font_fub42_tf);
}
