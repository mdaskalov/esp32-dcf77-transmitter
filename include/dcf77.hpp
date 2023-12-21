#pragma once

// https://en.wikipedia.org/wiki/DCF77
// https://www.dcf77logs.de/live

#include <string>

using namespace std;

class DCF77 {
  private:
    static char data[];

    void encodeBCD(int val, int start, int len)
    {
      for (int bit = 0; bit < len; bit++)
        data[start + bit] = '0' + ((((val / 10) << 4) + (val % 10) >> bit) & 1);
    }

    void evenParity(int start, int len)
    {
      char parity = '0';
      for (int bit = 0; bit < len; bit++)
        parity ^= data[start + bit] & 1;
      data[start + len] = parity;
    }

    void encodeDCF77(int tH, int tM, int dW, int dD, int dM, int dY, int dst)
    {
      encodeBCD(dst, 17, 2);
      encodeBCD(tM, 21, 7);
      evenParity(21, 7);
      encodeBCD(tH, 29, 6);
      evenParity(29, 6);
      encodeBCD(dD, 36, 6);
      encodeBCD(dW, 42, 3);
      encodeBCD(dM, 45, 5);
      encodeBCD(dY, 50, 8);
      evenParity(36, 22);
    }

  public:
    char getBit(int sec) { return data[sec % 60]; }
    void setTime(struct tm *local_time)
    {
      int tH = local_time->tm_hour;
      int tM = local_time->tm_min;
      int dW = local_time->tm_wday == 0 ? 7 : local_time->tm_wday;
      int dD = local_time->tm_mday;
      int dM = local_time->tm_mon + 1;
      int dY = local_time->tm_year % 100;
      int dst = local_time->tm_isdst > 0 ? 1 : 2; // 2=CET 1=CEST
      encodeDCF77(tH, tM, dW, dD, dM, dY, dst);
    }
};

char DCF77::data[] = {
    '0',                                                                  // 00: Start of minute
    '1', '1', '1', '1', '1', '1', '1', '1', '1', '0', '0', '0', '0', '0', // 01: Weather broadcast / Civil warning bits
    '0',                                                                  // 15: Call bit: abnormal transmitter operation
    '0',                                                                  // 16: Summer time announcement. Set during hour before change
    '0', '1',                                                             // 17: CEST=10 CET=01
    '0',                                                                  // 19: Leap second announcement. Set during hour before leap second
    '1',                                                                  // 20: Start of encoded time
    '0', '0', '0', '0', '0', '0', '0', '0',                               // 21: Minutes (7bit + parity, 00-59)
    '0', '0', '0', '0', '0', '0', '0',                                    // 29: Hours (6bit + parity, 0-23)
    '0', '0', '0', '0', '0',                                              // 36: Day of month (6bit, 1-31)
    '0', '0', '0',                                                        // 42: Day of week (3bit, 1-7, Monday=1)
    '0', '0', '0', '0', '0',                                              // 45: Month number (5bit, 1-12)
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',                     // 50: Year within century (8bit + parity, 00-99)
    '.'                                                                   // 59: Not used
};
