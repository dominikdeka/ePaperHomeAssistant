//Temperature - Humidity - Forecast
const String TXT_FORECAST_VALUES  = "Prognoza 3-dniowa";
const String TXT_FORECAST  = "Prognoza";
const String TXT_CONDITIONS       = "Warunki";
const String TXT_DAYS             = "(dni)";
const String TXT_TEMPERATURES     = "Temperatura";
const String TXT_TEMPERATURE_C    = "Temperatura (°C)";
const String TXT_TEMPERATURE_F    = "Temperatura (°F)";
const String TXT_HUMIDITY_PERCENT = "Wilgotnosc (%)";
const String TXT_HUMIDITY = "Wilgotnosc";

// Pressure
const String TXT_PRESSURE         = "Cisnienie";
const String TXT_PRESSURE_HPA     = "Cisnienie (hPa)";
const String TXT_PRESSURE_IN      = "Cisnienie (in)";
const String TXT_PRESSURE_STEADY  = "stale";
const String TXT_PRESSURE_RISING  = "rosnie";
const String TXT_PRESSURE_FALLING = "spada";

//RainFall / SnowFall
const String TXT_RAINFALL_MM = "Deszcz (mm)";
const String TXT_RAINFALL_IN = "Deszcz (in)";
const String TXT_SNOWFALL_MM = "Snieg (mm)";
const String TXT_SNOWFALL_IN = "Snieg (in)";
const String TXT_PRECIPITATION_SOON = "Opad.";


//Sun
const String TXT_SUNRISE  = "Wsch.";
const String TXT_SUNSET   = "Zach.";

//Moon
const String TXT_MOON_NEW             = "Now";
const String TXT_MOON_WAXING_CRESCENT = "Przybywający sierp";
const String TXT_MOON_FIRST_QUARTER   = "Pierwsza kwadra";
const String TXT_MOON_WAXING_GIBBOUS  = "Przybywający garbaty";
const String TXT_MOON_FULL            = "Pelnia";
const String TXT_MOON_WANING_GIBBOUS  = "Ubywający garbaty";
const String TXT_MOON_THIRD_QUARTER   = "Ostatnia kwadra";
const String TXT_MOON_WANING_CRESCENT = "Ubywający sierp ";

//Power / WiFi
const String TXT_POWER  = "Zasilanie";
const String TXT_WIFI   = "WiFi";
const String TXT_VOLTAGE   = "bateria: ";
const String TXT_CHARGE_BATTERY   = "naladuj baterie!";
const char* TXT_UPDATED = "aktualizacja:";


//Wind
//These are left untranslated due to string length constraints, but usually we use the English notation anyway
const String TXT_WIND_SPEED_DIRECTION = "Predkosc/Kierunek wiatru";
const String TXT_N   = "N";
const String TXT_NNE = "NNE";
const String TXT_NE  = "NE";
const String TXT_ENE = "ENE";
const String TXT_E   = "E";
const String TXT_ESE = "ESE";
const String TXT_SE  = "SE";
const String TXT_SSE = "SSE";
const String TXT_S   = "S";
const String TXT_SSW = "SSW";
const String TXT_SW  = "SW";
const String TXT_WSW = "WSW";
const String TXT_W   = "W";
const String TXT_WNW = "WNW";
const String TXT_NW  = "NW";
const String TXT_NNW = "NNW";

//Day of the week
const char* weekday_D[] = { "Niedziela", "Poniedzialek", "Wtorek", "Sroda", "Czwartek", "Piatek", "Sobota" };

//Month
const char* month_M[] = { "Styczen", "Luty", "Marzec", "Kwiecien", "Maj", "Czerwiec", "Lipiec", "Sierpien", "Wrzesien", "Pazdziernik", "Listopad", "Grudzien" };

String modes[] = { "kompaktowy", "kalendarz", "pogoda"};

String phases[] = { "laczenie wifi...", "pobieranie czasu...", "pobieranie kalendarza...", "pobieranie odczytow...", "pobieranie prognozy...", "drukowanie..." };
