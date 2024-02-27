//Wheather settings
String City             = "TORUN";                      // Your home city See: http://bulk.openweathermap.org/sample/
String Country          = "PL";                            // Your _ISO-3166-1_two-letter_country_code country code, on OWM find your nearest city and the country code is displayed
                                                           // https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes
String Language         = "PL";                            // NOTE: Only the weather description is translated by OWM
                                                           // Examples: Arabic (AR) Czech (CZ) English (EN) Greek (EL) Persian(Farsi) (FA) Galician (GL) Hungarian (HU) Japanese (JA)
                                                           // Korean (KR) Latvian (LA) Lithuanian (LT) Macedonian (MK) Slovak (SK) Slovenian (SL) Vietnamese (VI)
String Units            = "M";                             // Use 'M' for Metric or I for Imperial 
// const char* Timezone    = "GMT0BST,M3.5.0/01,M10.5.0/02";  // Choose your time zone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
                                                           // See below for examples
const char* ntpServer   = "0.pl.pool.ntp.org";             // Or, choose a time server close to you, but in most cases it's best to use pool.ntp.org to find an NTP server
                                                           // then the NTP system decides e.g. 0.pool.ntp.org, 1.pool.ntp.org as the NTP syem tries to find  the closest available servers
                                                           // EU "0.europe.pool.ntp.org"
                                                           // US "0.north-america.pool.ntp.org"
                                                           // See: https://www.ntppool.org/en/                                                           
int gmtOffset_sec     = 0;    // UK normal time is GMT, so GMT Offset is 0, for US (-5Hrs) is typically -18000, AU is typically (+8hrs) 28800
int daylightOffset_sec = 3600; // In the UK DST is +1hr or 3600-secs, other countries may use 2hrs 7200 or 30-mins 1800 or 5.5hrs 19800 Ahead of GMT use + offset behind - offset

const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe

// MQTT settings
String mqttTopicNames[] = {
  "dominikdeka@gmail.com/temperature/taras",
  "dominikdeka@gmail.com/temperature/loundry",
  "dominikdeka@gmail.com/temperature/front",
  "dominikdeka@gmail.com/temperature/salon",
  "dominikdeka@gmail.com/temperature/justyna",
  "dominikdeka@gmail.com/temperature/bedroom"
};