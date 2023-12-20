#pragma once

// https://en.wikipedia.org/wiki/DCF77
// https://www.dcf77logs.de/live

#include <string>

using namespace std;

class DCF77 {
  private:
    string data;

    string encodeBCD(int val, int bits)
    {
      string res;
      for (int bit = 0; bit < bits; bit++)
        res.push_back('0' + ((((val / 10) << 4) + (val % 10) >> bit) & 1));
      return res;
    }

    char evenParity(const string &val)
    {
      char parity = '0';
      for (auto c : val)
        parity ^= c & 1;
      return parity;
    }

    string encodeDCF77(int tH, int tM, int dW, int dD, int dM, int dY)
    {
      char   stMin = '0';
      string weather = "11111111100000";     // Weather broadcast / Civil warning bits
      char   fault = '0';                    // Call bit: abnormal transmitter operation
      char   dstAnn = '0';                   // Summer time announcement. Set during hour before change
      string dst = DST_OFFSET ? "10" : "01"; // DST
      char   leap = '0';                     // Leap second announcement. Set during hour before leap second
      char   stTime = '1';                   // Start of encoded time
      string min = encodeBCD(tM, 7);
      char   pMin = evenParity(min);
      string hour = encodeBCD(tH, 6);
      char   pHour = evenParity(hour);
      string mDay = encodeBCD(dD, 6);
      string wDay = encodeBCD(dW, 3);
      string mon = encodeBCD(dM, 5);
      string year = encodeBCD(dY, 8);
      char   pDate = evenParity(mDay + wDay + mon + year);
      return stMin + weather + fault + dstAnn + dst + leap + stTime + min + pMin + hour + pHour + mDay + wDay + mon + year + pDate;
    }

  public:
    void setTime(struct tm *local_time)
    {
      int tH = local_time->tm_hour;
      int tM = local_time->tm_min;
      int dW = local_time->tm_wday == 0 ? 7 : local_time->tm_wday;
      int dD = local_time->tm_mday;
      int dM = local_time->tm_mon + 1;
      int dY = local_time->tm_year % 100;
      data = encodeDCF77(tH, tM, dW, dD, dM, dY);
    }

    char getBit(int sec) { return (sec > data.length() ? '.' : data[sec]); }
};
