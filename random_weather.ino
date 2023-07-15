
/* Reference: 
 https://randomnerdtutorials.com/esp8266-nodemcu-https-requests
 https://www.youtube.com/watch?v=_L28Y0UNH-4
 https://arduinojson.org/
 https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
 https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C // See datasheet for Address (Try 0X3D if failure)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

// Edit API call interval here (optional)
unsigned long interval = 30 * 1000; // in milliseconds; Set interval to 30 seconds

// Initialise time
unsigned long lastTime = 0;

// Weather symbol (thanks to https://www.youtube.com/watch?v=_L28Y0UNH-4)
// 'Clear_Daylight', 24x24px
const unsigned char clear_day [] PROGMEM = {
  0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x10, 0x18, 0x08, 0x38, 0x00, 0x1c, 0x1c, 0x3c, 0x38, 0x08, 
  0xff, 0x10, 0x01, 0xc1, 0x80, 0x03, 0x00, 0xc0, 0x06, 0x00, 0x60, 0x06, 0x00, 0x60, 0x0c, 0x00, 
  0x30, 0xec, 0x00, 0x37, 0xec, 0x00, 0x37, 0x0c, 0x00, 0x30, 0x06, 0x00, 0x60, 0x06, 0x00, 0x60, 
  0x03, 0x00, 0xc0, 0x01, 0xc3, 0x80, 0x08, 0xff, 0x10, 0x1c, 0x3c, 0x38, 0x38, 0x00, 0x1c, 0x10, 
  0x18, 0x08, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00
};

// 'Clear_Night', 24x24px
const unsigned char clear_night [] PROGMEM = {
  0x08, 0x08, 0x80, 0x08, 0x1c, 0xc0, 0x1c, 0x08, 0xe0, 0x3e, 0x00, 0xf0, 0xff, 0x80, 0xd8, 0x3e, 
  0x00, 0xcc, 0x1c, 0x00, 0xc6, 0x08, 0x01, 0x86, 0x08, 0x01, 0x83, 0x00, 0x03, 0x03, 0x00, 0x03, 
  0x03, 0x40, 0x06, 0x03, 0xe0, 0x06, 0x03, 0x40, 0x1c, 0x03, 0x00, 0x78, 0x03, 0x01, 0xe0, 0x03, 
  0xff, 0x80, 0x06, 0x7e, 0x00, 0x06, 0x30, 0x00, 0x0c, 0x18, 0x00, 0x18, 0x0c, 0x00, 0x30, 0x07, 
  0x00, 0xe0, 0x03, 0xff, 0xc0, 0x00, 0xff, 0x00
};

// 'Thunderstorm', 24x24px
const unsigned char thunderstorm [] PROGMEM = {
  0x01, 0xf0, 0x00, 0x03, 0xf8, 0x70, 0x06, 0x0c, 0xf8, 0x1c, 0x07, 0x8c, 0x38, 0x03, 0x06, 0x60, 
  0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 0x00, 0x07, 0x7f, 0xff, 
  0xfe, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x10, 0x1e, 0x02, 0x20, 0x24, 0x04, 0x40, 0x48, 0x08, 
  0xf8, 0x9f, 0x9f, 0x11, 0x00, 0x82, 0x21, 0xfd, 0x04, 0x40, 0x0a, 0x08, 0x80, 0x14, 0x10, 0x00, 
  0x28, 0x00, 0x00, 0x50, 0x00, 0x00, 0x60, 0x00
};

// 'Drizzle', 24x24px
const unsigned char drizzle [] PROGMEM = {
  0x01, 0xf0, 0x00, 0x03, 0x18, 0x70, 0x06, 0x0c, 0xf8, 0x1c, 0x07, 0x8c, 0x38, 0x03, 0x06, 0x60, 
  0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 0x00, 0x07, 0x7f, 0xff, 
  0xfe, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x18, 0x61, 0x86, 0x18, 0x61, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x30, 0xc3, 0x0c, 0x30, 0xc3, 0x0c
};

// 'Rain', 24x24px
const unsigned char rain [] PROGMEM = {
  0x01, 0xf0, 0x00, 0x03, 0xf8, 0x70, 0x06, 0x0c, 0xf8, 0x1c, 0x07, 0x8c, 0x38, 0x03, 0x06, 0x60,
  0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 0x00, 0x07, 0x7f, 0xff, 
  0xfe, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x84, 0x21, 0x21, 0x08, 0x42, 
  0x42, 0x10, 0x84, 0x84, 0x21, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x84, 0x21, 0x21, 
  0x08, 0x42, 0x42, 0x10, 0x84, 0x84, 0x21, 0x08
};

// 'Snow', 24x24px
const unsigned char snow [] PROGMEM = {
  0x01, 0xf0, 0x00, 0x03, 0x18, 0x70, 0x06, 0x0c, 0xf8, 0x1c, 0x07, 0x8c, 0x38, 0x03, 0x06, 0x60, 
  0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 0x00, 0x07, 0x7f, 0xff, 
  0xfe, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0xa0, 0x50, 0x20, 0x40, 0x20, 0xa8, 0xa0, 0x50, 0x70, 
  0x00, 0x01, 0xfc, 0x04, 0x00, 0x70, 0x15, 0x00, 0xa8, 0x0e, 0x00, 0x20, 0x3f, 0x80, 0x00, 0x0e, 
  0x0a, 0x05, 0x15, 0x04, 0x02, 0x04, 0x0a, 0x05
};

// 'Fog', 24x24px
const unsigned char fog [] PROGMEM = {
  0x01, 0xf0, 0x00, 0x03, 0x18, 0x70, 0x06, 0x0c, 0xf8, 0x1c, 0x07, 0x8c, 0x38, 0x03, 0x06, 0x60, 
  0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 0x00, 0x07, 0x7f, 0xff, 
  0xfe, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xc3, 0x0c, 0x79, 0xe7, 0x9e, 
  0xcf, 0x3c, 0xf3, 0x86, 0x18, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xc3, 0x0c, 0x79, 
  0xe7, 0x9e, 0xcf, 0x3c, 0xf3, 0x86, 0x18, 0x61
};

// 'Clouds_Daylight', 24x24px
const unsigned char clouds_day [] PROGMEM = {
  0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x10, 0x18, 0x08, 0x38, 0x00, 0x1c, 0x1c, 0x3c, 0x38, 0x08, 
  0xff, 0x10, 0x01, 0xc3, 0x80, 0x03, 0x00, 0xc0, 0x06, 0x00, 0x60, 0x06, 0x00, 0x60, 0x0c, 0x00, 
  0x30, 0xec, 0x00, 0x37, 0xed, 0xf0, 0x37, 0x0f, 0xf8, 0x70, 0x06, 0x0c, 0xf8, 0x1c, 0x07, 0x8c, 
  0x38, 0x03, 0x06, 0x60, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xe0, 
  0x00, 0x07, 0x7f, 0xff, 0xfe, 0x3f, 0xff, 0xfc
};

// 'Clouds_Night', 24x24px
const unsigned char clouds_night [] PROGMEM = {
  0x00, 0x00, 0x02, 0x00, 0x01, 0x07, 0x04, 0x01, 0xc2, 0x04, 0x01, 0xe0, 0x0e, 0x01, 0xb0, 0x3f, 
  0x81, 0x98, 0x0e, 0x01, 0x8c, 0x04, 0x03, 0x0c, 0x04, 0x03, 0x06, 0x00, 0x03, 0x06, 0x00, 0x06, 
  0x03, 0x00, 0x06, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x38, 0x03, 0x01, 0xf0, 0x06, 0x7f, 0xc0, 0x06, 
  0x3e, 0x00, 0x0c, 0x30, 0x00, 0x0c, 0x18, 0x00, 0x18, 0x0c, 0x00, 0x30, 0x07, 0x00, 0xe0, 0x43, 
  0xc3, 0xc2, 0xe0, 0xff, 0x07, 0x40, 0x3c, 0x02
};

// 'Cloudy', 24x24px
const unsigned char cloudy [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x01, 0xf0, 0xe0, 0x03, 0x19, 0xf0, 0x0e, 
  0x0f, 0x1c, 0x1c, 0x06, 0x0e, 0x30, 0x00, 0x03, 0x60, 0x00, 0x03, 0x61, 0xc0, 0x03, 0x63, 0xe1, 
  0xc3, 0x36, 0x33, 0xe6, 0x3c, 0x1e, 0x3c, 0x78, 0x0c, 0x1c, 0xe0, 0x00, 0x06, 0xc0, 0x00, 0x06, 
  0xc0, 0x00, 0x06, 0xc0, 0x00, 0x06, 0xe0, 0x00, 0x0c, 0x7f, 0xff, 0xf8, 0x3f, 0xff, 0xf0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Country code according to ISO3166 (edited slightly)
const char* countrycode[249][2] PROGMEM = {
    {"AD", "Andorra"},
    {"AE", "United Arab Emirates"},
    {"AF", "Afghanistan"},
    {"AG", "Antigua and Barbuda"},
    {"AI", "Anguilla"},
    {"AL", "Albania"},
    {"AM", "Armenia"},
    {"AO", "Angola"},
    {"AQ", "Antarctica"},
    {"AR", "Argentina"},
    {"AS", "American Samoa"},
    {"AT", "Austria"},
    {"AU", "Australia"},
    {"AW", "Aruba"},
    {"AX", "Ã…land Islands"},
    {"AZ", "Azerbaijan"},
    {"BA", "Bosnia"},
    {"BB", "Barbados"},
    {"BD", "Bangladesh"},
    {"BE", "Belgium"},
    {"BF", "Burkina Faso"},
    {"BG", "Bulgaria"},
    {"BH", "Bahrain"},
    {"BI", "Burundi"},
    {"BJ", "Benin"},
    {"BL", "Saint BarthÃ©lemy"},
    {"BM", "Bermuda"},
    {"BN", "Brunei Darussalam"},
    {"BO", "Bolivia"},
    {"BQ", "BES Islands"},
    {"BR", "Brazil"},
    {"BS", "Bahamas"},
    {"BT", "Bhutan"},
    {"BV", "Bouvet Island"},
    {"BW", "Botswana"},
    {"BY", "Belarus"},
    {"BZ", "Belize"},
    {"CA", "Canada"},
    {"CC", "Cocos Islands"},
    {"CD", "DR Congo"},
    {"CF", "Central African Re.."},
    {"CG", "Congo"},
    {"CH", "Switzerland"},
    {"CI", "CÃ´te d'Ivoire"},
    {"CK", "Cook Islands"},
    {"CL", "Chile"},
    {"CM", "Cameroon"},
    {"CN", "China"},
    {"CO", "Colombia"},
    {"CR", "Costa Rica"},
    {"CU", "Cuba"},
    {"CV", "Cabo Verde"},
    {"CW", "CuraÃ§ao"},
    {"CX", "Christmas Island"},
    {"CY", "Cyprus"},
    {"CZ", "Czechia"},
    {"DE", "Germany"},
    {"DJ", "Djibouti"},
    {"DK", "Denmark"},
    {"DM", "Dominica"},
    {"DO", "Dominican Republic"},
    {"DZ", "Algeria"},
    {"EC", "Ecuador"},
    {"EE", "Estonia"},
    {"EG", "Egypt"},
    {"EH", "Western Sahara"},
    {"ER", "Eritrea"},
    {"ES", "Spain"},
    {"ET", "Ethiopia"},
    {"FI", "Finland"},
    {"FJ", "Fiji"},
    {"FK", "Falkland Islands"},
    {"FM", "Micronesia"},
    {"FO", "Faroe Islands"},
    {"FR", "France"},
    {"GA", "Gabon"},
    {"GB", "United Kingdom"},
    {"GD", "Grenada"},
    {"GE", "Georgia"},
    {"GF", "French Guiana"},
    {"GG", "Guernsey"},
    {"GH", "Ghana"},
    {"GI", "Gibraltar"},
    {"GL", "Greenland"},
    {"GM", "Gambia"},
    {"GN", "Guinea"},
    {"GP", "Guadeloupe"},
    {"GQ", "Equatorial Guinea"},
    {"GR", "Greece"},
    {"GS", "South Georgia and .."},
    {"GT", "Guatemala"},
    {"GU", "Guam"},
    {"GW", "Guinea-Bissau"},
    {"GY", "Guyana"},
    {"HK", "Hong Kong"},
    {"HM", "Heard Island and M.."},
    {"HN", "Honduras"},
    {"HR", "Croatia"},
    {"HT", "Haiti"},
    {"HU", "Hungary"},
    {"ID", "Indonesia"},
    {"IE", "Ireland"},
    {"IL", "Israel"},
    {"IM", "Isle of Man"},
    {"IN", "India"},
    {"IO", "British Indian Oce.."},
    {"IQ", "Iraq"},
    {"IR", "Iran"},
    {"IS", "Iceland"},
    {"IT", "Italy"},
    {"JE", "Jersey"},
    {"JM", "Jamaica"},
    {"JO", "Jordan"},
    {"JP", "Japan"},
    {"KE", "Kenya"},
    {"KG", "Kyrgyzstan"},
    {"KH", "Cambodia"},
    {"KI", "Kiribati"},
    {"KM", "Comoros"},
    {"KN", "Saint Kitts and Ne.."},
    {"KP", "North Korea"},
    {"KR", "South Korea"},
    {"KW", "Kuwait"},
    {"KY", "Cayman Islands"},
    {"KZ", "Kazakhstan"},
    {"LA", "Laos"},
    {"LB", "Lebanon"},
    {"LC", "Saint Lucia"},
    {"LI", "Liechtenstein"},
    {"LK", "Sri Lanka"},
    {"LR", "Liberia"},
    {"LS", "Lesotho"},
    {"LT", "Lithuania"},
    {"LU", "Luxembourg"},
    {"LV", "Latvia"},
    {"LY", "Libya"},
    {"MA", "Morocco"},
    {"MC", "Monaco"},
    {"MD", "Moldova"},
    {"ME", "Montenegro"},
    {"MF", "Saint Martin"},
    {"MG", "Madagascar"},
    {"MH", "Marshall Islands"},
    {"MK", "North Macedonia"},
    {"ML", "Mali"},
    {"MM", "Myanmar"},
    {"MN", "Mongolia"},
    {"MO", "Macao"},
    {"MP", "Northern Mariana I.."},
    {"MQ", "Martinique"},
    {"MR", "Mauritania"},
    {"MS", "Montserrat"},
    {"MT", "Malta"},
    {"MU", "Mauritius"},
    {"MV", "Maldives"},
    {"MW", "Malawi"},
    {"MX", "Mexico"},
    {"MY", "Malaysia"},
    {"MZ", "Mozambique"},
    {"NA", "Namibia"},
    {"NC", "New Caledonia"},
    {"NE", "Niger"},
    {"NF", "Norfolk Island"},
    {"NG", "Nigeria"},
    {"NI", "Nicaragua"},
    {"NL", "Netherlands"},
    {"NO", "Norway"},
    {"NP", "Nepal"},
    {"NR", "Nauru"},
    {"NU", "Niue"},
    {"NZ", "New Zealand"},
    {"OM", "Oman"},
    {"PA", "Panama"},
    {"PE", "Peru"},
    {"PF", "French Polynesia"},
    {"PG", "Papua New Guinea"},
    {"PH", "Philippines"},
    {"PK", "Pakistan"},
    {"PL", "Poland"},
    {"PM", "Saint Pierre and M.."},
    {"PN", "Pitcairn"},
    {"PR", "Puerto Rico"},
    {"PS", "Palestine, State of"},
    {"PT", "Portugal"},
    {"PW", "Palau"},
    {"PY", "Paraguay"},
    {"QA", "Qatar"},
    {"RE", "RÃ©union"},
    {"RO", "Romania"},
    {"RS", "Serbia"},
    {"RU", "Russia"},
    {"RW", "Rwanda"},
    {"SA", "Saudi Arabia"},
    {"SB", "Solomon Islands"},
    {"SC", "Seychelles"},
    {"SD", "Sudan"},
    {"SE", "Sweden"},
    {"SG", "Singapore"},
    {"SH", "Saint Helena, Asce.."},
    {"SI", "Slovenia"},
    {"SJ", "Svalbard and Jan M.."},
    {"SK", "Slovakia"},
    {"SL", "Sierra Leone"},
    {"SM", "San Marino"},
    {"SN", "Senegal"},
    {"SO", "Somalia"},
    {"SR", "Suriname"},
    {"SS", "South Sudan"},
    {"ST", "Sao Tome and Princ.."},
    {"SV", "El Salvador"},
    {"SX", "Sint Maarten"},
    {"SY", "Syrian Arab Republic"},
    {"SZ", "Eswatini"},
    {"TC", "Turks and Caicos I.."},
    {"TD", "Chad"},
    {"TF", "French Southern Te.."},
    {"TG", "Togo"},
    {"TH", "Thailand"},
    {"TJ", "Tajikistan"},
    {"TK", "Tokelau"},
    {"TL", "Timor-Leste"},
    {"TM", "Turkmenistan"},
    {"TN", "Tunisia"},
    {"TO", "Tonga"},
    {"TR", "Turkey"},
    {"TT", "Trinidad and Tobago"},
    {"TV", "Tuvalu"},
    {"TW", "Taiwan"},
    {"TZ", "Tanzania"},
    {"UA", "Ukraine"},
    {"UG", "Uganda"},
    {"UM", "United States Mino.."},
    {"US", "United States"},
    {"UY", "Uruguay"},
    {"UZ", "Uzbekistan"},
    {"VA", "Holy See"},
    {"VC", "Saint Vincent and .."},
    {"VE", "Venezuela"},
    {"VG", "British Virgin Isl.."},
    {"VI", "U.S. Virgin Islands"},
    {"VN", "Viet Nam"},
    {"VU", "Vanuatu"},
    {"WF", "Wallis and Futuna"},
    {"WS", "Samoa"},
    {"YE", "Yemen"},
    {"YT", "Mayotte"},
    {"ZA", "South Africa"},
    {"ZM", "Zambia"},
    {"ZW", "Zimbabwe"}
};

StaticJsonDocument<4096> doc; // Calculated by referring to https://arduinojson.org/v6/assistant

void setup() {
  // I2C pins for ESP-01
  Wire.begin(0, 2); // Edit this line if not using ESP-01

  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.println(F("Connecting to WiFi"));

  // Connect to WiFi using WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("RandomWeatherESP8266");

  Serial.println(F("Connected!"));
}

void loop() {
  if (((millis() - lastTime) > interval) || (lastTime == 0)) { // Fetch every interval seconds stated above or at the beginning
    lastTime = millis();
    delay(2000); // Prevent multiple API calls if error occurs (not affecting interval)
    if(WiFi.status() == WL_CONNECTED) {
      String response;
      DeserializationError error;

      // Fetch random location
      const char* random_location_path = "https://api.3geonames.org/?randomland=yes&json=1";

      response = httpsGETRequest(random_location_path);

      // Deserialize the JSON document
      error = deserializeJson(doc, response);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      // Parse details of major city
      String city = doc["major"]["city"];
      String state = doc["major"]["state"];
      String latt = doc["major"]["latt"];
      String longt = doc["major"]["longt"];

      // Fetch API again if no city given
      if (city == "{}") {
        return;
      }

      // Fetch current weather of given coordinate
      String weather_path = "https://api.open-meteo.com/v1/forecast?latitude=" + latt + "&longitude=" + longt + "&current_weather=true&timezone=auto";

      response = httpsGETRequest(weather_path.c_str());

      error = deserializeJson(doc, response);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      // Parse weather of location
      String temperature = doc["current_weather"]["temperature"];
      int weathercode = doc["current_weather"]["weathercode"];
      int is_day = doc["current_weather"]["is_day"];
      String time = doc["current_weather"]["time"];

      // Fetch API again if no temperature
      if (temperature == "null") {
        return;
      }

      // Clear the display
      display.clearDisplay();

      // Draw frame
      display.drawRect(0, 0, 128, 64, WHITE);
      display.drawLine(0, 35, 127, 35, WHITE);

      // Draw current weather diagram
      if ((weathercode == 0) || (weathercode == 1)) {
        if (is_day == 1) {
          display.drawBitmap(6, 6, clear_day, 24, 24, WHITE);
        } else if (is_day == 0) {
          display.drawBitmap(6, 6, clear_night, 24, 24, WHITE);
        }
      } else if (weathercode == 2) {
        if (is_day == 1) {
          display.drawBitmap(6, 6, clouds_day, 24, 24, WHITE);
        } else if (is_day == 0) {
          display.drawBitmap(6, 6, clouds_night, 24, 24, WHITE);
        }
      } else if (weathercode == 3) {
        display.drawBitmap(6, 6, cloudy, 24, 24, WHITE);
      } else if ((weathercode == 45) || (weathercode == 48)) {
        display.drawBitmap(6, 6, fog, 24, 24, WHITE);
      } else if ((weathercode >= 51) && (weathercode <= 57)) {
        display.drawBitmap(6, 6, drizzle, 24, 24, WHITE);
      } else if ((weathercode >= 61) && (weathercode <= 67)) {
        display.drawBitmap(6, 6, rain, 24, 24, WHITE);
      } else if ((weathercode >= 71) && (weathercode <= 77)) {
        display.drawBitmap(6, 6, snow, 24, 24, WHITE);
      } else if ((weathercode >= 80) && (weathercode <= 82)) {
        display.drawBitmap(6, 6, rain, 24, 24, WHITE);
      } else if ((weathercode >= 85) && (weathercode <= 86)) {
        display.drawBitmap(6, 6, snow, 24, 24, WHITE);
      } else if ((weathercode >= 95) && (weathercode <= 99)) {
        display.drawBitmap(6, 6, thunderstorm, 24, 24, WHITE);
      }
      
      display.setTextColor(WHITE);

      // Display current temperature
      display.setTextSize(2);
      display.setCursor(35, 5);
      display.print(temperature);
      display.print((char)247);
      display.print("C");

      // Display current local time and whether there is daylight
      display.setTextSize(1);
      display.setCursor(35, 24);

      int hrs = time.substring(11, 13).toInt();
      if (hrs == 0) {
        display.print("12am");
      } else if ((hrs >= 1) && (hrs <= 11)) {
        display.print(hrs);
        display.print("am");
      } else if (hrs == 12) {
        display.print("12pm");
      } else {
        display.print(hrs - 12);
        display.print("pm");
      }

      display.print(" ");
  
      if (is_day == 1) {
        display.print("(Day)");
      } else if (is_day == 0) {
        display.print("(Night)");
      }

      // Display city
      display.setCursor(6, 39);
      if (city.length() > 20) {
        display.println(city.substring(0, 18) + "..");
      } else {
        display.println(city);
      }

      // Display country
      display.setCursor(6, 51);
      if (state == "{}") {
        display.println("-");
      } else {
        // Search country code value using binary search
        int low = 0;
        int high = 248;
        int mid;
        const char* key = state.c_str();
        
        while (low <= high) {
            mid = (low + high) / 2;
            if (strcmp(countrycode[mid][0], key) == 0) {
                display.println(countrycode[mid][1]);
                break;
            } else if (strcmp(countrycode[mid][0], key) > 0) {
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }
      }

      display.display();
       
    } else {
      Serial.println(F("WiFi Disconnected"));
    }
  }
}

// Code from https://randomnerdtutorials.com/esp8266-nodemcu-https-requests/#esp8266-https-requests-no-certificate
// HTTPS request is made without verifying certificate as no sensitive information is involved in this project. However, to verify certificate, kindly refer to website above.
String httpsGETRequest(const char* serverName) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  // Ignore SSL certificate validation
  client->setInsecure();
  
  //Create an HTTPClient instance
  HTTPClient https;

  String payload = "{}";
  
  //Initializing an HTTPS communication using the secure client
  if (https.begin(*client, serverName)) {  // HTTPS
    // start connection and send HTTP header
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("Code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("Error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("Unable to connect\n");
  }
  return payload;
}