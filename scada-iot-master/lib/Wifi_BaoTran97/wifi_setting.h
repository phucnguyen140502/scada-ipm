
#pragma once // chỉ đọc một lần

/*
   21/2/2023
   thư viện kết nối wifi tự động cho esp8266 / esp32
   thư viện sử dụng file dữ liệu trong file FS các file yêu cầu gồm:
   - Config_wifi.html : file giao diện thêm wifi
   - Config_wifi.json : file lưu wifi đã cài đặt
*/

#if defined(ESP8266)     // nếu là ESP8266
#include <ESP8266WiFi.h> // thư viện wifi
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h> // thư viện server
#include <ESP8266WiFiMulti.h> // thư viện kết nối nhiều wifi
#include <LittleFS.h>         // thư viện quản lý file
ESP8266WiFiMulti wifiMulti;   // tạo đối tượng
#endif                        // ESP8266                                                          //

#if defined(ESP32)
#include <WiFi.h>      // thư viện wifi
#include <ESPmDNS.h>   // thư viện DNS
#include <WebServer.h> // thư viện server
#include <WiFiMulti.h> // thư viện kết nối nhiều wifi
#include <SPIFFS.h>    // thư viện quản lý file
WiFiMulti wifiMulti;   // tạo đối tượng
#endif                 // ESP32

#include <FS.h> // thư viện quản lý file
#include <Arduino.h>
#include <ArduinoJson.h> // thư viện chuẩn dữ liệu
#include "fileFS.h"      // thư viện xử lý file
#include <DNSServer.h>

// ============== WIFI CONFIGURATION (Static) ==============
#define WIFI_HOSTNAME       "HomeCTR"
#define WIFI_AP_SSID        "main"
#define WIFI_AP_PASSWORD    "123456789"

// WiFi networks to connect (add more as needed)
struct WiFiCredential {
    const char* ssid;
    const char* password;
};

const WiFiCredential WIFI_CREDENTIALS[] = {
    {"International University 2.4Ghz", ""},
    {"VuHongQuang", "vuhongquang"},
    // Add more networks here: {"ssid", "password"},
};
const int WIFI_CREDENTIALS_COUNT = sizeof(WIFI_CREDENTIALS) / sizeof(WIFI_CREDENTIALS[0]);
// =========================================================




#ifndef SERIAL
#define SERIAL Serial
#endif

boolean isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

String toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++)
    {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

void wifi_begin()
{                                   // thiết lập wifi
    FLASH_ACTIVE_LED;               // bật led báo

    // Initialize WiFi in AP+STA mode (required for MAC address to be available)
    WiFi.mode(WIFI_AP_STA);
    delay(100);  // Allow time for WiFi to initialize
    
    SERIAL.print("MAC Address: ");
    SERIAL.println(WiFi.macAddress());  // Debug: verify MAC is available
    
    // Sử dụng cấu hình WiFi tĩnh
    WiFi.softAPConfig(apIP, apIP, netMsk);
    SERIAL.print("Setting soft-AP with: ssid->" WIFI_AP_SSID " password->" WIFI_AP_PASSWORD " ..."); //
    SERIAL.println(WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD) ? "Ready" : "Failed!");       // khởi chạy softAP và thông báo kết quả lên serial
    SERIAL.print("Soft-AP IP address = ");                                                   //
    SERIAL.println(WiFi.softAPIP());                                                         // thông báo IP của softAP

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError); // nếu không tìm được DNS
    dnsServer.start(DNS_PORT, "*", apIP);               // thiết lập DNS server chuyển hướng tất cả domains về apIP

    WiFi.hostname(WIFI_HOSTNAME); // đặt tên cho thiết bị

    // Thêm các WiFi từ cấu hình tĩnh
    SERIAL.println("num of wifi: " + String(WIFI_CREDENTIALS_COUNT));
    for (int i = 0; i < WIFI_CREDENTIALS_COUNT; i++)
    {
        SERIAL.println("connect wifi: " + String(i + 1) + " ->  " + WIFI_CREDENTIALS[i].ssid);
        wifiMulti.addAP(WIFI_CREDENTIALS[i].ssid, WIFI_CREDENTIALS[i].password);
    }

    SERIAL.println("Connecting Wifi...");                            // hiển thị tình trạng kết nối wifi
    if (wifiMulti.run() == WL_CONNECTED)                             // nếu kết nối wifi thành công
    {                                                                //
        SERIAL.println("WiFi connected: " + String(WiFi.SSID()));    // thông báo tên wifi đã kết nối
        SERIAL.println("IP address: " + toStringIp(WiFi.localIP())); // thông báo ip thiết bị
    }
} //*/

String Wifi_scan_data()
{
    DynamicJsonDocument root(4096); // tạo tệp Json lưu dữ liệu tạm thời

    // Thêm thông tin cấu hình tĩnh
    root["hostname"] = WIFI_HOSTNAME;
    root["ssid_AP"] = WIFI_AP_SSID;
    root["num_wifi"] = WIFI_CREDENTIALS_COUNT;
    for (int i = 0; i < WIFI_CREDENTIALS_COUNT; i++) {
        root["ssid" + String(i + 1)] = WIFI_CREDENTIALS[i].ssid;
    }

    root["client_ID"] = toStringIp(server.client().localIP());
    root["softAPSSID"] = String(WiFi.softAPSSID());
    root["softAPIP"] = toStringIp(WiFi.softAPIP());
    root["SSID"] = String(WiFi.SSID());
    root["localIP"] = toStringIp(WiFi.localIP());

    SERIAL.println("\r\nWifi scan...");   //
    unsigned int n = WiFi.scanNetworks(); // tìm các wifi lân cận

    root["num_wifi_scan"] = n;

    SERIAL.println("scan success"); //
    if (n == 0)
    {                                        // nếu không có wifi lân cận
        SERIAL.println("no networks found"); //
    }
    else
    {                                                  // nếu có wifi lân cận
        SERIAL.println(String(n) + " networks found"); //

        for (int i = 0; i < n; ++i)
        {                                           //
            SERIAL.print(i + 1);                    //
            SERIAL.print(":\t");                    //
            SERIAL.print(WiFi.SSID(i));             //
            SERIAL.print("\t");                     //
            SERIAL.print(WiFi.RSSI(i));             //
            SERIAL.print("\t");                     //
            SERIAL.println(WiFi.encryptionType(i)); // Print SSID and RSSI for each network found

            root["SSID" + String(i)] = String(WiFi.SSID(i));
            root["RSSI" + String(i)] = WiFi.RSSI(i);
            root["encryptionType" + String(i)] = WiFi.encryptionType(i);
        } 
    } 

    String output;               //
    serializeJson(root, output); // chuyển json thành dữ liệu thuần
    return output;
}

String get_wifi_connection_state()
{
    uint8_t currentConnectionState = WiFi.status(); //
    switch (currentConnectionState)                 //
    {
    case WL_CONNECTED:
        return "WIFI CONNECTED";       //
    case WL_NO_SSID_AVAIL:             // Không tìm thấy SSID (mạng WiFi không khả dụng)!
        return "SSID NO AVALIABLE";    //
    case WL_CONNECT_FAILED:            // Kết nối thất bại! Sai mật khẩu hoặc không thể kết nối.
        return "WIFI CONNECT FAILED";  //
    case WL_IDLE_STATUS:               // Đang chờ kết nối...
        return "WIFI WANTING...";      //
    case WL_DISCONNECTED:              // Đã mất kết nối WiFi!
        return "WiFi DISCONNECTED";    //
    case WL_CONNECTION_LOST:           // Kết nối bị mất!
        return "WIFI CONNECTION LOST"; //
    case WL_SCAN_COMPLETED:            // Quá trình quét WiFi đã hoàn thành.
        return "WIFI SCAN COMPLETED";  //
    default:                           // Trạng thái WiFi không xác định.
        return "WIFI UNDEFINE";        //
    }
}

void wifi_loop()
{

    dnsServer.processNextRequest(); //
    server.handleClient();          // chạy web server

    static uint8_t lastConnectionState; // Trạng thái kết nối trước đó

    uint8_t currentConnectionState = WiFi.status();    //
    if (currentConnectionState != lastConnectionState) // Kiểm tra nếu trạng thái kết nối thay đổi
    {                                                  //
        if (currentConnectionState == WL_CONNECTED)
        {
            SERIAL.println("WiFi connected: " + String(WiFi.SSID()));    // thông báo tên wifi đã kết nối
            SERIAL.println("IP address: " + toStringIp(WiFi.localIP())); // thông báo ip thiết bị
        }
        else
        {
            Serial.println(get_wifi_connection_state());
        }
        lastConnectionState = currentConnectionState; // Cập nhật lại trạng thái kết nối
    }

    static unsigned long t;

    if ((WiFi.status() != WL_CONNECTED) && (t < millis()))    // if WiFi is down, try reconnecting
    {
        t = millis() + 30000;
        FLASH_ACTIVE_LED
        SERIAL.print(millis());
        SERIAL.println(" Reconnecting to WiFi...");
        wifiMulti.run(); //

        // WiFi.disconnect();
        // WiFi.reconnect();
    }
    else if ((WiFi.status() == WL_CONNECTED) && (millis() > 30ul * 60ul * 1000ul))
    {
        WiFi.softAPdisconnect(true);
    }
}

void wifi_server_on() // hàm gọi kết nối wifi
{

    server.on("/wifi", HTTP_GET, []() {                          // gọi hàm trả về giao diện thiết lập wifi
        FLASH_ACTIVE_LED;                                        // bật led báo
        server.sendHeader("Location", "Config_wifi.html", true); // chuyển hướng đến trang
        server.send(302, "text/plain", "");                      // xác nhận chuyển hướng
    });                                                          //

    server.on("/_WIFI_SCAN_", HTTP_GET, []() { // gọi hàm trả về giao diện thiết lập wifi
        FLASH_ACTIVE_LED;                      // bật led báo
        String re = Wifi_scan_data();          // lấy dữ liệu giao diện
        server.send(200, "text/plain", re);    // gửi đi
    });

    server.on("/wifisave", HTTP_PUT, []() { //
        FLASH_ACTIVE_LED;                   // bật led báo

        DynamicJsonDocument data(4096);                                          // đệm Json
        DeserializationError error = deserializeJson(data, server.arg("plain")); // chuyển dữ liệu nhận được về dạng Json
        if (error)
        {                                                                                  // nếu lỗi
            server.send(500, "text/plain", "FAIL to convert json. " + server.arg("plain")); // báo lỗi
        }
        else
        { // nếu không lỗi
            String ssid = data["ssid"].as<String>();         // đọc tên wifi
            String password = data["password"].as<String>(); // đọc mật khẩu
            SERIAL.println("ssid: " + ssid);                 // xuất lên serial
            SERIAL.println("password: " + password);         // xuất lên serial

            wifiMulti.addAP(ssid.c_str(), password.c_str()); // thêm wifi vào danh sách kết nối (chỉ lưu trong RAM)
            wifiMulti.run();                                 //

            // Trả về thông báo thành công (WiFi chỉ được thêm tạm thời, sẽ mất khi khởi động lại)
            String output = "{\"status\":\"ok\",\"message\":\"WiFi added (runtime only, not persistent)\",\"ssid\":\"" + ssid + "\"}";
            server.send(200, "text/plain", output);
        }
    });
}