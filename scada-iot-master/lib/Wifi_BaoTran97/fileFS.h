
#pragma once // chỉ đọc một lần

#if defined(ESP8266)          // nếu là ESP8266
#include <ESP8266WebServer.h> // thư viện server
#endif                        // ESP8266                                                          //

#if defined(ESP32)     // nếu là ESP32
#include <WebServer.h> // thư viện server
#endif                 // ESP32

#include <FS.h>                  // thư viện quản lý file
#include <ArduinoJson.h>         // thư viện chuẩn dữ liệu
// #include <LittleFS.h>            // thư viện quản lý file
// #define FILESYSTEM LittleFS      // định nghĩa thay thế
// #define FILESYSTEMSTR "LittleFS" // định nghĩa thay thế dạng chuỗi




#include <SPIFFS.h>                                                         // thư viện quản lý file
#define  FILESYSTEM      SPIFFS                                             // định nghĩa thay thế
#define  FILESYSTEMSTR   "SPIFFS"                                           // định nghĩa thay thế dạng chuỗi




#ifndef SERIAL
#define SERIAL Serial
#endif

String getContentType(String filename)   // chuyển đổi phần mở rộng tệp thành loại MIME
{                                        //
    if (filename.endsWith(".html"))      //
        return "text/html";              // dạng HTML
    else if (filename.endsWith(".htm"))  //
        return "text/html";              // dạng HTML
    else if (filename.endsWith(".css"))  //
        return "text/css";               // dạng CSS
    else if (filename.endsWith(".xml"))  //
        return "text/xml";               // dạng text
    else if (filename.endsWith(".png"))  //
        return "image/png";              // dạng ảnh png
    else if (filename.endsWith(".gif"))  //
        return "image/gif";              // dạng ảnh động gif
    else if (filename.endsWith(".jpg"))  //
        return "image/jpeg";             // dạng ảnh
    else if (filename.endsWith(".ico"))  //
        return "image/x-icon";           // dạng biểu tượng
    else if (filename.endsWith(".js"))   //
        return "application/javascript"; // dạng js
    else if (filename.endsWith(".pdf"))  //
        return "application/pdf";        // dạng pdf
    else if (filename.endsWith(".zip"))  //
        return "application/zip";        // dạng nén
    else if (filename.endsWith(".gz"))   //
        return "application/x-gzip";     // dạng nén
    return "text/plain";                 // mặc định dạng text
}

uint8_t fileFS_begin()                              // khỏi tạo fileFS nếu lỗi trả về 0 nếu hoàn thành trả về 1
{                                                   //
    SERIAL.println("mounting " FILESYSTEMSTR "..."); // thông báo khởi động hệ thống file
    if (!FILESYSTEM.begin(true))                    //
    {                                               // nếu hệ thống file khởi động không thành công
        SERIAL.println("Failed to mount file system"); // thông báo không thể khởi động hệ thống file
        ESP.restart();                              // khỏi động lại ESP
        return 0;                                   // thoát
    }
    return 1; // thoát
}

// void format_Json_document(String filename)    // chèn thêm enter vào tài liệu
// {                                             //
//     File file;                                // tạo đối tượng file
//     if (!filename.startsWith("/"))            //
//         filename = "/" + filename;            // nếu tên file không bắt đầu bằng dấu "/" thì thêm dấu "/" vào tên file
//     file = FILESYSTEM.open(filename, "r");    // mở tệp ở chế độ đọc
//     String DataFile = file.readString();      // đọc file
//     file.close();                             // đóng file
//     DataFile.replace("{", "{\r\n");           // thêm dấu enter sau dấu {
//     DataFile.replace("}", "\r\n}");           // thêm dấu enter trước dấu }
//     DataFile.replace("},\"", "},\r\n\r\n\""); // thêm 2 dấu enter giữa 2 nhóm dữ liệu
//     DataFile.replace(",\"", ",\r\n\"");       // thêm dấu enter sau mỗi dữ liệu
//     file = FILESYSTEM.open(filename, "w");    // mở tệp ở chế độ ghi
//     file.print(DataFile);                     // ghi dữ liệu
//     file.close();                             // đóng tệp
// }

String format_Json(String data){
    data.replace("{", "{\r\n");               // thêm dấu enter sau dấu {
    data.replace("}", "\r\n}");               // thêm dấu enter trước dấu }
    data.replace("},\"", "},\r\n\r\n\"");     // thêm 2 dấu enter giữa 2 nhóm dữ liệu
    data.replace(",\"", ",\r\n\"");           // thêm dấu enter sau mỗi dữ liệu
    return data;
}


// void DataFile_read()                                                  // đọc file data
// {                                                                     //
//     File file = FILESYSTEM.open("/data.json", "r");                   // mở tệp ở chế độ đọc
//     String DataFile = file.readString();                              // đọc file
//     SERIAL.println(DataFile);                                         // hiển thị lên Serial
//     DeserializationError error = deserializeJson(JsonData, DataFile); // chuyển dữ liệu về dạng Json
//     if (error)                                                        //
//         cmd.println("erro converter data to json");                   // hiển thị lên Serial
//     file.close();                                                     // đóng file
// }

// void DataFile_write()                               // chèn thêm enter vào tài liệu
// {                                                   //
//     File file = FILESYSTEM.open("/data.json", "w"); // mở tệp ở chế độ ghi
//     serializeJson(JsonData, file);                  // ghi dữ liệu
//     file.close();                                   // đóng tệp
//     format_Json_document("/data.json");             // định dạng lại file
// }

File fsUploadFile;                                       // tạo hệ thống tệp để lưu file
void handleFileUpload()                                  // hàm upload file vào FILESYSTEM
{                                                        //
    HTTPUpload &upload = server.upload();                // thư viện upload
    if (upload.status == UPLOAD_FILE_START)              // nếu là bắt đầu upload
    {                                                    //
        String filename = upload.filename;               // đọc tên file
        if (!filename.startsWith("/"))                   //
            filename = "/" + filename;                   // nếu tên file không bắt đầu bằng dấu "/" thì thêm dấu "/" vào tên file
        SERIAL.print("handleFileUpload Name: " + filename); // xuất lên serial
        fsUploadFile = FILESYSTEM.open(filename, "w");   // mở file ở chế độ ghi
        filename = String();                             // xóa tên file
    }

    else if (upload.status == UPLOAD_FILE_WRITE)                // nếu ở giai đoạn ghi giữ liệu
    {                                                           //
        if (fsUploadFile)                                       //
            fsUploadFile.write(upload.buf, upload.currentSize); // ghi dữ liệu nhận được vào file
    }

    else if (upload.status == UPLOAD_FILE_END)      // nếu kết thúc quá trình ghi
    {                                               //
        if (fsUploadFile)                           //
        {                                           // nếu hoàn tất quá trình ghi file
            fsUploadFile.close();                   // đóng file
            server.sendHeader("Location", "/file"); // chuyển hướng
            server.send(303);                       // xác nhận lệnh chuyển hướng
        }
        else                                                             //
        {                                                                // nếu quá trình ghi bị lỗi
            server.send(500, "text/plain", "500: couldn't create file"); // báo không thể hoàn thành ghi dữ liệu
        } //
    } //
} //

void fileFS_server_on()                  // server on
{                                        //
    server.on("/file", HTTP_POST, []() { // nếu máy khách đăng upload file
        FLASH_ACTIVE_LED;                // bật đèn
        server.send(200);                // gửi status 200 (OK) tới máy khách báo đã sẵn sàng nhận file
    },                                   //
              handleFileUpload);         // nhận và lưu file

    server.on("/file", HTTP_GET, []() {                                                   // trang quản lý file
        FLASH_ACTIVE_LED;                                                                 // bật đèn
        String html;                                                                      //
        html += "<!DOCTYPE html>";                                                        //
        html += "<html>";                                                                 //
        html += "<head>";                                                                 //
        html += "<meta charset='utf-8'>";                                                 //
        html += "<meta http-equiv='X-UA-Compatible' content='IE=edge'>";                  //
        html += "<title>File Manager</title>";                                            //
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; //
        html += "</head>";                                                                //
        html += "<body>";                                                                 //

        html += "<style>";                                                                                           // định dạng table
        html += "table {border-collapse:collapse;border-spacing:0;width:100%;display:table;border:1px solid #ccc;}"; //
        html += "tr:nth-child(odd)  {background-color:#fff}";                                                        //
        html += "tr:nth-child(even) {background-color: #dddddd;}";                                                   //
        html += "</style>";                                                                                          //

        html += "<table>";                                                                     //
        html += "<thead>";                                                                     //
        html += "<tr height='30px' style='background-color: #ff8888;'>";                       //
        html += "<th width='100%' align='left'><a>Hyperlink</a></th>";                         //
        html += "</tr>";                                                                       //
        html += "</thead>";                                                                    //
        html += "<tr><td align='left'><a href='/reset'>Reset</a></td></tr>";                   //
        html += "<tr><td align='left'><a href='/'>Home page</a></td></tr>";                    //
        html += "<tr><td align='left'><a href='/wifi'>Wifi setting</a></td></tr>";             //
        html += "<tr><td align='left'><a href='/cmd'>Comman Port</a></td></tr>";               //
        html += "<tr><td align='left'><a href='/firmware'>Firmware Update</a></td></tr>";             //
        html += "<tr><td align='left'><a href='/DataFileRead'>Read data flie</a></td></tr>";   //
        html += "<tr><td align='left'><a href='/DataFileWrite'>Write data flie</a></td></tr>"; //
        html += "</table>";                                                                    //
        html += "<br>";                                                                        //

        html += "<table>";                                               // tạo table
        html += "<thead>";                                               //
        html += "<tr height='30px' style='background-color: #ff8888;'>"; // tạo dấu ngắt định dạng table
        html += "<th width='30%' align='left'><a>filename</a></th>";     // hiển thị tên file
        html += "<th width='10%' align='center'><a>size</a></th>";       // hiển thị kích thước file
        html += "<th width='20%' align='center'><a>download</a></th>";   // hiển thị phím tải xuống file
        html += "<th width='20%' align='center'><a>edit</a></th>";       // hiển thị phím sửa file
        html += "<th width='20%' align='center'><a>delete</a></th>";     // hiển thị phím xóa file
        html += "</tr>";                                                 // kết thúc dòng table
        html += "</thead>";                                              //

        SERIAL.println("scan file");                    // hiển thị tên file lên serial
        DynamicJsonDocument root(4096);                 // tạo tệp Json
        String filehref;                                //
        String filename;                                //
        String filesize;                                //
        String tab_str;                                 //
        String dir_str;                                 //
        int tab_of_dir = 1;                             //
        root["dir" + String(tab_of_dir)] = "/";         //
        root["dir" + String(tab_of_dir) + "count"] = 0; //

    next_dir: //
#if defined(ESP8266)
        dir_str = String(root["dir" + String(tab_of_dir)]); //
        Dir dir = FILESYSTEM.openDir(dir_str);              //
        SERIAL.println("open directory: " + dir_str);       // hiển thị tên file lên serial

        for (int i = 0; i < int(root["dir" + String(tab_of_dir) + "count"]); i++)
            dir.next(); // bỏ qua các tệp đã đọc trước đó

    next_file:
        root["dir" + String(tab_of_dir) + "count"] = int(root["dir" + String(tab_of_dir) + "count"]) + 1;

        if (dir.next())
        { //

            filename = String(dir.fileName()); // đọc tên file
            filesize = String(dir.fileSize()); // đọc kích thước file
            tab_str = "";                      //
            for (int i = 1; i < tab_of_dir; i++)
                tab_str += "&emsp;"; //

            if (filename.startsWith("/"))
                filename = filename.substring(1); // nếu tên file bắt đầu bằng dấu "/" thì xóa dấu "/"
            if (dir.isDirectory())
            {                                            // nếu là thư mục
                cmd.println(filename + " is directory"); // hiển thị tên file lên serial
                tab_of_dir++;                            //
                root["dir" + String(tab_of_dir)] = String(root["dir" + String(tab_of_dir - 1)]) + filename + "/";
                root["dir" + String(tab_of_dir) + "count"] = 0; //
                goto label_html_directory;                      //
            }
            else
            {                                                                   //
                filehref = String(root["dir" + String(tab_of_dir)]) + filename; //
                goto label_html_flie;                                           //
            } //
        } //

#endif // ESP8266

#if defined(ESP32)
        dir_str = root["dir" + String(tab_of_dir)].as<String>(); //
        File dir = FILESYSTEM.open(dir_str);                     // mở FILESYSTEM
        SERIAL.println("open directory: " + dir_str);            // hiển thị tên file lên serial

        for (int i = 0; i < int(root["dir" + String(tab_of_dir) + "count"]); i++)
            dir.openNextFile(); // bỏ qua các tệp đã đọc trước đó

    next_file:
        root["dir" + String(tab_of_dir) + "count"] = int(root["dir" + String(tab_of_dir) + "count"]) + 1;
        String NextFileName = dir.getNextFileName();
        File file = FILESYSTEM.open(NextFileName); // mở file tiếp theo
        if (file)
        {                                   // đọc tất cả các file
            filename = NextFileName;        // đọc tên file
            filesize = String(file.size()); // đọc kích thước file
            tab_str = "";                   //
            for (int i = 1; i < tab_of_dir; i++)
                tab_str += "&emsp;"; //

            if (filename.startsWith("/"))
                filename = filename.substring(1); // nếu tên file bắt đầu bằng dấu "/" thì xóa dấu "/"
            if (file.isDirectory())
            {                                            // nếu là thư mục
                SERIAL.println(filename + " is directory"); // hiển thị tên file lên serial
                tab_of_dir++;                            //
                root["dir" + String(tab_of_dir)] = root["dir" + String(tab_of_dir - 1)].as<String>() + filename + "/";
                root["dir" + String(tab_of_dir) + "count"] = 0; //
                goto label_html_directory;                      //
            }
            else
            {                                                                        //
                filehref = root["dir" + String(tab_of_dir)].as<String>() + filename; //
                goto label_html_flie;                                                //
            } //
        }
#endif // ESP32

        tab_of_dir--;
        if (tab_of_dir)
            goto next_dir;

        if (0)
        { // bỏ qua khi chạy từ trên xuống

        label_html_directory:                      //
            SERIAL.println("DIRECTORY: " + filename); // hiển thị tên file lên serial

            html += "<tr>";                                                         // tạo dấu ngắt định dạng table
            html += "<td width='30%' align='left'>" + tab_str + filename + "</td>"; // hiển thị tên file
            html += "<td></td>";                                                    //
            html += "<td></td>";                                                    //
            html += "<td></td>";                                                    //
            html += "<td></td>";                                                    //
            html += "</tr>";                                                        // kết thúc dòng table
            goto next_dir;                                                          //

        label_html_flie:
            SERIAL.println("FILE: " + filename + " \t" + filesize + "B"); // hiển thị tên file lên serial

            html += "<tr>";                                                                                 // tạo dấu ngắt định dạng table
            html += "<td align='left'>" + tab_str + "<a href='" + filehref + "'>" + filename + "</a></td>"; // hiển thị tên file
            html += "<td align='center'>" + filesize + "B</td>";                                            // hiển thị kích thước file
            html += "<td align='center'><a href='/file_download?name=" + filehref + "'>download</a></td>";  // hiển thị phím xóa file
            html += "<td align='center'><a href='/file_edit.html?name=" + filehref + "'>edit</a></td>";     // hiển thị phím xóa file
            html += "<td align='center'><a href='/file_delete?name=" + filehref + "'>delete</a></td>";      // hiển thị phím xóa file
            html += "</tr>";                                                                                // kết thúc dòng table
            goto next_file;
        }

        html += "</table>"; // kết thúc table

        html += "<form method='post' enctype='multipart/form-data'>";  // form uoload file
        html += "<input type='file' name='name'>";                     //
        html += "<input class='button' type='submit' value='Upload'>"; //
        html += "</form>";                                             //
        html += "<br><br>";                                            //

        html += "</body>"; //
        html += "</html>"; //

        server.send(200, "text/html", html); // gửi dưới dạng html
    });                                      //

    server.on("/file_read", HTTP_GET, []() {             // đọc file
        FLASH_ACTIVE_LED;                                // bật đèn
        String filename = server.arg("name");            // đọc tên file
        if (!filename.startsWith("/"))                   //
            filename = "/" + filename;                   // nếu tên file không bắt đầu bằng "/" thì thêm dấu "/" vào tên file
        File file = FILESYSTEM.open(filename, "r");      // mở tệp ở chế độ đọc
        String DataFile = file.readString();             // đọc file
        file.close();                                    // đóng file
        server.send(200, "text/plain", DataFile);        // gửi dữ liệu
        SERIAL.print("Handle File Read Name: " + filename); // xuất lên serial
        SERIAL.println(DataFile);                           // hiển thị lên Serial
    });                                                  //

    server.on("/file_write", HTTP_POST, []() {            // lưu file
        FLASH_ACTIVE_LED;                                 // bật đèn
        String filename = server.arg("name");             // đọc tên file
        if (!filename.startsWith("/"))                    //
            filename = "/" + filename;                    // nếu tên file không bắt đầu bằng "/" thì thêm dấu "/" vào tên file
        File file = FILESYSTEM.open(filename, "w");       // mở tệp ở chế độ ghi
        String DataFile = server.arg("plain");            // lấy nội dung file
        file.print(DataFile);                             // ghi file
        file.close();                                     // đóng file
        server.send(200, "text/plain", DataFile);         // gửi dữ liệu
        SERIAL.print("Handle Fil eWrite Name: " + filename); // xuất lên serial
        SERIAL.println(DataFile);                            // hiển thị lên Serial
    });                                                   //

    server.on("/file_delete", []() {                     // lệnh xóa file
        FLASH_ACTIVE_LED;                                // bật led báo
        String filename = server.arg("name");            // đọc tên file
        if (!filename.startsWith("/"))                   //
            filename = "/" + filename;                   // nếu tên file không bắt đầu bằng "/" thì thêm dấu "/" vào tên file
        FILESYSTEM.remove(filename);                     // xóa file
        server.sendHeader("Location", "/file");          // chuyển hướng đến trang quản lý file
        server.send(303);                                // xác nhận chuyển hướng file
        SERIAL.print("handleFileDelete Name: " + filename); // xuất lên serial
    });                                                  //

    server.on("/file_download", []() {                                                    // lệnh tải file
        FLASH_ACTIVE_LED;                                                                 // bật led báo
        String filename = server.arg("name");                                             // đọc tên file
        if (!filename.startsWith("/"))                                                    //
            filename = "/" + filename;                                                    // nếu tên file không bắt đầu bằng "/" thì thêm dấu "/" vào tên file
        File download = FILESYSTEM.open(filename, "r");                                   // mở file ở chế độ đọc
        if (filename.startsWith("/"))                                                     //
            filename = filename.substring(1);                                             // nếu tên file bắt đầu bằng dấu "/" thì xóa dấu "/"
        if (download)                                                                     //
        {                                                                                 // nếu file không lỗi
            server.sendHeader("Content-Type", "text/text");                               //
            server.sendHeader("Content-Disposition", "attachment; filename=" + filename); //
            server.sendHeader("Connection", "close");                                     //
            server.streamFile(download, "application/octet-stream");                      //
            download.close();                                                             //
        } 
        server.send(500, "text/plain", "500: couldn't load file"); // báo không thể hoàn thành mở file                                                                     //
    });                                                            //

    server.on("/file_format", []() {            // lệnh xóa tất cả các file
        FLASH_ACTIVE_LED;                       // bật led báo
        FILESYSTEM.format();                    // format
        server.sendHeader("Location", "/file"); // chuyển hướng đến trang quản lý file
        server.send(303);                       // xác nhận chuyển hướng file
    });                                         //

} 
