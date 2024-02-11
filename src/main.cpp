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

#define PWM_CHANNEL    0
#define PWM_FREQENCY   77500
#define PWM_RESOLUTION 8 // 1-16 bit
#define PWM_DUTY_ON    128
#define PWM_DUTY_OFF   0

static DCF77  dcf77;
static string dcf77_data;
static bool   time_synced;

static int dashSecs[] = {1, 15, 21, 29, 36, 42, 45, 50};

#ifdef CORE2
#include <AXP192.h>
#include <RTC.h>
AXP192 axp;
RTC    rtc;
#endif

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

void log_char(char c)
{
  Serial.print(c);
  dcf77_data.push_back(c);
}

void log_dcf77_data(int sec, int bit)
{
  if (time_synced) {
    if (sec == 0)
      dcf77_data.clear();
    if (find(begin(dashSecs), end(dashSecs), sec) != end(dashSecs))
      log_char('-');
    log_char('0' + bit);
  }
  else
    Serial.print('.');
}

void setup()
{
  Serial.begin(115200);

#ifdef CORE2
  axp.begin();
  axp.SetLcdVoltage(3300);
  RTC_TimeTypeDef tt;
  rtc.GetTime(&tt);
  timespec ts;
  ts.tv_sec = ((tt.Hours) * 60 + tt.Minutes) * 60 + tt.Seconds;
  ts.tv_nsec = 0;
  clock_settime(CLOCK_REALTIME, &ts);
  Serial.printf("RTC Time: %02d:%02d:%02d\n", tt.Hours, tt.Minutes, tt.Seconds);
#endif

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
  configTzTime(NTP_TZ, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  Serial.println(WiFi.isConnected() ? "Connected." : "Connection timeout.");

  // Configure carrier frequency using PWM
  Serial.printf("Using GPIO%d\n", DCF77_GPIO);
  ledcSetup(PWM_CHANNEL, PWM_FREQENCY, PWM_RESOLUTION);
  ledcAttachPin(DCF77_GPIO, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, PWM_DUTY_ON);
#ifdef USE_DISPLAY
  tft.fillScreen(DISPLAY_BG);
#endif
}

void loop()
{
  auto now = chrono::system_clock::now();
  // Wait next sec
  int msToNextSec = 1000 - std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
  delay(msToNextSec);
  now += chrono::milliseconds(msToNextSec);
  int sec = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count() % 60;
  if (sec < 59) {
    int bit = dcf77.getBit(sec);
    ledcWrite(PWM_CHANNEL, PWM_DUTY_OFF);
    delay(bit ? 200 : 100); // 1=200ms 0=100ms
    ledcWrite(PWM_CHANNEL, PWM_DUTY_ON);
    log_dcf77_data(sec, bit);
  }
  else {
    auto ttEncode = chrono::system_clock::to_time_t(now + chrono::seconds(1) + chrono::minutes(DCF77_TIME_OFS));
    auto tmEncode = localtime(&ttEncode);
    if (tmEncode->tm_year > 100) {
      time_synced = true;
      dcf77.setTime(tmEncode);
      Serial.print(tmEncode, "\n%a, %d.%m.%y %H:%M %Z ");
    }
  }

#ifdef USE_DISPLAY
  displayTime(now, sec == 0);
#endif
}
