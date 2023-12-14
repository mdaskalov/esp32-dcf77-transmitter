// esp32-dcf77-transmitter
// Copyright (c) 2023 Milko Daskalov

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

#include <WiFi.h>
#include <driver/ledc.h>

#include "dcf77.hpp"

#define PWM_DUTY_ON  2
#define PWM_DUTY_OFF 0

static DCF77  dcf77;
static string dcf77_data;

#ifdef USE_DISPLAY
#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void displayMessage(const string &msg, int font = DISPLAY_MSG_FONT, bool clear = true)
{
  if (clear)
    tft.fillScreen(DISPLAY_BG);
  tft.setTextColor(DISPLAY_FG, DISPLAY_BG, true);
  tft.setTextDatum(CC_DATUM);
  tft.drawString(msg.c_str(), DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, font);
}

void displayTime(const chrono::system_clock::time_point &time, bool clear = true)
{
  char timeStr[10];
  auto tt = chrono::system_clock::to_time_t(time);
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&tt));
  if (timeStr[0] == '0')
    timeStr[0] = ' ';
  displayMessage(timeStr, DISPLAY_TIME_FONT, clear);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(dcf77_data.c_str(), DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DATA_FONT);
}
#endif

void add_dcf77_data(char c)
{
  Serial.print(c);
  dcf77_data.push_back(c);
}

void wait_next_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  delayMicroseconds(1000000 - tv.tv_usec);
}

void setup()
{
  Serial.begin(115200);

#ifdef USE_DISPLAY
  tft.init();
  tft.setRotation(DISPLAY_ROTATION);
  displayMessage("Connecting...");
#endif

  // connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  Serial.printf("Connecting to %s...\n", WIFI_SSID);
  WiFi.waitForConnectResult();
  configTime(GMT_OFFSET * 3600, DST_OFFSET * 3600, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  Serial.println("Connected.");

  // Configure carrier frequency using PWM
  ledcSetup(DCF77_PWM_CHAN, DCF77_PWM_FREQ, DCF77_PWM_RES);
  ledcAttachPin(DCF77_PWM_PIN, DCF77_PWM_CHAN);
  ledcWrite(DCF77_PWM_CHAN, PWM_DUTY_ON);
#ifdef USE_DISPLAY
  tft.fillScreen(DISPLAY_BG);
#endif
  Serial.printf("ESP: Free: %i, Min: %i, Size: %i, Alloc: %i\n", ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getHeapSize(), ESP.getMaxAllocHeap());
}

void loop()
{
  wait_next_sec();
  auto now = chrono::system_clock::now();
  int  sec = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count() % 60;
  if (sec == 0) {
    dcf77_data.clear();
    auto ttNextMin = chrono::system_clock::to_time_t(now + chrono::minutes(1));
    auto tmNextMin = localtime(&ttNextMin);
    dcf77.setTime(tmNextMin);
    Serial.print(tmNextMin, "\n%a, %d.%m.%y %H:%M ");
  }
  int bit = dcf77.getBit(sec);
  if (bit < 0)
    add_dcf77_data('.');
  else if (sec <= 58) {
    for (auto s : {1, 15, 21, 29, 36, 42, 45, 50}) {
      if (s == sec)
        add_dcf77_data('-');
    }
    add_dcf77_data('0' + bit);
    ledcWrite(DCF77_PWM_CHAN, PWM_DUTY_OFF);
    delayMicroseconds(bit ? 200000 : 100000); // 1=200ms 0=100ms
    ledcWrite(DCF77_PWM_CHAN, PWM_DUTY_ON);
  }
#ifdef USE_DISPLAY
  displayTime(now, sec == 0);
#endif
}
