#include "ImageAvatarServoConfig.h"
enum Servos {
    AXIS_X,
    AXIS_Y
};

const char* servoNames[] = { "x_axis", "y_axis" };

ImageAvatarServoConfig::ImageAvatarServoConfig() {

}
ImageAvatarServoConfig::~ImageAvatarServoConfig(){

}

void ImageAvatarServoConfig::loadConfig(fs::FS& fs, const char *filename) {
    Serial.printf("----- ImageAvatarServoConfig::loadConfig:%s\n", filename);
    File file = fs.open(filename);
    int res = file.available();
    Serial.printf("file:available:%d\n", res);
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.printf("json file read error: %s\n", filename);
        Serial.printf("error%s\n", error.c_str());
    }
    setServoSettings(doc);

    file.close();
#ifdef DEBUG
     printAllParameters();
#endif
}

void ImageAvatarServoConfig::setServoSettings(DynamicJsonDocument doc) {
     // SetParameters
    JsonObject initial_settings = doc["initial_settings"];
    JsonObject axis_x = initial_settings["x_axis"];
    _servo_initial[AXIS_X].pin = axis_x["pin"];
    _servo_initial[AXIS_X].position_center = axis_x["center"];
    _servo_initial[AXIS_X].position_upper  = axis_x["upper"];
    _servo_initial[AXIS_X].position_lower  = axis_x["lower"];
    _servo_initial[AXIS_X].offset          = axis_x["offset"];

    JsonObject axis_y = initial_settings["y_axis"];
    _servo_initial[AXIS_Y].pin = axis_y["pin"];
    _servo_initial[AXIS_Y].position_center = axis_y["center"];
    _servo_initial[AXIS_Y].position_upper  = axis_y["upper"];
    _servo_initial[AXIS_Y].position_lower  = axis_y["lower"];
    _servo_initial[AXIS_Y].offset          = axis_y["offset"];
   
}

servo_initial_s* ImageAvatarServoConfig::getServoSettings(uint8_t axis_no) {
    return &_servo_initial[axis_no];
}

void ImageAvatarServoConfig::printAllParameters() {
    Serial.println("ImageAvatarServoConfig:printAllParameters");
    for (int i=0; i<AXIS_NUMBER; i++) {
        Serial.printf("ServoName:%s", servoNames[i]);
        Serial.printf("servo_pin:%d,", _servo_initial[i].pin);
        Serial.printf("servo_center:%d,", _servo_initial[i].position_center);
        Serial.printf("servo_upper:%d,", _servo_initial[i].position_upper);
        Serial.printf("servo_lower:%d,", _servo_initial[i].position_lower);
        Serial.printf("offset:%d\n", _servo_initial[i].offset);
    }
}
