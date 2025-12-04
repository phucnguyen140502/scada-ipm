/*
   21/2/2023
   thư viện cập nhật firmware trên web server cho esp8266 / esp32
*/

#ifndef Firmware_H // nếu chưa được biên dịch
#define Firmware_H // đánh dấu đã biên dịch

#if defined(ESP8266)                 // nếu là ESP8266
#include <ESP8266WiFi.h>             // thư viện wifi
#include <ESP8266WebServer.h>        // thư viện server
#include <ESP8266HTTPUpdateServer.h> // thư viện cập nhật code online
#endif                               // ESP8266                                                          //

#if defined(ESP32)
#include <WiFi.h>      // thư viện wifi
#include <WebServer.h> // thư viện server
#include <Update.h>    // thư viện cập nhật code online
#endif                 // ESP32

#include <WiFiClient.h> //








void firmware_update_server_on()
{                                                           //
    server.on("/log_in", HTTP_GET, []() {                   //
        FLASH_ACTIVE_LED;                                   // bật led báo
        server.sendHeader("Location", "log_in.html", true); // chuyển hướng đến trang
        server.send(302, "text/plain", "");                 // xác nhận chuyển hướng
    });                                                     //

    server.on("/firmware", HTTP_GET, []() {                          //
        FLASH_ACTIVE_LED;                                            // bật led báo
        server.sendHeader("Location", "firmware_update.html", true); // chuyển hướng đến trang
        server.send(302, "text/plain", "");                          // xác nhận chuyển hướng
    });                                                              //

    server.on("/update_firmware", HTTP_POST, []() {                          // handling uploading firmware file
        FLASH_ACTIVE_LED;                                                    // bật led báo
        server.sendHeader("Connection", "close");                            //
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK"); //
        delay(100);                                                          //
        ESP.restart();                                                       //
    },
              []() {                //
                  FLASH_ACTIVE_LED; // bật led báo
                  HTTPUpload &upload = server.upload();
                  if (upload.status == UPLOAD_FILE_START)
                  {
                      Serial.printf("Update: %s\n", upload.filename.c_str());

#if defined(ESP8266)
                      Serial.setDebugOutput(true);
                      WiFiUDP::stopAll();
                      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                      if (!Update.begin(maxSketchSpace))
                      { // start with max available size
                          Update.printError(Serial);
                      }
#endif // ESP8266

#if defined(ESP32)
                      if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                      { // start with max available size UPDATE_SIZE_UNKNOWN=0xFFFFFFFF
                          Update.printError(Serial);
                      }
#endif // ESP32
                  }
                  else if (upload.status == UPLOAD_FILE_WRITE)
                  {
                      /* flashing firmware to ESP*/
                      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                      {
                          Update.printError(Serial);
                      }
                  }
                  else if (upload.status == UPLOAD_FILE_END)
                  {
                      if (Update.end(true))
                      { // true to set the size to the current progress
                          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                      }
                      else
                      {
                          Update.printError(Serial);
                      }
                  }
#if defined(ESP8266)
                  yield();
#endif
              });
}

#endif
