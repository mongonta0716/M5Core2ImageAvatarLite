#include "ImageAvatarServo.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ServoEasing.hpp"

static ServoEasing _servo[AXIS_NUMBER];

ImageAvatarServo::ImageAvatarServo(fs::FS& json_fs, const char* filename) {
    _json_fs = &json_fs;
    _filename = filename;
    _config.loadConfig(json_fs, filename);
    _config.printAllParameters();
    for (int i=0;i<AXIS_NUMBER;i++) {
        _servo_init[i] = _config.getServoSettings(i);
    }
}

ImageAvatarServo::~ImageAvatarServo(void) {
}

int ImageAvatarServo::checkParam(uint8_t servo_no, int degree) {
    Serial.printf("checkParam:%d", degree);
    if (_servo_init[servo_no]->position_upper < degree) {
        return _servo_init[servo_no]->position_upper;
    } 
    if (_servo_init[servo_no]->position_lower > degree) {
        return _servo_init[servo_no]->position_lower;
    }
    return degree;
}

void ImageAvatarServo::init() {
  _config.loadConfig(*_json_fs, _filename);
  _config.printAllParameters();
}

void ImageAvatarServo::attach(uint8_t servo_no) {
    _servo[servo_no].attach(_servo_init[servo_no]->pin,
                            _servo_init[servo_no]->position_center);
}

void ImageAvatarServo::attachAll() {
    for (int i=0;i<AXIS_NUMBER;i++) {
      attach(i);
    }
}

void ImageAvatarServo::move(uint8_t servo_no, int degree, uint_fast16_t millis_move) {
    _servo[servo_no].startEaseToD(checkParam(servo_no, degree), millis_move);
    while(ServoEasing::areInterruptsActive()) {
      vTaskDelay(33);
    }
}

void ImageAvatarServo::moveXY(int degree_x, int degree_y,
                        uint_fast16_t millis_move_x, uint_fast16_t millis_move_y) {
    _servo[AXIS_X].startEaseToD(checkParam(AXIS_X, degree_x), millis_move_x);
    _servo[AXIS_Y].startEaseToD(checkParam(AXIS_Y, degree_y), millis_move_y);
    while(ServoEasing::areInterruptsActive()) {
      vTaskDelay(33);
    }
}

void ImageAvatarServo::check() {
    for (int i=0;i<AXIS_NUMBER;i++) {
      move(i, _servo_init[i]->position_center, 1000);
      move(i, _servo_init[i]->position_upper, 1000);
      move(i, _servo_init[i]->position_center, 1000);
      move(i, _servo_init[i]->position_lower, 1000);
      move(i, _servo_init[i]->position_center, 1000);
    }
}