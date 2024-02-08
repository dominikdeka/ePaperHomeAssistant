struct Bounds {
    int16_t tbx; 
    int16_t tby; 
    uint16_t tbw;
    uint16_t tbh;
};

// String  Time_str, Date_str; // strings to hold time and received weather data
// int     wifi_signal, CurrentHour = 0, CurrentMin = 0, CurrentSec = 0;
// long    StartTime = 0;

#define autoscale_on  true
#define autoscale_off false
#define barchart_on   true
#define barchart_off  false

// float pressure_readings[max_readings]    = {0};
float temperature_readings[max_readings] = {0};
// float humidity_readings[max_readings]    = {0};
float rain_readings[max_readings]        = {0};
float snow_readings[max_readings]        = {0};

void displayCurrentState();
void drawGraph(int x_pos, int y_pos, int gwidth, int gheight, float Y1Min, float Y1Max, String title, float DataArray[], int readings, boolean auto_scale, boolean barchart_mode);
void displayForecastWeather(int x, int y, int index, int width);
void displayForecastSection(int x, int y);
void displayConditionsSection(int x, int y, int width, String IconName);
void displayToday(int leftOffset, int topOffset, int width, int height);
int displayWeather(int leftOffset);
int displayCalendarData();
int displayCalendarData2();
struct Bounds getMaxBounds(String words[], byte size);

void displayCurrentState() {
  display.setRotation(0);
  display.setFont(&FreeSerifBold12pt7b);

  String modeMsg = modes[applicationState.viewMode];

  byte modesNo = sizeof(modes) / sizeof(String);
  Bounds modesBounds[3];
  int accuw = 0;
  int16_t tbx, tby; uint16_t tbw, tbh;
  for(int i = modesNo - 1; i >= 0; i--) {
    display.getTextBounds(modes[i], accuw, 0, &tbx, &tby, &tbw, &tbh);
    accuw += tbw;
    modesBounds[i] = {tbx, tby, tbw, tbh};
  }

  Bounds bounds = getMaxBounds(modes, sizeof(modes) / sizeof(String));
  uint16_t maxh = bounds.tbh;

  String phaseMsg = phases[applicationState.currentPhase];
  display.getTextBounds(phaseMsg, 0, 0, &tbx, &tby, &tbw, &tbh);
  int16_t phaseX = 0 - tbx;
  int16_t phaseY = 0 - tby;
  uint16_t phaseW = tbw;
  if (tbh > maxh) {
    maxh = tbh;
  }

  String batteryMsg = "bateria: " + String(applicationState.voltage) + 'V';
  display.getTextBounds(batteryMsg, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t batteryX = (display.width() - 30 - accuw + phaseW)/2 - tbw/2 - bounds.tbx;
  uint16_t batteryY = 0 - bounds.tby;

  display.setPartialWindow(0, 0, display.width(), maxh + 8);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    for(int i = modesNo - 1; i >= 0; i--) {    
      int x = display.width() - modesBounds[i].tbw - (modesNo - i + 1) * 10 - modesBounds[i].tbx;
      int y = 0 - modesBounds[i].tby;
      if (i == applicationState.viewMode) {
        display.setTextColor(GxEPD_WHITE);
        display.fillRect(x - 5, 0, modesBounds[i].tbw + 10, maxh + 8, GxEPD_BLACK);
      }
      display.setCursor(x, y + 5);
      display.print(modes[i]);
      display.setTextColor(GxEPD_BLACK);
    }

    display.setCursor(batteryX, batteryY + 5);
    display.print(batteryMsg);

    display.setCursor(phaseX, phaseY + 5);
    display.print(phaseMsg);
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

  int16_t tbx, tby; uint16_t tbw, accuw, tbh;
  display.getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
  display.setCursor(x_pos + (gwidth-tbw) / 2, y_pos - 15 - tby);
  display.print(title);

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
  display.setFont(&FreeSerif9pt7b);
  int f = 0;
  do {
    displayForecastWeather(x, y, f, width);
    f++;
  } while (f <= 4);
  // Pre-load temporary arrays with with data - because C parses by reference
  int r = 0;
  do {
    // if (Units == "I") pressure_readings[r] = WxForecast[r].Pressure * 0.02953;   else pressure_readings[r] = WxForecast[r].Pressure;
    if (Units == "I") rain_readings[r]     = WxForecast[r].Rainfall * 0.0393701; else rain_readings[r]     = WxForecast[r].Rainfall;
    if (Units == "I") snow_readings[r]     = WxForecast[r].Snowfall * 0.0393701; else snow_readings[r]     = WxForecast[r].Snowfall;
    temperature_readings[r] = WxForecast[r].Temperature;
    // humidity_readings[r]    = WxForecast[r].Humidity;
    r++;
  } while (r < max_readings);
  int gwidth = 150, gheight = 72;
  int gx = (width - gwidth * 2) / 3 + 5;
  int gy = y + 100;
  int gap = gwidth + gx;
  // display.setFont(&FreeSerif9pt7b);
  // drawString(SCREEN_WIDTH / 2, gy - 40, TXT_FORECAST_VALUES, CENTER); // Based on a graph height of 60
  // display.setFont(&FreeSerif9pt7b);
  // (x,y,width,height,MinValue, MaxValue, Title, Data Array, AutoScale, ChartMode)
  // DrawGraph(gx + 0 * gap, gy, gwidth, gheight, 900, 1050, Units == "M" ? TXT_PRESSURE_HPA : TXT_PRESSURE_IN, pressure_readings, max_readings, autoscale_on, barchart_off);
  drawGraph(x + gx + 0 * gap, gy, gwidth, gheight, 10, 30,    Units == "M" ? TXT_TEMPERATURE_C : TXT_TEMPERATURE_F, temperature_readings, max_readings, autoscale_on, barchart_off);
  // DrawGraph(gx + 2 * gap, gy, gwidth, gheight, 0, 100,   TXT_HUMIDITY_PERCENT, humidity_readings, max_readings, autoscale_off, barchart_off);
  const int Rain_array_size = sizeof(rain_readings) / sizeof(float);
  const int Snow_array_size = sizeof(snow_readings) / sizeof(float);
  if (SumOfPrecip(rain_readings, Rain_array_size) >= SumOfPrecip(snow_readings, Snow_array_size))
    drawGraph(x + gx + 1 * gap + 5, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_RAINFALL_MM : TXT_RAINFALL_IN, rain_readings, Rain_array_size, autoscale_on, barchart_on);
  else drawGraph(x + gx + 1 * gap + 5, gy, gwidth, gheight, 0, 30, Units == "M" ? TXT_SNOWFALL_MM : TXT_SNOWFALL_IN, snow_readings, Snow_array_size, autoscale_on, barchart_on);
}
//#########################################################################################
void displayConditionsSection(int x, int y, int width, String IconName) {
  int iconCentreX = x + 86; int iconCentreY =  y + 70;
  displayWheatherIcon(iconCentreX, iconCentreY, IconName, LargeIcon);

  int16_t tbx, tby; uint16_t tbw, accuw, tbh;

  iconCentreX = iconCentreX + 90;
  iconCentreY = iconCentreY - 45;

  addcloud(iconCentreX - 9, iconCentreY - 3, Small * 0.5, 2); // Cloud top left
  addcloud(iconCentreX + 3, iconCentreY - 3, Small * 0.5, 2); // Cloud top right
  addcloud(iconCentreX, iconCentreY, Small * 0.5, 2); // Main cloud
  display.setFont(&FreeSerif9pt7b);
  String cloudCover = String(WxConditions[0].Cloudcover) + "%";
  String humidity = TXT_HUMIDITY + ": " + String(WxConditions[0].Humidity, 0) + "%";
  // display.getTextBounds(cloudCover, 0, 0, &tbx, &tby, &tbw, &tbh);
  // display.setCursor(iconCentreX + 20, iconCentreY - 10 - tby);
  // display.print(cloudCover);
  accuw = tbw;
  tbh = 22;
  drawString(iconCentreX + 20 + 5, iconCentreY, cloudCover + "  " + humidity, LEFT);
  // display.getTextBounds(humidity, 0, 0, &tbx, &tby, &tbw, &tbh);
  // display.setCursor(iconCentreX + 24 + accuw + 5, iconCentreY - 10 - tby);
  // display.print(humidity);

  String temperature = TXT_TEMPERATURES + ": " + String(WxConditions[0].Temperature, 1) + "°C (" + String(WxConditions[0].High, 0) + "°|" + String(WxConditions[0].Low, 0) + "°)";
  drawString(iconCentreX - 20, iconCentreY + tbh, temperature, LEFT);
  // display.getTextBounds(temperature, 0, 0, &tbx, &tby, &tbw, &tbh);
  // display.setCursor(iconCentreX - 20, iconCentreY - 5 - tby + tbh);
  // display.print(temperature);

  String slope_direction = TXT_PRESSURE_STEADY;
  if (WxConditions[0].Trend == "+") slope_direction = TXT_PRESSURE_RISING;
  if (WxConditions[0].Trend == "-") slope_direction = TXT_PRESSURE_FALLING;

  String preasure = TXT_PRESSURE + ": " + String(WxConditions[0].Pressure, 0) + "hPa (" + slope_direction + ")";
  drawString(iconCentreX - 20, iconCentreY + 2 * tbh, preasure, LEFT);
  // display.getTextBounds(preasure, 0, 0, &tbx, &tby, &tbw, &tbh);
  // display.setCursor(iconCentreX - 20, iconCentreY - tby + 2 * tbh);
  // display.print(preasure);

  String sunrise = TXT_SUNRISE + ": " + ConvertUnixTime(WxConditions[0].Sunrise + WxConditions[0].Timezone).substring(0, 5) + " " + TXT_SUNSET + ": " + ConvertUnixTime(WxConditions[0].Sunset + WxConditions[0].Timezone).substring(0, 5);
  drawString(iconCentreX - 20, iconCentreY + 3 * tbh, sunrise, LEFT);
  // display.getTextBounds(sunrise, 0, 0, &tbx, &tby, &tbw, &tbh);
  // display.setCursor(iconCentreX - 20, iconCentreY + 15 - tby + 3 * tbh);
  // display.print(sunrise);
}

void displayToday(int leftOffset, int topOffset, int width, int height) {
  display.drawRect(leftOffset, topOffset, width, height, GxEPD_BLACK);

  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.setFont(&FreeSerif9pt7b);
  display.getTextBounds(dateTime.month, 0, 0, &tbx, &tby, &tbw, &tbh);
  display.setCursor(leftOffset + (width - tbw) / 2, topOffset + 5 - tby);
  display.print(dateTime.month);

  display.setFont(&FreeSerifBold24pt7b);
  display.getTextBounds(dateTime.date, 0, 0, &tbx, &tby, &tbw, &tbh);
  display.setCursor(leftOffset + (width - tbw) / 2, topOffset + (height - tbh) / 2 - tby);
  display.print(dateTime.date);

  display.setFont(&FreeSerif9pt7b);
  display.getTextBounds(dateTime.weekDay, 0, 0, &tbx, &tby, &tbw, &tbh);
  display.setCursor(leftOffset + (width - tbw) / 2, topOffset + height - tbh - tby - 5);
  display.print(dateTime.weekDay);
}

int displayWeather(int leftOffset) {
  const int topOffset = 35;
  const byte padding = 5;

  display.setRotation(0);
  display.setFont(&FreeSerif9pt7b);
  display.setTextColor(GxEPD_BLACK);

  byte topicsNumber = sizeof(mqttTopics) / sizeof(MqttTopic);
  byte otherNumber = 0;
  String values[topicsNumber + otherNumber];
  String labels[topicsNumber + otherNumber];
  struct Bounds valuesBounds[topicsNumber + otherNumber];
  struct Bounds labelsBounds[topicsNumber + otherNumber];
  for(byte i = 0; i < topicsNumber + otherNumber; i++) {
    labels[i] = mqttTopics[i].label + ": ";
    values[i] = mqttTopics[i].tempValue + "°C/" + mqttTopics[i].humValue + "%";
  }

  // labels[topicsNumber + 2] = "cisnienie:";
  // values[topicsNumber + 2] = String(WxConditions[0].Pressure, 0) + "hPa";
  // labels[topicsNumber + 3] = "temperatura:";
  // values[topicsNumber + 3] = String(WxConditions[0].Temperature, 1) + " C";

  for(byte i = 0; i < topicsNumber + otherNumber; i++) {
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(values[i], 0, 0, &tbx, &tby, &tbw, &tbh);
    valuesBounds[i] = {tbx, tby, tbw, tbh};

    display.getTextBounds(labels[i], 0, 0, &tbx, &tby, &tbw, &tbh);
    labelsBounds[i] = {tbx, tby, tbw, tbh};
  }

    // Serial.print(tbh);
    // Serial.print(tbh * multiple);
    // Serial.print(tby);
  Bounds maxValuesBounds = getMaxBounds(values, topicsNumber + otherNumber);
  Bounds maxLabelsBounds = getMaxBounds(labels, topicsNumber + otherNumber);
  int tableWidth = 200; //maxValuesBounds.tbw + maxLabelsBounds.tbw + 10 + (2 * padding);
  int tableHeight = 140; //maxLabelsBounds.tbh * (topicsNumber + otherNumber) + (2 * padding);
  Serial.println(String(tableHeight));
  Serial.println(String(maxValuesBounds.tbw));
  Serial.println(String(maxLabelsBounds.tbw));
  int tableLeftOffset = display.width() - tableWidth;
  int todayWitdth = display.width() - tableWidth - leftOffset - 5;
  display.setPartialWindow(leftOffset, topOffset, display.width() - leftOffset, display.height()- topOffset);
  display.firstPage();
  u8g2Fonts.setFont(u8g2_font_helvB12_tf);
  do
  {

    // display.fillRect(display.width() - 200, topOffset, 200, 140, GxEPD_BLACK);
    // display.fillRect(GxEPD_WHITE);
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
    // for(byte i = 0; i < topicsNumber + otherNumber; i++) {

    //   uint16_t x = tableLeftOffset + 10 + maxLabelsBounds.tbw;
    //   uint16_t y = topOffset + padding + (maxLabelsBounds.tbh * i) - valuesBounds[i].tby;
    //   display.setTextColor(GxEPD_BLACK);
    //   display.setCursor(x, y);
    //   display.print(values[i]);
    //   // x = tableLeftOffset + maxLabelsBounds.tbw - labelsBounds[i].tbw;
    //   // y = topOffset + padding + (maxLabelsBounds.tbh * i) - labelsBounds[i].tby;
    //   // display.setTextColor(GxEPD_BLACK);
    //   // display.setCursor(x, y);
    //   // display.print(labels[i]);
    // }
    displayConditionsSection(leftOffset, topOffset + tableHeight, display.width() - leftOffset, WxConditions[0].Icon);
    displayForecastSection(leftOffset, topOffset + tableHeight + 120, display.width() - leftOffset);
    displayToday(leftOffset, topOffset, todayWitdth, tableHeight);

  }
  while (display.nextPage());

  return topOffset + maxLabelsBounds.tbh * 6 + 10;
}

int displayCalendarData() {
  const int offset = 35;
  const int padding = 5;
  display.setRotation(0);
  display.setFont(&FreeSerif12pt7b);

  byte daysNumber = calEvents.size();
  typedef struct {
      Bounds start;
      std::vector<Bounds> events;
  } calDateBounds;
  std::vector<calDateBounds> calEventBounds;


  uint16_t tbw_date_max, tbh_date_max;
  uint16_t tbw_max, tbh_max;
  // display.getTextBounds(calEvents[0].start, 0, 0, &tbx_date, &tby_date, &tbw_date, &tbh_date);
  byte eventCounter = 0;
  for(byte i = 0; i < daysNumber; i++) {
    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(calEvents[i].start, 0, 0, &tbx, &tby, &tbw, &tbh);
    if (tbw > tbw_date_max) {
      tbw_date_max = tbw;
    }
    if (tbh > tbh_date_max) {
      tbh_date_max = tbh;
    }

    calDateBounds bounds;
    bounds.start = {tbx, tby, tbw, tbh};

    byte eventsNumber = calEvents[i].events.size();
    for(byte j = 0; j < eventsNumber; j++) {
      display.getTextBounds(calEvents[i].events[j], 0, 0, &tbx, &tby, &tbw, &tbh);

      if (tbw > tbw_max) {
        tbw_max = tbw;
      }
      if (tbh > tbh_max) {
        tbh_max = tbh;
      }
      bounds.events.push_back({tbx, tby, tbw, tbh});

      eventCounter ++;
    }
    calEventBounds.push_back(bounds);
  }


  display.setPartialWindow(0, offset, tbw_date_max + tbw_max + 10, tbh_max * eventCounter + padding * 2);
  display.firstPage();
  do
  {
    display.setTextColor(GxEPD_WHITE);
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, offset, tbw_date_max + 10, tbh_date_max * eventCounter + padding * 2, GxEPD_BLACK);    
    byte eventCounter = 0;
    for(byte i = 0; i < daysNumber; i++) {

      uint16_t x = 5 - calEventBounds[i].start.tbx;
      uint16_t y = offset + padding + (eventCounter * tbh_max) - calEventBounds[i].events[0].tby;

      display.setCursor(x, y);
      display.print(calEvents[i].start);
      byte eventsNumber = calEvents[i].events.size();
      eventCounter += eventsNumber;
    }
  // }
  // while (display.nextPage());


  // // display.setPartialWindow(tbw_date + 15, offset, tbw_max + 15, tbh_max * eventCounter + padding * 2);
    display.setTextColor(GxEPD_BLACK);
  // display.firstPage();
  // do
  // {
  //   display.fillScreen(GxEPD_WHITE);
    eventCounter = 0;
    for(byte i = 0; i < daysNumber; i++) {
      byte eventsNumber = calEvents[i].events.size();
      for(byte j = 0; j < eventsNumber; j++) {
        uint16_t w = tbw_date_max + 15 - calEventBounds[i].events[j].tbx;
        uint16_t z = offset + padding + (eventCounter * tbh_max) - calEventBounds[i].events[j].tby;
        display.setCursor(w, z);
        display.print(calEvents[i].events[j]);
        eventCounter ++;
      }
    }
  }
  while (display.nextPage());

  return tbw_date_max + tbw_max + 25;
}

int displayCalendarData2() {
  const int offset = 35;
  const int tableWidth = 430;
  const int maxTableHeight = display.height() - offset;

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

  display.setPartialWindow(0, offset, tableWidth, display.width());
  display.firstPage();
  do
  {
    int currentHeight = offset;
    for(byte i = 0; i < daysNumber; i++) {
      currentHeight = drawEventsDay(currentHeight, maxTableHeight, calEvents[i].start, date_max_width, calEvents[i].events, tableWidth - date_max_width);
      if (currentHeight == 0) {
        break;
      }
    }
  }
  while (display.nextPage());

  return tableWidth;
}

struct Bounds getMaxBounds(String words[], byte size) {
  struct Bounds bounds = {0, 0, 0, 0};
  for (byte i=0;i<size;i++) {
    int16_t tmp_tbx, tmp_tby; uint16_t tmp_tbw, tmp_tbh;
    display.getTextBounds(words[i], 0, 0, &tmp_tbx, &tmp_tby, &tmp_tbw, &tmp_tbh);
    if (tmp_tbw > bounds.tbw) {
      bounds.tbx = tmp_tbx;
      bounds.tbw = tmp_tbw;
    }    
    if (tmp_tbh > bounds.tbh) {
      bounds.tby = tmp_tby;
      bounds.tbh = tmp_tbh;
    }
  }
  return bounds;
}
