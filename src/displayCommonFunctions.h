uint16_t drawString(int x, int y, String text, String align = "left", const uint8_t *fontName = u8g2_font_helvB12_tf);
int drawEventsDay(byte section, String date, int col1W, std::vector<String> events, int col2W);
int wrapText(const char* text, int x, int y, int maxWidth);

//#########################################################################################
uint16_t drawString(int x, int y, String text, String align, const uint8_t *fontName) {
  u8g2Fonts.setFont(fontName);
  uint16_t width = u8g2Fonts.getUTF8Width(text.c_str());
  if (align == "right")  x = x - width;
  if (align == "center") x = x - width / 2;
  u8g2Fonts.setCursor(x, y);
  u8g2Fonts.print(text);
  return width;
}

int drawEventsDay(byte section, int currentHeight, int maxTableHeight, String date, int col1W, std::vector<String> events, int col2W) {
    u8g2Fonts.setFont(u8g2_font_helvB14_te);
    int16_t ta = u8g2Fonts.getFontAscent(); // positive
    int16_t td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
    int dateHeight = ta - td; // text box height
    u8g2Fonts.setCursor(section * LEFT_SECTION_WIDTH, currentHeight + dateHeight);
    u8g2Fonts.print(date);

    u8g2Fonts.setFont(u8g2_font_luRS18_tf);
    byte eventsNumber = events.size();
    // int16_t newHeight = currentHeight + eventsNumber * lineHeight;
    // if (newHeight > maxTableHeight) {
    //   return 0;
    // }
    ta = u8g2Fonts.getFontAscent(); // positive
    td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
    int16_t lineHeight = ta - td; // text box height

    int eventsHeight = 0;
    for(byte j = 0; j < eventsNumber; j++) {
      eventsHeight += wrapText(events[j].c_str(), section * LEFT_SECTION_WIDTH + col1W + 5, currentHeight + eventsHeight, col2W);
      if (currentHeight + eventsHeight + lineHeight > maxTableHeight) {
        return 0;
      }
    }
    return currentHeight + eventsHeight;
}
int wrapText(const char* text, int x, int y, int maxWidth) {
  int16_t ta = u8g2Fonts.getFontAscent(); // positive
  int16_t td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
  int16_t lineHeight = ta - td; // text box height

  uint8_t length = strlen(text);
  char buffer[length + 1];
  strcpy(buffer, text);

  char* token = strtok(buffer, " ");
  int16_t currentWidth = 0;
  int lines = 1;
  u8g2Fonts.setCursor(x, y + lines * lineHeight);
  while (token != NULL) {
    int16_t tokenWidth = u8g2Fonts.getUTF8Width(token);
    // Serial.println(token);
    // Serial.println(tokenWidth);
    if (currentWidth + tokenWidth > maxWidth) {
      currentWidth = 0;
      lines++;
      u8g2Fonts.setCursor(x, y + lines * lineHeight);
    }
    u8g2Fonts.print(token);
    u8g2Fonts.print(" ");
    currentWidth += tokenWidth + u8g2Fonts.getUTF8Width(" ");
    token = strtok(NULL, " ");
  }
  return lines * lineHeight;
}