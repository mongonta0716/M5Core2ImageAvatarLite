#include "ImageAvatarSystemConfig.h"


ImageAvatarSystemConfig::ImageAvatarSystemConfig() {};
ImageAvatarSystemConfig::~ImageAvatarSystemConfig() {};

void ImageAvatarSystemConfig::loadConfig(fs::FS& fs, const char *json_filename) {
    Serial.printf("----- ImageAvatarSystemConfig::loadConfig:%s\n", json_filename);
    File file = fs.open(json_filename);
    int res = file.available();
    Serial.printf("file:available:%d\n", res);
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.printf("json file read error: %s\n", json_filename);
        Serial.printf("error%s\n", error.c_str());
    }
    setSystemConfig(doc);

    file.close();
#ifdef DEBUG
     printAllParameters();
#endif
}

void ImageAvatarSystemConfig::setSystemConfig(DynamicJsonDocument doc) {

    _volume = doc["volume"];
    _lcd_brightness = doc["lcd_brightness"];
    JsonArray avatar_json = doc["avatar_json"];
    _avatar_count = avatar_json.size();

    for (int i=0; i<_avatar_count; i++) {
        _avatar_jsonfiles[i] = avatar_json[i].as<String>();
    }
    _bluetooth_device_name = doc["bluetooth_device_name"].as<String>();
    _bluetooth_reconnect = doc["bluetooth_reconnect"];
    _servo_jsonfile = doc["servo_json"].as<String>(); 
    _servo_random_mode = doc["servo_random_mode"];
}

void ImageAvatarSystemConfig::printAllParameters() {
    Serial.printf("Volume: %d\n", _volume);
    Serial.printf("Brightness: %d\n", _lcd_brightness);
    Serial.printf("Avatar Max Count: %d\n", _avatar_count);
    for (int i=0; i<_avatar_count; i++) {
        Serial.printf("Avatar Json FileName:%d :%s\n", i, (const char *)_avatar_jsonfiles[i].c_str());
    }
    Serial.printf("Bluetooth Device Name: %s\n", _bluetooth_device_name);
    Serial.printf("Bluetooth Reconnect :%s\n", _bluetooth_reconnect ? "true" : "false");
    Serial.printf("Servo Json FileName: %s\n", (const char *)_servo_jsonfile.c_str());
    Serial.printf("Servo Random Mode:%s\n", _servo_random_mode ? "true" : "false");
}