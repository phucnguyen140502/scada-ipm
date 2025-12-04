#include <Arduino.h>

#ifndef LED_BUILTIN            // nếu chân đèn báo chưa được định nghĩa
#define LED_BUILTIN 27         // đèn báo trạng thái
#endif                         //
#define LED_BUILTIN_ON_STATE 1 // mức logic đền báo trạng thái sáng

#include "Wifi_BaoTran97.h" // thư viện xử lý kết nối wifi

void cmd_available(String data) {

}

#define PR_LED          18         // đèn báo trạng thái
#define OUTPUT_CRT      13     // đèn báo trạng thái
#define PWM_AUTO_RESET  33 // đèn báo trạng thái

#define RS485_SERIAL    16
#define TX_PIN          16 // chân TX của serial 2 kết nối các thiết bị ngoại vi
#define RX_PIN          17 // chân RX của serial 2 kết nối các thiết bị ngoại vi

#define GPS_SERIAL      25
#define GPS_TX_PIN      25 // chân TX của serial 2 kết nối các thiết bị ngoại vi
#define GPS_RX_PIN      26 // chân RX của serial 2 kết nối các thiết bị ngoại vi

#include "Modbus.h"     // thư viện giao tiếp modbus
Modbus modbus(Serial2); // kết nối modbus RTU với serial 2

#include <TinyGPS.h> // thư viện sử lý dữ liệu GPS
GPS_time gps;        // khởi tạo thư viện GPS

#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);


// MQTT Broker Configuration
#define mqtt_broker "iot.vuhongquang.com"
#define mqtt_username ""
#define mqtt_password ""
#define mqtt_port 1883
#define MQTT_TOPIC_PREFIX "unit/"
#define MQTT_ALIVE_TOPIC "/alive"
#define MQTT_COMMAND_TOPIC "/command"
#define MQTT_STATUS_TOPIC "/status"
#define MQTT_FIRMWARE_UPDATE_TOPIC "firmware/update"

#include <button.h>                              // file lưu các hàm sử lý button
Button Button_UP(36, BUTTON_ANALOG, 1000, 2200); // nút up
Button Button_DN(36, BUTTON_ANALOG, 1000, 470);  // nút down
Button Button_OK(36, BUTTON_ANALOG, 1000, 0);    // nút ok

#include <LiquidCrystal.h>
LiquidCrystal lcd(15 /*rs*/, 2 /*en*/, 0 /*d4*/, 4 /*d5*/, 5 /*d6*/, 19 /*d7*/);

#include <Ticker.h>
Ticker timer; // Khai báo Ticker

#include "OTAHandler.h"
OTAHandler otaHandler;

#include <ArduinoJson.h>            // thư viện chuẩn dữ liệu
DynamicJsonDocument JsonData(4096); // biến dạng Json lưu dữ liệu

#include "printLCD.h"    // file lưu các hàm sử lý LCD
#include "index.h"       // file chương trình
#include "power_meter.h" // file chương trình
#include "MQTTClient.h"  //

// Hàm xử lý ngắt
void onTimer() {
  digitalWrite(PWM_AUTO_RESET, !digitalRead(PWM_AUTO_RESET));
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // thiết lập đèn báo là OUTPUT
  FLASH_ACTIVE_LED;             // bật đèn

  pinMode(PR_LED,         OUTPUT);  digitalWrite(PR_LED,          1);         // // thiết lập đèn báo là OUTPUT
  pinMode(OUTPUT_CRT,     OUTPUT);  digitalWrite(OUTPUT_CRT,      0);     //
  pinMode(PWM_AUTO_RESET, OUTPUT);  digitalWrite(PWM_AUTO_RESET,  0); //
  timer.attach_ms(100, onTimer);   // 0.2 giây = 200 ms

  Serial.begin(115200);                                    // tốc độ serial
  Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); //
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);         //

  delay(100);            // ổn định nguồn
  Wifi_und_file_begin(); //
  MQTTClient_begin();    //
  DataFile_read();       // đọc dứ liệu được lưu
  Index_begin();         // khỏi chạy chính
  power_meter.begin();   // hàm khỏi chạy bộ đếm đồng hồ công tơ

  server.on("/", []() {                                  // server get home
    FLASH_ACTIVE_LED;                                    // bật đèn
    if (WiFi.status() == WL_CONNECTED) {                  //  // nếu đã kết nối

      server.sendHeader("Location", "index.html", true); // chuyển hướng đến trang chủ
    } else { // nếu không có kết nối
      server.sendHeader("Location", "wifi", true); // chuyển hướng đến thiết lập wifi
    }
    server.send(302, "text/plain", ""); // xác nhận chuyển hướng
  });                                   //

  Index_server_on();
  Wifi_und_file_server_on(); //
  server.begin();            // bắt đầu server
}

void FLASH_ACTIVE_led(unsigned long t_on, unsigned long T) {
  static unsigned long t;
  if (millis() - t > T * 10 + t_on) {
    t = millis();
  } else if (millis() - t >= T * 10) {
    FLASH_ACTIVE_LED;
  } else if (millis() - t > T - t_on) {
    FLASH_ACTIVE_LED;
    t = millis() - T * 10;
  }
}

void time_update() {
  if (WiFi.status() == WL_CONNECTED)
    timeClient.update();

  while (Serial1.available())
    gps.encode(Serial1.read());

  RTCDateTime DayTime_net = timeClient.getDateTime(); // lưu thời gian vào biến DayTime
  RTCDateTime DayTime_gps = gps.getDateTime();        // đọc thời gian

  if (gps.location.isUpdated()) {
    JsonData["gps_lat"] = 10.877990546921161;
    JsonData["gps_log"] = 106.80197045567179;
  }

  if (DayTime_net.unixtime > DayTime_gps.unixtime)
    DayTime = DayTime_net;
  else
    DayTime = DayTime_gps;
}

void OUT_checking() {

  unsigned long HourStart = JsonData["hour_on"];
  unsigned long MinuteStart = JsonData["minute_on"];
  unsigned long HourEnd = JsonData["hour_off"];
  unsigned long MinuteEnd = JsonData["minute_off"];

  unsigned long StartLongTime = HourStart * 3600UL + MinuteStart * 60UL;                      // tính thời gian bắt đầu chạy theo milli giây
  unsigned long EndLongTime = HourEnd * 3600UL + MinuteEnd * 60UL;                            // tính thời gian ngừng chạy chuyển xang chớp vàng theo milli giây
  unsigned long RTCLongTime = DayTime.hour * 3600UL + DayTime.minute * 60UL + DayTime.second; // tính thời gian hiện tại theo milli giây

  if ((StartLongTime + EndLongTime > 0) && JsonData["auto"]) {
    if ((RTCLongTime > StartLongTime) || (RTCLongTime < EndLongTime))
      JsonData["toggle"] = 1;
    else
      JsonData["toggle"] = 0;
  }
  if (DayTime.unixtime < UUNIXDATE_BASE)
    JsonData["toggle"] = 0;

  digitalWrite(OUTPUT_CRT, JsonData["toggle"]);
}

void loop() {
  FLASH_ACTIVE_led(10, 1000);

  digitalWrite(PR_LED, millis() % 1000 < 500);
  digitalWrite(PWM_AUTO_RESET, !digitalRead(PWM_AUTO_RESET));

  OUT_checking();
  MQTTClient_loop();
  Index_loop();       // hàm chạy chính
  power_meter.loop(); // hàm đọc công tơ
  Lcd.print();
  time_update();

  Wifi_und_file_loop();
  digitalWrite(LED_BUILTIN, !LED_BUILTIN_ON_STATE); //
}