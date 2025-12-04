#include <ArduinoJson.h> // thư viện chuẩn dữ liệu

unsigned long time_save = 1ul * 60ul * 1000ul;

void InitializeDefaults()
{
  // Set default GPS coordinates if null
  if (JsonData["gps_lat"].isNull()) JsonData["gps_lat"] = 10.877990546921161;
  if (JsonData["gps_log"].isNull()) JsonData["gps_log"] = 106.80197045567179;
  
  // Set default schedule times if null
  if (JsonData["hour_on"].isNull())   JsonData["hour_on"] = 0;
  if (JsonData["minute_on"].isNull()) JsonData["minute_on"] = 0;
  if (JsonData["hour_off"].isNull())  JsonData["hour_off"] = 0;
  if (JsonData["minute_off"].isNull()) JsonData["minute_off"] = 0;
  
  // Set default toggle/auto states if null
  if (JsonData["auto"].isNull())   JsonData["auto"] = 0;
  if (JsonData["toggle"].isNull()) JsonData["toggle"] = 0;
  
  // Set default energy values if null
  if (JsonData["total_energy"].isNull()) JsonData["total_energy"] = 0;
  if (JsonData["voltage"].isNull())      JsonData["voltage"] = 0;
  if (JsonData["current"].isNull())      JsonData["current"] = 0;
  if (JsonData["power"].isNull())        JsonData["power"] = 0;
  if (JsonData["power_factor"].isNull()) JsonData["power_factor"] = 0;
  if (JsonData["frequency"].isNull())    JsonData["frequency"] = 0;
}

void DataFile_read()
{                                                                   // đọc file data
  File file = FILESYSTEM.open("/data.json", "r");                   // mở tệp ở chế độ đọc
  String DataFile = file.readString();                              // đọc file
  cmd.println(DataFile);                                            // hiển thị lên Serial
  DeserializationError error = deserializeJson(JsonData, DataFile); // chuyển dữ liệu về dạng Json
  if (error)                                                        //
    SERIAL.println("erro converter data to json");                  // hiển thị lên Serial
  file.close();                                                     // đóng file
  InitializeDefaults();                                             // fill null values with defaults
} //

void DataFile_write()
{                                                 // chèn thêm enter vào tài liệu
  String output;                                  //
  serializeJson(JsonData, output);                // chuyển json thành dữ liệu thuần
  output = format_Json(output);                   //
  File file = FILESYSTEM.open("/data.json", "w"); // mở tệp ở chế độ ghi
  file.print(output);                             //
  file.close();                                   // đóng tệp
}

void Index_begin()
{
  InitializeDefaults();
}

void Index_loop()
{
  if (DayTime.year > 2020)
  {
    JsonData[String("power_D") + String(DayTime.day)] = JsonData["total_energy"];
    JsonData[String("power_M") + String(DayTime.month)] = JsonData["total_energy"];
  }

  if (time_save < millis())
  {
    DataFile_write();
    time_save = millis() + 8ul * 3600ul * 60ul * 1000ul;
  }
}

void server_send_json_data()
{
  String output;                          //
  serializeJson(JsonData, output);        // chuyển json thành dữ liệu thuần
  server.send(200, "text/plain", output); // gửi đi
}

void Index_server_on() // server on
{
  server.on("/DataFileRead", HTTP_GET, []() { // lấy dữ liệu
    FLASH_ACTIVE_LED;                         // bật led báo
    DataFile_read();                          //
    server_send_json_data();                  // trả về json data
  });                                         //

  server.on("/DataFileWrite", HTTP_GET, []() { // lấy dữ liệu
    FLASH_ACTIVE_LED;                          // bật led báo
    DataFile_write();                          //
    server_send_json_data();                   // trả về json data
  });                                          //

  server.on("/state", HTTP_GET, []() { // lấy dữ liệu
    FLASH_ACTIVE_LED;                  // bật led báo
    server_send_json_data();           // trả về json data
  });                                  //

  server.on("/state", HTTP_PUT, []() {                                       // nhận dữ liệu
    FLASH_ACTIVE_LED;                                                        // bật led báo
    DynamicJsonDocument root(4096);                                          // đệm Json
    DeserializationError error = deserializeJson(root, server.arg("plain")); // chuyển dữ liệu nhận được về dạng Json
    if (error)
    {                                                                                // nếu lỗi
      server.send(500, "text/plain", "FAIL to conver json. " + server.arg("plain")); // báo lỗi
    }
    else
    { // nếu không lỗi

      JsonData = root;
      time_save = millis() + 1ul * 60ul * 1000ul;
      server_send_json_data(); // trả về json data
    } //
  }); //
} //
