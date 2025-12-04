
#pragma once // chỉ đọc một lần
#define _CMD_

class CMD : public Print // class CMD dùng chung hàm với class Print
{
public:
    char buf[2048];      // biến đệm cmd
    uint8_t new_row = 1; // nếu là đầu dòng
    uint8_t count;       // đánh dấu thứ tự trong 1 giây
    uint32_t oldtime;    // đánh dấu thời gian
    uint8_t dir = 0;     //

    CMD()
    {
        clear();
    }

    void begin() // khởi tạo cmd
    {
        clear();
    }

    void clear()
    {
        for (unsigned int i = 0; i < sizeof(buf); i++) // quét loàn bộ dữ liệu của cmd
        {                                              //
            buf[i] = '-';                              // xóa dữ liệu
        }
    }

    String time_to_string()                                                               // chuyển ngày giờ thành chuỗi
    {                                                                                     //
        String TimeStr;                                                                   //
        if (DayTime.unixtime > 1000)                                                      //
        {                                                                                 //
            char buf[32];                                                                 //
            sprintf(buf, "%02u:%02u:%02u", DayTime.hour, DayTime.minute, DayTime.second); //
            TimeStr = buf;                                                                //
        }
        else                            //
        {                               //
            TimeStr = String(millis()); // Chuyển giá trị millis() thành chuỗi
        } //
        return TimeStr; //
    } //

    virtual size_t write(uint8_t data) // hàm liên kết write
    {
        if (new_row)                       //
        {                                  //
            new_row = 0;                   //
            this->print(time_to_string()); //
            if (dir)                       //
                this->print("<<<");        //
            else                           //
                this->print(">>>");        //
            dir = 0;                       //
        }

        for (unsigned int i = 0; i < sizeof(buf) - 1; i++) // chạy toàn bọ dũ liệu cmd
            buf[i] = buf[i + 1];                           // dịch chuyển dữ liệu lên 1 ô
        buf[sizeof(buf) - 1] = data;                       // ô cuối cùng lưu dữ liệu mới
        if (data == '\n')
            new_row = 1;    //
        Serial.write(data); // xuất lên serial
        return 1;           // báo thành công và tiếp tục
    }
};
CMD cmd; // tạo class CMD với định dạng của class cmd

#define SERIAL cmd

void cmd_available(String data);

void cmd_server_on() // hàm khởi chạy cmd từ server
{

    server.on("/CMD", HTTP_GET, []() {                    // nếu yêu cầu mở cmd
        FLASH_ACTIVE_LED;                                 // bật đèn
        server.sendHeader("Location", "/CMD.html", true); // chuyển hướng đến file CMD.html
        server.send(302, "text/plain", "");               // xác nhận chuyển hướng
    });                                                   //

    server.on("/cmd", HTTP_GET, []() {                    // nếu yêu cầu mở cmd
        FLASH_ACTIVE_LED;                                 // bật đèn
        server.sendHeader("Location", "/CMD.html", true); // chuyển hướng đến file CMD.html
        server.send(302, "text/plain", "");               // xác nhận chuyển hướng
    });                                                   //

    server.on("/_CMD_", HTTP_GET, []() {         // nếu yêu cầu dữ liệu từ cmd
        FLASH_ACTIVE_LED;                        // bật đèn
        server.send(200, "text/plain", cmd.buf); // gửi dữ liệu
    });                                          //

    server.on("/_CMD_", HTTP_PUT, []() {  // nhận dữ liệu
        FLASH_ACTIVE_LED;                 // bật led báo
        String buf = server.arg("plain"); //
        cmd_available(buf);               //
        cmd.dir = 1;                      //
        cmd.println(buf);                 //
    });                                   //
}
