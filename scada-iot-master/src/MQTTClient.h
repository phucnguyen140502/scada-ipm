
#include <WiFi.h> // thư viện wifi
#include <PubSubClient.h>
#include <ArduinoJson.h> // thư viện chuẩn dữ liệu

// Utility function to get formatted MAC address (e.g., 6cc84056319c)
String getFormattedMAC() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return mac;
}

// Get device ID using MAC address with colons
String getDeviceID() {
  return getFormattedMAC();
}

void MQTTsendDATA(int key = 0) {
  static unsigned long t;
  if (key) t = millis() + 2000ul;
  if (t > millis()) return;
  t = millis() + 10000ul;
  FLASH_ACTIVE_LED;

  power_meter.read(1);
  DynamicJsonDocument root(4096); // tạo tệp Json lưu dữ liệu tạm thời

  root["time"]          = DayTime.unixtime;
  root["auto"]          = JsonData["auto"];
  root["toggle"]        = JsonData["toggle"];
  root["gps_log"]       = JsonData["gps_log"];
  root["gps_lat"]       = JsonData["gps_lat"];
  root["voltage"]       = JsonData["voltage"];
  root["current"]       = JsonData["current"];
  root["power"]         = JsonData["power"];
  root["power_factor"]  = JsonData["power_factor"];
  root["frequency"]     = JsonData["frequency"];
  root["total_energy"]  = JsonData["total_energy"];

  root["hour_on"]       = JsonData["hour_on"];
  root["minute_on"]     = JsonData["minute_on"];
  root["hour_off"]      = JsonData["hour_off"];
  root["minute_off"]    = JsonData["minute_off"];

  String output; // Serialize JSON and publish
  serializeJson(root, output);
  SERIAL.println("Publishing JSON data:");
  SERIAL.println(output);

  String topic_status = MQTT_TOPIC_PREFIX + getDeviceID() + MQTT_STATUS_TOPIC;

  if (client.publish(topic_status.c_str(), output.c_str()))
    SERIAL.println("JSON data published successfully");
  else
    SERIAL.println("Failed to publish JSON data");
}

void handleCommand(const String &command) {
  DynamicJsonDocument root(4096);
  DeserializationError error = deserializeJson(root, command);

  if (error) {
    SERIAL.println("Failed to parse command JSON.");
    return;
  }

  String commandType = root["command"];

  if (commandType == "REBOOT") {
    SERIAL.println("Rebooting device...");
    ESP.restart();

  } else if (commandType == "AUTO") {
    String state = root["payload"];

    if (state == "on") {
      SERIAL.println("Enabling auto mode...");
      JsonData["auto"] = 1;
    } else if (state == "off") {
      SERIAL.println("Disabling auto mode...");
      JsonData["auto"] = 0;
    } else {
      SERIAL.print("Unknown auto state: ");
      SERIAL.println(state);
    }

  } else if (commandType == "TOGGLE") {
    String state = root["payload"];
    if (state == "on") {
      SERIAL.println("Toggling device ON...");
      JsonData["toggle"] = 1;
    } else if (state == "off") {
      SERIAL.println("Toggling device OFF...");
      JsonData["toggle"] = 0;
    } else {
      SERIAL.print("Unknown toggle state: ");
      SERIAL.println(state);
    }
  } else if (commandType == "SCHEDULE") {
    JsonObject payload      = root[   "payload"];
    JsonData["hour_on"]     = payload["hour_on"];
    JsonData["minute_on"]   = payload["minute_on"];
    JsonData["hour_off"]    = payload["hour_off"];
    JsonData["minute_off"]  = payload["minute_off"];
  } else {
    SERIAL.print("Unknown command type: ");
    SERIAL.println(commandType);
  }

  DataFile_write();
  MQTTsendDATA(1);
}

void MQTTcallback(char *topic, uint8_t *payload, unsigned int length) {
  FLASH_ACTIVE_LED

  String topicStr = String(topic);
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  SERIAL.print("Main - Message arrived on topic: ");
  SERIAL.println(topicStr);
  SERIAL.print("Main - Message: ");
  SERIAL.println(message);

  String topic_ID         = getDeviceID();
  String topic_command    = MQTT_TOPIC_PREFIX + topic_ID + MQTT_COMMAND_TOPIC;
  String topic_status     = MQTT_TOPIC_PREFIX + topic_ID + MQTT_STATUS_TOPIC;
  String topic_alive      = MQTT_TOPIC_PREFIX + topic_ID + MQTT_ALIVE_TOPIC;
  String topic_updateID   = MQTT_TOPIC_PREFIX + topic_ID + MQTT_FIRMWARE_UPDATE_TOPIC;
  String topic_update     = MQTT_FIRMWARE_UPDATE_TOPIC;

  if ((topicStr == topic_update) || (topicStr == topic_updateID)) { // Handle OTA messages{
    Lcd.print_message("OTA update...", 0, true);
    otaHandler.handleOtaMessage(message);
  } else if (topicStr == topic_command) { // Handle business logic messages{
    SERIAL.println("Main - Processing business logic command...");
    handleCommand(message); // Handle the command logic
  }
}

void MQTTClient_begin() {
}


void MQTTClient_loop() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  if (!client.connected()) {
    static uint32_t Time_reconnect_mqtt;
    if (Time_reconnect_mqtt > millis())
      return;
    Time_reconnect_mqtt = millis() + 30000;
    FLASH_ACTIVE_LED;

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(MQTTcallback);

    String topic_ID = getDeviceID();
    String topic_command  = MQTT_TOPIC_PREFIX + topic_ID + MQTT_COMMAND_TOPIC;
    String topic_status   = MQTT_TOPIC_PREFIX + topic_ID + MQTT_STATUS_TOPIC;
    String topic_alive    = MQTT_TOPIC_PREFIX + topic_ID + MQTT_ALIVE_TOPIC;
    String topic_updateID = MQTT_TOPIC_PREFIX + topic_ID + MQTT_FIRMWARE_UPDATE_TOPIC;
    String topic_update   = MQTT_FIRMWARE_UPDATE_TOPIC;

    String lwt_message    = "0";
    String client_id      = "esp32-client-" + topic_ID;

    client.setBufferSize(512);
    SERIAL.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password, topic_alive.c_str(), 1, false, lwt_message.c_str())) {
      SERIAL.println("Public EMQX MQTT broker connected");
    }
    else {
      SERIAL.print("Failed with state ");
      SERIAL.println(client.state());
    }

    client.publish(  topic_alive.c_str(), "6", true);
    client.subscribe(topic_command.c_str());
    client.subscribe(topic_update.c_str());
    client.subscribe(topic_updateID.c_str());
    return;
  }

  client.loop(); // Handle MQTT communication
  MQTTsendDATA();
}
