

//inspired by
/*
  https://www.dfrobot.com/blog-917.html
  https://github.com/bklevence/MagTag_Moon
  https://www.instructables.com/Wireless-Lunar-Phase-Tracker/
*/
#include <ctime>
#include <iostream>
#include <AiEsp32RotaryEncoder.h>
#include <AiEsp32RotaryEncoderNumberSelector.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <font/moon_phases34pt7b.h>
#include <font/FreeMono9pt7b.h>
#include <font/Envy_Code_R11pt7b.h>
#include <string.h>
#include "time.h"
#include "sntp.h"

//#include "secrets.ino"

#define TFT_SCK    36
#define TFT_MOSI   35
#define TFT_MISO   37
#define TFT_CS     34
#define TFT_DC     38
#define TFT_RESET  33

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

#define ROTARY_ENCODER_A_PIN 16
#define ROTARY_ENCODER_B_PIN 17
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_STEPS 5
#define ROTARY_ENCODER_VCC_PIN 4
AiEsp32RotaryEncoder *rotaryEncoder = new AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN,
    ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN,
    ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
AiEsp32RotaryEncoderNumberSelector numberSelector = AiEsp32RotaryEncoderNumberSelector();

// Color definitions
#define SNOW     0xF7DE
#define SKY      0x08A3

#define HOST "api.farmsense.net"

#define ENDPOINT "/v1/moonphases/?"

const char* cert = \
                   "-----BEGIN CERTIFICATE-----\n" \
                   "MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n" \
                   "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
                   "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n" \
                   "WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
                   "RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
                   "AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n" \
                   "R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n" \
                   "sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n" \
                   "NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n" \
                   "Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n" \
                   "/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n" \
                   "AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n" \
                   "Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n" \
                   "FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n" \
                   "AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n" \
                   "Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n" \
                   "gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n" \
                   "PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n" \
                   "ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n" \
                   "CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n" \
                   "lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n" \
                   "avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n" \
                   "yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n" \
                   "yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n" \
                   "hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n" \
                   "HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n" \
                   "MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n" \
                   "nLRbwHOoq7hHwg==\n" \
                   "-----END CERTIFICATE-----\n";

const char* ntpServer1 = "time.windows.com";
const char* ntpServer2 = "time.google.com";
const long  gmtOffset_sec = -25200;
const int   daylightOffset_sec = -21600;
const char* time_zone = "MST7MDT,M4.1.0,M10.5.0";
unsigned long epochTime;

Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ILI9341 display = Arduino_ILI9341(&bus, TFT_RESET);

// For HTTPS requests
WiFiClientSecure client;

const char* moon_data[7][2];
int moonIndex[7][1];
String moonReadableDate[7];

void IRAM_ATTR readEncoderISR()
{
  rotaryEncoder->readEncoder_ISR();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  wifiSetup();

  rotaryEncoder->begin();
  rotaryEncoder->setup(readEncoderISR);
  numberSelector.attachEncoder(rotaryEncoder);
  //sets initial value to 0, being the
  //beginning of the moonIndex array
  numberSelector.setRange(0, 6, 1, false, 0);
  numberSelector.setValue(0);

  setupGui();
}

void loop() {
  // put your main code here, to run repeatedly:

  //code in here is to use rotary encoder to cycle through moon phases of particular days
  //get input from rotary encoder
  if (rotaryEncoder->encoderChanged())
  {
    //call screen update function
    //Serial.println(numberSelector.getValue());
    updateDisplay(numberSelector.getValue());
  }
}

void wifiSetup() {
  ////////remove before turning in
  const char* ssid     = "";     // your network SSID (name of wifi network)
  const char* password = ""; // your network password

  String day1 = "";
  String day2 = "";
  String day3 = "";
  String day4 = "";
  String day5 = "";
  String day6 = "";
  String day7 = "";

  //this block of code is from WiFiClientSecure example
  //Initialize serial and wait for port to open:
  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);

  //get time in Unix epoch format for API
  epochTime = getLocalTime();
  day1 = String(epochTime);
  day2 = String(epochTime + 86400 * 1);
  day3 = String(epochTime + 86400 * 2);
  day4 = String(epochTime + 86400 * 3);
  day5 = String(epochTime + 86400 * 4);
  day6 = String(epochTime + 86400 * 5);
  day7 = String(epochTime + 86400 * 6);

  for (int i; i < 7; i++) {
    moonReadableDate[i] = getReadableDate(epochTime + 86400 * i);
  }

  //check cert
  client.setCACert(cert);

  Serial.println("\nStarting connection to server...");

  if (!client.connect(HOST, 443)) {
    Serial.println("Connection failed!");
    //maybe add something to the screen that indicates failure
    return;
  }
  else {
    Serial.println("Connected to server!");
    client.setTimeout(10000000);
    // Make an API request:
    client.println(String("GET ") + ENDPOINT + "d[]=" + day1 + "&d[]=" +
                   day2 + "&d[]=" + day3 + "&d[]=" + day4 + "&d[]=" + day5 +
                   "&d[]=" + day6 + "&d[]=" + day7 + " HTTP/1.1");
//    Serial.println(String("GET ") + ENDPOINT + "d[]=" + day1 + "&d[]=" +
//                   day2 + "&d[]=" + day3 + "&d[]=" + day4 + "&d[]=" + day5 +
//                   "&d[]=" + day6 + "&d[]=" + day7 + " HTTP/1.1");
    //client.println(String("GET /v1/moonphases/?d[]=1669431367&d[]=1669517767&d[]=1669604167&d[]=1669690567&d[]=1669776967&d[]=1669863367&d[]=1669949767 HTTP/1.1");

    client.println(String("Host: ") + HOST);
    client.println("Connection: close");
    client.println();
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  /*Uses the incredible ArduinoJSON library, this saved me so much time*/
  // Allocate the JSON document
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  //Json filter
  StaticJsonDocument<96> filter;

  JsonObject filter_0 = filter.createNestedObject();
  filter_0["Moon"][0] = true;
  filter_0["Index"] = true;
  filter_0["Phase"] = true;

  //deserialize Json
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  int pos = 0;
  for (JsonObject item : doc.as<JsonArray>()) {
    const char* Moon_0 = item["Moon"][0];
    int Index = item["Index"];
    const char* Phase = item["Phase"];

    //put extracted data into moon_data array
    moon_data[pos][0] = Moon_0;
    moon_data[pos][1] = Phase;
    moonIndex[pos][0] = Index;
    pos++; //increment
  }
  // Disconnect
  client.stop();
}

void setupGui() {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  //set up TFT
  display.begin();
  display.setRotation(4);

  //fill screen with dark blue color
  display.fillScreen(SKY);
  display.setFont(&Envy_Code_R11pt7b);
  display.setTextSize(1);
  display.setTextColor(SNOW);

  //updateDisplay(0);

//  //write date, preferably
//  display.getTextBounds(moonReadableDate[0], 0, 0, &x1, &y1, &width, &height);
//  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 290));
//  display.print(moonReadableDate[0]);
//  
//  //write info from api
//  //prints the farmer's almanac "nickname"
//  display.getTextBounds(moon_data[0][0], 0, 0, &x1, &y1, &width, &height);
//  display.setCursor((TFT_WIDTH - width - 30) / 2, (TFT_HEIGHT - 250));
//  display.printf("\"%s\"", moon_data[0][0]);
//
//  //prints the moon phase/illumination index from array
//  display.getTextBounds("Index: %d", 0, 0, &x1, &y1, &width, &height);
//  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 220));
//  display.printf("Index: %d", moonIndex[0][0]);
//
//  //print moon phase
//  display.getTextBounds(moon_data[0][1], 0, 0, &x1, &y1, &width, &height);
//  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 20));
//  display.println(moon_data[0][1]);
//
//  //change font to moon phases by Curtis Clark
//  display.setFont(&moon_phases34pt7b);
//  display.setTextSize(2);
//  display.getTextBounds("A", 0, 0, &x1, &y1, &width, &height);
//  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 60));
//  //later change to get illumination index
}

void updateDisplay(int i) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;
  char phase;
  int index;

  display.setFont(&Envy_Code_R11pt7b);
  display.setTextSize(1);
  
  //write date, preferably
  display.getTextBounds(moonReadableDate[i], 0, 0, &x1, &y1, &width, &height);
  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 290));
  display.fillRect(0, 0, 240, 40, SKY);
  display.print(moonReadableDate[i]);

  //display.fillScreen(SKY);
  //prints the farmer's almanac "nickname"
  display.getTextBounds(moon_data[i][0], 0, 0, &x1, &y1, &width, &height);
  display.setCursor((TFT_WIDTH - width - 30) / 2, (TFT_HEIGHT - 250));
  display.fillRect(0, 40, 240, 40, SKY);
  display.printf("\"%s\"", moon_data[i][0]);

  //prints the moon phase/illumination index from array
  display.getTextBounds("Index: %d", 0, 0, &x1, &y1, &width, &height);
  display.fillRect(0, 80, 240, 40, SKY);
  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 220));
  display.printf("Index: %d", moonIndex[i][0]);

  //print moon phase
  display.getTextBounds(moon_data[i][1], 0, 0, &x1, &y1, &width, &height);
  display.fillRect(0, 280, 240, 40, SKY);
  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 20));
  display.println(moon_data[i][1]);

  //change font to moon phases by Curtis Clark
  display.setFont(&moon_phases34pt7b);
  display.setTextSize(2);
  display.getTextBounds("A", 0, 0, &x1, &y1, &width, &height);
  display.fillRect(0, 120, 240, 150, SKY);
  display.setCursor((TFT_WIDTH - width) / 2, (TFT_HEIGHT - 60));
  index = moonIndex[i][0];
  phase = alphaMoon(index);
  display.print(phase);
}

unsigned long getLocalTime(){
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return (0);
  }
  time(&now);
  return now;
}

String getReadableDate(long int epoch) {
  char readable[100];
  strftime(readable, sizeof(readable), "%A %b %e", localtime(&epoch));
  return String(readable);
}

//the MagTag Moon project on github helped me out a lot here
char alphaMoon(int index) {
  int i = 0;

  char moonChar[] = {'A', 'B', 'C', 'D', 'E', 'F',
                     'G', 'H', 'I', 'J', 'K', 'L', 'M',
                     '@', 'N', 'O', 'P', 'Q', 'R', 'S',
                     'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '0'
                    };
                    
  //lower case uses characters without outline on moon
  //  char moonChar[] = {'a', 'b', 'c', 'd', 'e', 'f',
  //                     'g', 'h', 'i', 'j', 'k', 'l', 'm',
  //                     '@', 'n', 'o', 'p', 'q', 'r', 's',
  //                     't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '0'
  //                    };

  //this math is to round down so the indexing works
  //since the average lunar cycle is 29.53059 days
  i = int((index / 29.56) * 28);
  Serial.println(moonChar[i]);
  return moonChar[i];
}
