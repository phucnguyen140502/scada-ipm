
#include "TinyGPS++.h"

#ifndef __RTCDateTime__   // nếu mảng lưu thời gian chưa được tạo
#define __RTCDateTime__   // đánh dấu đã tạo
struct RTCDateTime {      // mảng dữ liệu lưu thời gian
  uint16_t year = 0;      // năm
  uint8_t month = 0;      // tháng
  uint8_t day = 0;        // ngày
  uint8_t hour = 0;       // giờ
  uint8_t minute = 0;     // phút
  uint8_t second = 0;     // giây
  uint8_t dayOfWeek = 0;  // thứ
  uint32_t unixtime = 0;  // thời gian dài tính bằng giây từ ngày 1/1/1970
};                        //
#endif                    //




#ifndef GPS_time_h  //
#define GPS_time_h  // đánh dấu đẫ đọc

#define UUNIXDATE_BASE 946684800       // thời gian bù trừ từ ngày 1/1/1970 đến ngày 1/1/2000
class GPS_time : public TinyGPSPlus {  // thư viện GPS time được sử dụng hàm của TinyGPS
public:                                // biến mảng toàn cục


  RTCDateTime getDateTime(void) {  // hàm đọc thời gian
    RTCDateTime new_time;          // tạo mảng lưu thòi gian

    new_time.hour = time.hour();
    new_time.minute = time.minute();
    new_time.second = time.second();

    // Lấy ngày, tháng, năm
    new_time.day = date.day();
    new_time.month = date.month();
    new_time.year = date.year();

    unsigned long new_time_unixtime = ConverterDateTimeToUnixtime(new_time) + 7ul * 3600ul;  // chuyển thành dạng thời gian unixtime + zone


    if (new_time_unixtime > old_read_unixtime) {                                                                  // nếu thời gian mới đọc được lớn hơn thời gian đọc lần trước
      Time_long = new_time_unixtime;                                                                              // cập nhật thời gian mới
      Time_read = millis();                                                                                       // cập nhật thời gian đọc
      if (Time_long_offset == 0) {                                                                                // nếu chưa có dữ liệu
        Time_long_offset = new_time_unixtime;                                                                     // cập nhật thời gian mới
        Time_read_offset = millis();                                                                              // cập nhật thời gian đọc
      } else if ((Time_long - Time_long_offset > 60) && (Time_long - Time_long_offset < 14ul * 24ul * 3600ul)) {  // nếu khoảng cách giữa hai thời gian kiểm tra đủ lâu
        Time_scale = double(Time_long - Time_long_offset) / (double(Time_read - Time_read_offset) / 1000.0);      // tính sai số thạch anh
      }                                                                                                           //
    }                                                                                                             //

    old_read_unixtime = new_time_unixtime;                                        // đánh dấu đã đọc
    if (Time_read == 0) return ConverterDateTimeToRTCDateTime(0, 0, 0, 0, 0, 0);  // nêu chưa từng có dữ liệu trả về thời gian bằng 0
    if (Time_read > millis()) Time_read = millis();                               // nếu bị tràn
    if ((millis() - Time_read) * Time_scale >= 60000) {
      Time_read += int(30000 / Time_scale);
      Time_long += 30;
    }

    //   lcd.setCursor(0, 1);                                                                      // đặt con trỏ
    //   lcd.print(30000 / Time_scale, 6);                                                           // hiển thị số đt
    return ConverterUnixtimeToDateTime(Time_long + int((millis() - Time_read) / 1000.0 * Time_scale));  // trả thời gian được tính toán
  }                                                                                                     //



  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////converter time

  uint32_t ConverterDateTimeToUnixtime(RTCDateTime dt) {                                           // hàm chuyển về Unixtime
    return ConverterDateTimeToUnixtime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);  // hàm chuyển về Unixtime
  }

  uint32_t ConverterDateTimeToUnixtime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {  // hàm chuyển về Unixtime
    int32_t days = day - 1;                                                                                                        // trừ đi một ngày
    for (int i = 1; i < month; ++i) {                                                                                              // lặp các thàng
      if (i == 2) {                                                                                                                // nếu là thàng 2
        days += (28 + isLeapYear(year));                                                                                           // có 28 hoặc 29 ngày
      } else if (i == 4 || i == 6 || i == 9 || i == 11) {                                                                          // nếu là thàng 4,6,9,11
        days += 30;                                                                                                                // có 30 ngày
      } else {                                                                                                                     // các thàng còn lại
        days += 31;                                                                                                                // 31 ngày
      }                                                                                                                            //
    }                                                                                                                              //
    for (uint16_t i = 2000; i < year; ++i)
    {                                                                                                                              // lặp các năm từ năm 2000
      days += (365 + isLeapYear(i));                                                                                               // tính số ngày từng năm
    } //

    return UUNIXDATE_BASE + (((days * 24 + hour) * 60 + minute) * 60) + second;  // chuyển về Unixtime
  }                                                                              //

  RTCDateTime ConverterDateTimeToRTCDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {  // hàm chuyển kiểu dữ liệu
    RTCDateTime t;                                                                                                                       // biến đệm lưu trữ
    t.year = year;                                                                                                                       // lưu
    t.month = month;                                                                                                                     // lưu
    t.day = day;                                                                                                                         // lưu
    t.hour = hour;                                                                                                                       // lưu
    t.minute = minute;                                                                                                                   // lưu
    t.second = second;                                                                                                                   // lưu
    return t;                                                                                                                            // trả nề kết quả
  }                                                                                                                                      //



  RTCDateTime ConverterUnixtimeToDateTime(uint32_t ut) {  // hàm chuyển Unixtime về Date Time
    RTCDateTime t;                                        // biến đệm lưu dữ liệu

    t.unixtime = ut;                       // lưu Unixtime
    t.dayOfWeek = ((ut / 86400) + 4) % 7;  // tính thứ của tuần
    uint32_t tt = ut - UUNIXDATE_BASE;     // chuyển Unixtime từ 1970 về 2000
    t.second = tt % 60;                    // tính số giây
    tt /= 60;                              // chia 60
    t.minute = tt % 60;                    // tính số phút
    tt /= 60;                              // chia 60
    t.hour = tt % 24;                      // tính số giờ
    int16_t days = tt / 24;                // tính tổng số ngày
    bool leap;                             // biến đệm kiểm tra năm nhuận
    t.year = 2000;                         // năm bắt đầu từ năm 2000
    while (true) {                         // lặp đến khi đủ điều kiện
      leap = isLeapYear(t.year);           // tính xem có phải năm nhuận không
      if (days < 365 + leap) {             // nếu số ngày còn lại nhỏ hơn một năm
        break;                             // thoát while
      }                                    //
      days -= (365 + leap);                // trừ đi số ngày của năm
      ++t.year;                            // cộng năm thêm 1
    }                                      //

    for (t.month = 1; t.month < 12; ++t.month) {                                   // lặp các tháng
      int8_t daysPerMonth = 31;                                                    // mặc định là 31 ngày
      if (t.month == 2) {                                                          // nếu là thàng 2
        daysPerMonth = leap ? 29 : 28;                                             // có 28 hoặc 29 ngày
      } else if (t.month == 4 || t.month == 6 || t.month == 9 || t.month == 11) {  // nếu là thàng 4,6,9,11
        daysPerMonth = 30;                                                         // có 30 ngày
      }                                                                            //
      if (days < daysPerMonth) {                                                   // nếu số ngày còn lại nhỏ hơn một thàng
        break;                                                                     // thoát for
      }                                                                            //
      days -= daysPerMonth;                                                        // trừ đi số ngày của tháng
    }                                                                              //
    t.day = days + 1;                                                              // tính ngày của tháng

    return t;  // trả về kết quả
  }            //



  String strDayOfWeek(uint8_t dayOfWeek) {
    switch (dayOfWeek) {
      case 1: return "Monday"; 
      case 2: return "Tuesday";
      case 3: return "Wednesday"; 
      case 4: return "Thursday";
      case 5: return "Friday";
      case 6: return "Saturday"; 
      case 7: return "Sunday";
      default: return "Unknown";
    }
  }



  String strMonth(uint8_t month) {
    switch (month) {
      case 1: return "January"; 
      case 2: return "February"; 
      case 3: return "March"; 
      case 4: return "April";
      case 5: return "May";
      case 6: return "June";
      case 7: return "July"; 
      case 8: return "August"; 
      case 9: return "September"; 
      case 10: return "October";
      case 11: return "November";
      case 12: return "December"; 
      default: return "Unknown";
    }
  }

  bool isLeapYear(uint16_t year) {
    return ((year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 1 : 0);
  }

  uint8_t isERRO() {
    if (Time_read == 0) return 2;
    return 0;
  }



  double Time_scale = 1.0;
private:
  unsigned long Time_read = 0;
  unsigned long Time_long = 0;
  unsigned long old_read_unixtime = 0xFFFFFFFF;

  unsigned long Time_read_offset = 0;
  unsigned long Time_long_offset = 0;
};

#endif
