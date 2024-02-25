#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  false

float temperature_readings[max_readings] = {0};
float rain_readings[max_readings]        = {0};
float snow_readings[max_readings]        = {0};

void displayCurrentState();
void drawGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, float DataArray[], int readings, boolean auto_scale, boolean barchart_mode);
void displayForecastWeather(int x, int y, int index, int width);
void displayForecastSection(int x, int y);
void displayConditionsSection(int x, int y, int width, String IconName);
void displayToday(int leftOffset, int topOffset, int width, int height);
void displayWeather(int leftOffset);
void displayCalendarData();

void displayCurrentState() {
  byte modesNo = sizeof(modes) / sizeof(String);

  String phaseMsg = applicationState.currentPhase < 6
    ? phases[applicationState.currentPhase]
    : applicationState.lastUpdate;
  String batteryMsg = applicationState.voltage > MINIMAL_VOLTAGE  
    ? TXT_VOLTAGE + String(applicationState.voltage) + 'V'
    : TXT_CHARGE_BATTERY;

  display.setPartialWindow(0, 0, display.width(), STATUS_AREA_HEIGH);
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
        display.fillRect(x, 0, w + 10, STATUS_AREA_HEIGH, GxEPD_BLACK);
      }

      drawString(x + 5, 20, modes[i], LEFT, u8g2_font_helvB14_te);

      u8g2Fonts.setForegroundColor(GxEPD_BLACK);
      u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    }
    uint16_t phaseW = drawString(0, 20, phaseMsg, LEFT, u8g2_font_helvB14_te);
    drawString((display.width() - accumulatedWidth + phaseW)/2, 20, batteryMsg, CENTER, u8g2_font_helvB14_te);
  }
  while (display.nextPage());

}

void drawGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, float DataArray[], int readings, boolean auto_scale, boolean barchart_mode) {
#define auto_scale_margin 0 // Sets the autoscale increment, so axis steps up in units of e.g. 3
#define y_minor_axis 5      // 5 y-axis division markers
  float maxYscale = -10000;
  float minYscale =  10000;
  int last_x, last_y;
  float x2, y2;
  if (auto_scale == true) {
    for (int i = 1; i < readings; i++ ) {
      if (DataArray[i] >= maxYscale) maxYscale = DataArray[i];
      if (DataArray[i] <= minYscale) minYscale = DataArray[i];
    }
    maxYscale = round(maxYscale + auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Max
    Y1Max = round(maxYscale + 0.5);
    if (minYscale != 0) minYscale = round(minYscale - auto_scale_margin); // Auto scale the graph and round to the nearest value defined, default was Y1Min
    Y1Min = round(minYscale);
  }
  // Draw the graph
  last_x = x_pos;
  last_y = y_pos + (Y1Max - constrain(DataArray[1], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight;
  display.drawRect(x_pos, y_pos, gwidth + 3, gheight + 2, GxEPD_BLACK);
  drawString(x_pos + gwidth / 2, y_pos - 2, title, CENTER);

  // Draw the data
  for (int gx = 0; gx < readings; gx++) {
    y2 = y_pos + (Y1Max - constrain(DataArray[gx], Y1Min, Y1Max)) / (Y1Max - Y1Min) * gheight + 1;
    if (barchart_mode) {
      x2 = x_pos + gx * (gwidth / readings) + 2;
      display.fillRect(x2, y2, (gwidth / readings) - 2, y_pos + gheight - y2 + 2, GxEPD_BLACK);
    } 
    else
    {
      x2 = x_pos + gx * gwidth / (readings - 1) + 1; // max_readings is the global variable that sets the maximum data that can be plotted
      display.drawLine(last_x, last_y, x2, y2, GxEPD_BLACK);
    }
    last_x = x2;
    last_y = y2;
  }
  //Draw the Y-axis scale
#define number_of_dashes 20
  for (int spacing = 0; spacing <= y_minor_axis; spacing++) {
    for (int j = 0; j < number_of_dashes; j++) { // Draw dashed graph grid lines
      if (spacing < y_minor_axis) display.drawFastHLine((x_pos + 3 + j * gwidth / number_of_dashes), y_pos + (gheight * spacing / y_minor_axis), gwidth / (2 * number_of_dashes), GxEPD_BLACK);
    }
    if ((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing) < 5 || title == TXT_PRESSURE_IN) {
      drawString(x_pos - 4, y_pos + gheight * spacing / y_minor_axis + 5, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 1), RIGHT);
    }
    else
    {
      if (Y1Min < 1 && Y1Max < 10)
        drawString(x_pos - 4, y_pos + gheight * spacing / y_minor_axis + 7, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 1), RIGHT);
      else
        drawString(x_pos - 4, y_pos + gheight * spacing / y_minor_axis + 7, String((Y1Max - (float)(Y1Max - Y1Min) / y_minor_axis * spacing + 0.01), 0), RIGHT);
    }
  }

  String lastDate = String(WxForecast[0].Period.substring(0,10));
  int day1X, day2X, day3X;
  for (int gx = 0; gx < readings; gx++) {
    if(String(WxForecast[gx].Period.substring(0,10)) != lastDate) {
      lastDate = String(WxForecast[gx].Period.substring(0,10));
      x2 = x_pos + gx * (gwidth / readings) + 2;
      drawString(x2, y_pos + gheight + 13, "|", LEFT);
    }
  }
  // for (int i = 0; i <= 2; i++) {
  //   drawString(15 + x_pos + gwidth / 3 * i, y_pos + gheight + 3, "|", LEFT);
  // }
}

void displayForecastWeather(int x, int y, int index, int width) {
  int fwidth = 73;
  int offset = (width - fwidth * 5) / 2;
  x = offset + x + fwidth * index;
  display.drawRect(x, y, fwidth - 1, 81, GxEPD_BLACK);
  display.drawLine(x, y + 17, x + fwidth - 3, y + 17, GxEPD_BLACK);
  displayWheatherIcon(x + fwidth / 2, y + 43, WxForecast[index].Icon, SmallIcon);
  drawString(x + fwidth / 2, y + 14, String(ConvertUnixTime(WxForecast[index].Dt + WxConditions[0].Timezone).substring(0,5)), CENTER);
  drawString(x + fwidth / 2, y + 79, String(WxForecast[index].High, 0) + "°/" + String(WxForecast[index].Low, 0) + "°", CENTER);
}

void displayForecastSection(int x, int y, int width) {
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
  drawGraph(x + gx + 0 * gap, gy, gwidth, gheight, 10, 30,    Units == "M" ? TXT_TEMPERATURE_C : TXT_TEMPERATURE_F, temperature_readings, max_readings, true, barchart_off);
  const int Rain_array_size = sizeof(rain_readings) / sizeof(float);
  const int Snow_array_size = sizeof(snow_readings) / sizeof(float);
  if (SumOfPrecip(rain_readings, Rain_array_size) >= SumOfPrecip(snow_readings, Snow_array_size))
    drawGraph(x + gx + 1 * gap + 5, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_RAINFALL_MM : TXT_RAINFALL_IN, rain_readings, Rain_array_size, autoscale_on, barchart_on);
  else drawGraph(x + gx + 1 * gap + 5, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_SNOWFALL_MM : TXT_SNOWFALL_IN, snow_readings, Snow_array_size, autoscale_on, barchart_on);
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
  drawString(iconCentreX + 20 + 5, iconCentreY, cloudCover + "  " + humidity, LEFT);

  String temperature = TXT_TEMPERATURES + ": " + String(WxConditions[0].Temperature, 1) + "°C (" + String(WxConditions[0].High, 0) + "°|" + String(WxConditions[0].Low, 0) + "°)";
  drawString(iconCentreX - 20, iconCentreY + tbh, temperature, LEFT);

  String slope_direction = TXT_PRESSURE_STEADY;
  if (WxConditions[0].Trend == "+") slope_direction = TXT_PRESSURE_RISING;
  if (WxConditions[0].Trend == "-") slope_direction = TXT_PRESSURE_FALLING;

  String preasure = TXT_PRESSURE + ": " + String(WxConditions[0].Pressure, 0) + "hPa (" + slope_direction + ")";
  drawString(iconCentreX - 20, iconCentreY + 2 * tbh, preasure, LEFT);

  String sunrise = TXT_SUNRISE + ": " + ConvertUnixTime(WxConditions[0].Sunrise + WxConditions[0].Timezone).substring(0, 5) + " " + TXT_SUNSET + ": " + ConvertUnixTime(WxConditions[0].Sunset + WxConditions[0].Timezone).substring(0, 5);
  drawString(iconCentreX - 20, iconCentreY + 3 * tbh, sunrise, LEFT);
}

void displayToday(int leftOffset, int topOffset, int width, int height) {
  display.drawRect(leftOffset, topOffset, width, height, GxEPD_BLACK);

  drawString(leftOffset + width / 2, topOffset + 17, applicationState.month, CENTER);
  drawString(leftOffset + width / 2, topOffset + height - 5, applicationState.weekDay, CENTER);
  drawString(leftOffset + width / 2, topOffset + height / 2 + 21, applicationState.date, CENTER, u8g2_font_fub42_tf);
}

void displayWeather() {
  const int leftOffset = CALENDAR_AREA_WIDTH;
  const int topOffset = STATUS_AREA_HEIGH + 10;
  const byte padding = 5;

  int tableWidth = 200; 
  int tableHeight = 140; 
  int tableLeftOffset = display.width() - tableWidth;
  int todayWitdth = display.width() - tableWidth - leftOffset - 5;
  display.setPartialWindow(leftOffset, topOffset, display.width() - leftOffset, display.height()- topOffset);
  display.firstPage();
  u8g2Fonts.setFont(u8g2_font_helvB12_tf);
  do
  {
    display.drawBitmap(display.width() - 200, topOffset, home, 200, 140, GxEPD_BLACK);

    u8g2Fonts.setCursor(tableLeftOffset + 61, topOffset + 28);
    u8g2Fonts.print(mqttTopics[5].tempValue + "°");
    u8g2Fonts.setCursor(tableLeftOffset + 61, topOffset + 63);
    u8g2Fonts.print(mqttTopics[4].tempValue + "°");
    u8g2Fonts.setCursor(tableLeftOffset + 61, topOffset + 98);
    u8g2Fonts.print(mqttTopics[3].tempValue + "°");
    u8g2Fonts.setCursor(tableLeftOffset + 61, topOffset + 133);
    u8g2Fonts.print(mqttTopics[1].tempValue + "°");
    u8g2Fonts.setCursor(tableLeftOffset + 130, topOffset + 133);
    u8g2Fonts.print(mqttTopics[0].tempValue + "°");
    u8g2Fonts.setCursor(tableLeftOffset + 130, topOffset + 63);
    u8g2Fonts.print(mqttTopics[2].tempValue + "°");

    displayConditionsSection(leftOffset, topOffset + tableHeight, display.width() - leftOffset, WxConditions[0].Icon);
    displayForecastSection(leftOffset, topOffset + tableHeight + 120, display.width() - leftOffset);
    displayToday(leftOffset, topOffset, todayWitdth, tableHeight);

  }
  while (display.nextPage());
}


void displayCalendarData() {
  const int topOffset = STATUS_AREA_HEIGH + 10;
  const int tableWidth = CALENDAR_AREA_WIDTH;
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

  display.setPartialWindow(0, topOffset, CALENDAR_AREA_WIDTH, display.width());
  display.firstPage();
  do
  {
    int currentHeight = topOffset;
    for(byte i = 0; i < daysNumber; i++) {
      currentHeight = drawEventsDay(currentHeight, maxTableHeight, calEvents[i].start, date_max_width, calEvents[i].events, CALENDAR_AREA_WIDTH - date_max_width);
      if (currentHeight == 0) {
        break;
      }
    }
  }
  while (display.nextPage());
}
