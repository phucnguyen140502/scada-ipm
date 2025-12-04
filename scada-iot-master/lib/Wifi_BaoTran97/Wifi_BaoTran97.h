
#pragma once // chỉ đọc một lần

#ifndef FLASH_ACTIVE_LED
#ifndef LED_BUILTIN                                                       // nếu chân đèn báo chưa được định nghĩa
#define LED_BUILTIN 27                                                    // đèn báo trạng thái
#endif                                                                    //
#ifndef LED_BUILTIN_ON_STATE
#define LED_BUILTIN_ON_STATE 1                                            // mức logic đền báo trạng thái sáng
#endif
#define FLASH_ACTIVE_LED digitalWrite(LED_BUILTIN, LED_BUILTIN_ON_STATE); // thay thế chớp led
#endif

#if defined(ESP8266)          //
#include <ESP8266WebServer.h> // thư viện server
ESP8266WebServer server(80);  // chạy server port 80
#endif                        //

#if defined(ESP32)
#include <WebServer.h> // thư viện server
WebServer server(80);  // chạy server port 80
#endif                 //

#include <Arduino.h> // thư viện hàm arduino


#include <DNSServer.h> // thư viện DNS
DNSServer dnsServer;   //
#define DNS_PORT 53    //

IPAddress apIP(8, 8, 8, 8);         //
IPAddress netMsk(255, 255, 255, 0); //
IPAddress gateway(8, 8, 0, 1);      //
IPAddress subnet(255, 255, 0, 0);   //
IPAddress DNS(8, 8, 8, 8);          //

#include <Udp.h>                                                      // thư viện UDP
#include <WiFiUdp.h>                                                  // thư viện UDP
#include "NTPClient.h"                                                // thư viện cập nhật thời gian thực
WiFiUDP ntpUDP;                                                       // sử dụng giao thức UDP
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7 * 3600, 60000); // khai báo server thời gian thực
RTCDateTime DayTime;                                                  // biến lưu thời gian thực


#include "CMD.h"          // thư viện xuất serial trên web
#include "fileFS.h"       // thư viện xử lý file
#include "firmware.h"     // thư viện xử lý update frimware
#include "wifi_setting.h" // thư viện xử lý kết nối wifi

void handleNotFound()
{                                                              // hàm thực hiện khi trang yêu cầu không tồn tại
    FLASH_ACTIVE_LED;                                          // bật đèn
    String message = "File Not Found\n\n";                     // báo trang yêu cầu không tồn tại
    message += "URI: ";                                        // URI
    message += server.uri();                                   // hiện thị URI
    message += "\nMethod: ";                                   // Method
    message += (server.method() == HTTP_GET) ? "GET" : "POST"; // hiện thị loại Method
    message += "\nArguments: ";                                // Arguments
    message += server.args();                                  // hiện thị Arguments
    message += "\n";                                           // xuống hàng
    for (uint8_t i = 0; i < server.args(); i++)
    {                                                                     // tìm tất cả các arg
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; // hiển thị arg và giá trị
    } //
    server.send(404, "text/plain", message); // trả về kết quả dạng text
} //

void Wifi_und_file_begin()
{
    fileFS_begin(); // khỏi tạo fileFS nếu lỗi trả về 0 nếu hoàn thành trả về 1
    wifi_begin();   // thiết lập wifi

    timeClient.begin(); // thiết lập kết nối thời gian
}

void Wifi_und_file_server_on()
{
    cmd_server_on();             // hàm chạy cmd từ web
    fileFS_server_on();          // hàm chạy fileFS từ web
    firmware_update_server_on(); // hàm chạy firmware update từ web
    wifi_server_on();            // hàm chạy wifi từ web

    server.on("/reset", []() {                  // lệnh reset
        FLASH_ACTIVE_LED;                       // bật đèn
        server.send(200, "text/html", "reset"); // thông báo
        delay(100);                             // đợi một chút
        ESP.restart();                          // reset
    });                                         //

    server.onNotFound(handleNotFound); // không tìn thấy trang yêu cầu
    server.serveStatic("/", FILESYSTEM, "/", "max-age=86400");
}

void Wifi_und_file_loop()
{

    if (WiFi.status() == WL_CONNECTED)
    {
        timeClient.update(); // cập nhật thời gian mới
    }

    DayTime = timeClient.getDateTime(); // lưu thời gian vào biến DayTime
    wifi_loop();


    // static uint32_t t;
    // if (t<millis()){
    //     t=millis()+1000;
    //     //cmd.println(DayTime.unixtime);
    // }
}