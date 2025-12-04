//

#include <Arduino.h> // thư viện hàm arduino
#include <WiFi.h>    // thư viện wifi

#include <Ticker.h>
Ticker timer_lcd; // Khai báo Ticker
void onTimer_lcd();

void Timer_lcd_run()
{
  if (!timer_lcd.active())
    timer_lcd.attach_ms(100, onTimer_lcd);
}

void Timer_lcd_stop()
{
  if (timer_lcd.active())
    timer_lcd.detach();
}

class LCD
{
public:
  uint8_t display_index;        // biến lưu vị trí màn hình được chọn
  uint8_t Cursor_line;          // biến lưu dòng trên màn hình được chọn
  uint8_t display_set;          // biến thể hiện chế dộ màn hình
  uint8_t Cursor_index;         // biến lưu vị trí con trỏ
  uint32_t auto_reset_lcd_time; //
  volatile uint8_t LCD_busy;             // biến báo có được phép cập nhật màn hình không

  LCD()
  {                                   // hàm khởi động LCD
    begin();                          // khởi động LCD loại 1602
    lcd.setCursor(0, 0);              // đi chuyển con trở
    lcd.print("Program starting..."); // thông báo khởi động
  }

  void begin()        // hàm khởi động LCD
  {                   //
    lcd.begin(20, 4); // khởi động LCD loại 1602
  }

  String Full_line(String data, uint8_t margin, uint8_t col_num = 20) // Kiểm tra độ dài của SSID và thêm dấu cách nếu cần
  {
    uint8_t dir = 0;
    while (data.length() < col_num)
    {
      if (margin == 1)
        dir = 0;
      else if (margin == 3)
        dir = 1;

      if (!dir)
        data = data + " ";
      else
        data = " " + data;

      dir = !dir;
    }
    return data;
  }

  void Write_full_line(uint8_t line, String data)
  {
    Write_full_line(line, 1, data);
  }
  void Write_full_line_center(uint8_t line, String data)
  {
    Write_full_line(line, 2, data);
  }

  void Write_full_line(uint8_t line, uint8_t margin, String data)
  {
    lcd.setCursor(0, line);             // đặt con trỏ
    lcd.print(Full_line(data, margin)); //
  }

  unsigned long button_old_pluse_time()
  {                                                       // hàm tính thời gian kể từ lần cuối cùng nhấn nút đơn vị millis
    unsigned long re = millis() - Button_OK.time_refresh; // lấy thời gian làm mới của nút ok
    if (millis() - Button_UP.time_refresh < re)           //
      re = millis() - Button_UP.time_refresh;             // nếu nhấn nút up sau thì lấy thời gian làm mới của nút up
    if (millis() - Button_DN.time_refresh < re)           //
      re = millis() - Button_DN.time_refresh;             // nếu nhấn nút down sau thì lấy thời gian làm mới của nút down
    return re;                                            // trả về thời gian sau cùng mà nút được nhấn
  }

  void auto_back_home() // hàm tự động quay về màm hình chính khi các nút nhấn không được nhấn trong 5 phút
  {                     //

    if (auto_reset_lcd_time < millis())       // reset lcd mỗi 5s
    {                                         //
      auto_reset_lcd_time = millis() + 5000; //
      begin();                                // nếu quá lâu thì khởi động lại lcd
    }

    if (button_old_pluse_time() > 300000) // nếu không có nút nào được nhần trong 5 phút
    {                                     //
      Timer_lcd_stop();                   // khi không có tương tác nút nhấn thì lcd không cần cập nhật nhiều
      if (display_index != 0)             //
      {                                   //
        display_index = 0;                // về màn hình chính
        Cursor_line = 0;                  // về dòng đầu
        display_set = 0;                  // thoát cài đặt
      }
    }
    else
    {
      Timer_lcd_run();
    }
  }

  void LCD_display_print_index() // hàm hiển thị chính
  {                              //
    char s[32];                  // biến đệm sử lý chuỗi
    if (display_index == 0)      // nếu ở màn hình chính
    {                            //

      if (Button_OK.IsFalling()) //
      {                          // nhấn bất kì nút nào
        if (JsonData["auto"])
        {
          JsonData["auto"] = 0;
          JsonData["toggle"] = 1;
        }
        else if (JsonData["toggle"])
        {
          JsonData["auto"] = 0;
          JsonData["toggle"] = 0;
        }
        else
        {
          JsonData["auto"] = 1;
        }
      }

      if ((Button_UP.IsFalling()) || (Button_DN.IsFalling())) //
      {                                                       // nhấn bất kì nút nào
        display_index = 1;                                    // vào trang chọn giá trị cài đặt
        Cursor_line = 0;                                      // con trỏ đặt ở hàng đầu
        lcd.clear();
      }

      sprintf(s, "%02u:%02u:%02u", DayTime.hour, DayTime.minute, DayTime.second); // hiển thị ngày giờ
      Write_full_line_center(0, s);                                               //

      sprintf(s, "%02u/%02u/%04u", DayTime.day, DayTime.month, DayTime.year); // hiển thị ngày giờ
      Write_full_line_center(1, s);                                           //

      Write_full_line_center(2, " "); //

      if (JsonData["auto"])
        Write_full_line(3, " on    >auto<   off ");
      else if (JsonData["toggle"])
        Write_full_line(3, ">on<    auto    off ");
      else
        Write_full_line(3, " on     auto   >off<");

      // lcd.setCursor(0, 3);                                      // đặt con trỏ
      // if (WiFi.status() == WL_CONNECTED)                        //
      //   Write_full_line_center(3, WiFi.SSID());                 //
      // else                                                      //
      //   Write_full_line_center(3, get_wifi_connection_state()); //
    }
    else
    {
      if (Button_UP.IsFalling()) // nếu nhấn nút lên
      {                          //
        if (Cursor_line == 0)    // đang ở dòng trên cùng
          display_index = 0;     // thì di về home
        else                     // đang ở dòng khác
          Cursor_line--;         //  di chuyển lên dòng trên
      }
      if (Button_DN.IsFalling()) // nếu nhấn nút xuống
      {                          //
        if (Cursor_line < 2)     // đang ở dòng khác
          Cursor_line++;         // di chuyển xuống dòng dưới
      }
      if (Button_OK.IsFalling()) // nếu nhấn nút chọn
      {                          //
        display_set = 1;         // chuyển qua chế độ set
        lcd.clear();             // xóa màn hình
      }

      int HourStart = JsonData["hour_on"];
      int MinuteStart = JsonData["minute_on"];

      int HourEnd = JsonData["hour_off"];
      int MinuteEnd = JsonData["minute_off"];

      lcd.setCursor(1, 0); // đặt con trỏ
      sprintf(s, " Time On:   %02u:%02u ", HourStart, MinuteStart);
      lcd.print(s);        // xuất ra màn hình
      lcd.setCursor(1, 1); // đặt con trỏ
      sprintf(s, " Time Off:  %02u:%02u ", HourEnd, MinuteEnd);
      lcd.print(s);
      lcd.setCursor(1, 2);
      lcd.print(" WIFI information ");

      for (int i = 0; i < 3; i++)
      {
        if (Cursor_line == i)
        {
          lcd.setCursor(0, i);
          lcd.print(">");
          lcd.setCursor(19, i);
          lcd.print("<");
        }
        else
        {
          lcd.setCursor(0, i);
          lcd.print(" ");
          lcd.setCursor(19, i);
          lcd.print(" ");
        }
      }

      Write_full_line_center(3, " "); //
    }
  }

  void SetTimeOn()
  {
    int HourStart = JsonData["hour_on"];
    int MinuteStart = JsonData["minute_on"];

    if (Button_OK.IsFalling())
    {
      if (Cursor_index < 1)
      {
        Cursor_index++;
      }
      else
      {
        Cursor_index = 0;
        display_set = 0;
      }
    }

    if (Button_DN.IsFallingContinuous())
    {
      switch (Cursor_index)
      {
      case 0:
        HourStart = (HourStart + 23) % 24;
        break;
      case 1:
        MinuteStart = (MinuteStart + 59) % 60;
        break;
      }
    }

    if (Button_UP.IsFallingContinuous())
    {
      switch (Cursor_index)
      {
      case 0:
        HourStart = (HourStart + 1) % 24;
        break;
      case 1:
        MinuteStart = (MinuteStart + 1) % 60;
        break;
      }
    }

    char s[32];                                                     // biến đệm sử lý chuỗi
    if ((millis() % 1000 < 300) && (button_old_pluse_time() > 500)) //
    {
      switch (Cursor_index)
      {
      case 0:
        sprintf(s, "__:%02u", MinuteStart);
        break;
      case 1:
        sprintf(s, "%02u:__", HourStart);
        break;
      } //
    }
    else
    {
      sprintf(s, "%02u:%02u", HourStart, MinuteStart);
    }

    Write_full_line_center(0, "Set up time on"); //
    Write_full_line_center(1, s);                //
    Write_full_line_center(2, " ");              //
    Write_full_line_center(3, " ");              //

    JsonData["hour_on"] = HourStart;
    JsonData["minute_on"] = MinuteStart;
  }

  void SetTimeOff()
  {
    int HourEnd = JsonData["hour_off"];
    int MinuteEnd = JsonData["minute_off"];

    if (Button_OK.IsFalling())
    {
      if (Cursor_index < 1)
      {
        Cursor_index++;
      }
      else
      {
        Cursor_index = 0;
        display_set = 0;
      }
    }

    if (Button_DN.IsFallingContinuous())
    {
      switch (Cursor_index)
      {
      case 0:
        HourEnd = (HourEnd + 23) % 24;
        break;
      case 1:
        MinuteEnd = (MinuteEnd + 59) % 60;
        break;
      }
    }

    if (Button_UP.IsFallingContinuous())
    {
      switch (Cursor_index)
      {
      case 0:
        HourEnd = (HourEnd + 1) % 24;
        break;
      case 1:
        MinuteEnd = (MinuteEnd + 1) % 60;
        break;
      }
    }

    char s[32]; // biến đệm sử lý chuỗi
    if ((millis() % 1000 < 300) && (button_old_pluse_time() > 500))
    {
      switch (Cursor_index)
      {
      case 0:
        sprintf(s, "__:%02u", MinuteEnd);
        break;
      case 1:
        sprintf(s, "%02u:__", HourEnd);
        break;
      }
    }
    else
    {
      sprintf(s, "%02u:%02u", HourEnd, MinuteEnd);
    }

    Write_full_line_center(0, "Set up time off"); //
    Write_full_line_center(1, s);                 //
    Write_full_line_center(2, " ");               //
    Write_full_line_center(3, " ");               //

    JsonData["hour_off"] = HourEnd;
    JsonData["minute_off"] = MinuteEnd;
  }

  void ShowWifiInfomation()
  {
    if (Button_OK.IsFalling() || Button_DN.IsFalling() || Button_UP.IsFalling())
    {
      display_set = 0; //
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Write_full_line(0, WiFi.SSID());                //
      Write_full_line(1, toStringIp(WiFi.localIP())); //
    }
    else
    {
      Write_full_line(0, get_wifi_connection_state()); //
      Write_full_line(1, " ");                         //
    }

    Write_full_line(2, " "); //
    Write_full_line(3, " "); //
  }

  void LCD_display_setup() // hàm chọn trang thiết lập
  {
    switch (Cursor_line)
    {
    case 0:
      SetTimeOn();
      break;

    case 1:
      SetTimeOff();
      break;

    case 2:
      ShowWifiInfomation();
      break;
    }
  }

  void print_message(String message, uint8_t row, uint8_t clear = 0)
  {
    Timer_lcd_stop();
    if (clear)
      lcd.clear();
    Write_full_line(row, message); //
  }

  void print() // hàm điều hướng xuất LCD
  {
    if (LCD_busy)     // nếu có nhiều lệnh gọi cùng lúc
      return;         // không thực hiện việc cập nhật màn hình
    LCD_busy++;       // đánh dấu đang bắt đầu cập nhật màn hình

    auto_back_home(); // kiểm tra về màn hình chính tự động

    if (!display_set)
    {                            // nếu không ở chế dộ thiết lập
      LCD_display_print_index(); // xuất màn hình theo vị trí con trở
    }
    else
    {                      // nếu ở chế độ thiết lập
      LCD_display_setup(); // chọn trang thiết lập
    }
    LCD_busy = 0; // cho phép tiếp tục cập nhật ở lần tiếp theo
  }
};
LCD Lcd;

void onTimer_lcd() // Hàm xử lý ngắt
{
  Lcd.print();
}