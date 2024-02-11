#pragma once

// https://en.wikipedia.org/wiki/DCF77
// https://www.dcf77logs.de/live

#include <string>

#define DST_BIT     17
#define MIN_BIT     21
#define HOUR_BIT    29
#define DAY_BIT     36
#define WEEKDAY_BIT 42
#define MONTH_BIT   45
#define YEAR_BIT    50

using namespace std;

class DCF77 {
  private:
    static int dcf77_bits[];

    void encodeBCD(int start, int len, int val)
    {
      int byte = ((val / 10) << 4) + (val % 10);
      for (int bit = 0; bit < len; bit++)
        dcf77_bits[start + bit] = (byte >> bit) & 1;
    }

    void evenParity(int start, int len)
    {
      char parity = 0;
      for (int bit = 0; bit < len; bit++)
        parity ^= dcf77_bits[start + bit] & 1;
      dcf77_bits[start + len] = parity;
    }

  public:
    int  getBit(int sec) { return dcf77_bits[sec % 60]; }
    void setTime(struct tm *tm)
    {
      encodeBCD(DST_BIT, 2, tm->tm_isdst > 0 ? 1 : 2);
      encodeBCD(MIN_BIT, 7, tm->tm_min);
      evenParity(MIN_BIT, 7);
      encodeBCD(HOUR_BIT, 6, tm->tm_hour);
      evenParity(HOUR_BIT, 6);
      encodeBCD(DAY_BIT, 6, tm->tm_mday);
      encodeBCD(WEEKDAY_BIT, 3, tm->tm_wday == 0 ? 7 : tm->tm_wday);
      encodeBCD(MONTH_BIT, 5, tm->tm_mon + 1);
      encodeBCD(YEAR_BIT, 8, tm->tm_year % 100);
      evenParity(DAY_BIT, 22);
    }
};

int DCF77::dcf77_bits[] = {
    0,                                        // 00: Start of minute
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 01: Weather broadcast / Civil warning bits
    0,                                        // 15: Call bit: abnormal transmitter operation
    0,                                        // 16: Summer time announcement. Set during hour before change
    0, 1,                                     // 17: 01=CET, 10=CEST
    0,                                        // 19: Leap second announcement. Set during hour before leap second
    1,                                        // 20: Start of encoded time
    0, 0, 0, 0, 0, 0, 0, 0,                   // 21: Minutes (7bit + parity, 00-59)
    0, 0, 0, 0, 0, 0, 0,                      // 29: Hours (6bit + parity, 0-23)
    0, 0, 0, 0, 0, 0,                         // 36: Day of month (6bit, 1-31)
    0, 0, 0,                                  // 42: Day of week (3bit, 1-7, Monday=1)
    0, 0, 0, 0, 0,                            // 45: Month number (5bit, 1-12)
    0, 0, 0, 0, 0, 0, 0, 0, 0,                // 50: Year within century (8bit + parity, 00-99)
    0                                         // 59: Not used
};
