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
        _last_position[i] = _servo_init[i]->position_center;
    }
}

ImageAvatarServo::~ImageAvatarServo(void) {
}

int ImageAvatarServo::checkParam(uint8_t servo_no, int degree) {
    //Serial.printf("checkParam:%d", degree);
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
    _last_position[servo_no] = _servo_init[servo_no]->position_center;
    _servo[servo_no].attach(_servo_init[servo_no]->pin,
                            _last_position[servo_no]); 
    _servo[servo_no].setEasingType(EASE_QUADRATIC_IN_OUT);
}

void ImageAvatarServo::attachAll() {
    for (int i=0;i<AXIS_NUMBER;i++) {
        attach(i);
    }
}

void ImageAvatarServo::detach(uint8_t servo_no) {
    _servo[servo_no].detach();
}

void ImageAvatarServo::detachAll() {
    for (int i=0;i<AXIS_NUMBER;i++) {
        detach(i);
    }
}

void ImageAvatarServo::move(uint8_t servo_no, int degree, uint_fast16_t millis_move) {
    _last_position[servo_no] = checkParam(servo_no, degree);
    _servo[servo_no].startEaseToD(_last_position[servo_no], millis_move);
    while(ServoEasing::areInterruptsActive()) {
      vTaskDelay(33);
    }
}

void ImageAvatarServo::moveXY(int degree_x, int degree_y,
                        uint_fast16_t millis_move_x, uint_fast16_t millis_move_y,
                        bool auto_attach) {
    if (auto_attach) {
        for (int i=0;i<AXIS_NUMBER;i++) {
            _servo[i].attach(_servo_init[i]->pin, _last_position[i]);
        }
    }
    Serial.printf("last_moveX: %d\n", _last_position[AXIS_X]);
    Serial.printf("last_moveY: %d\n", _last_position[AXIS_Y]);
    _last_position[AXIS_X] = checkParam(AXIS_X, degree_x);
    _last_position[AXIS_Y] = checkParam(AXIS_Y, degree_y);
    Serial.printf("moveX: %d\n", _last_position[AXIS_X]);
    Serial.printf("moveY: %d\n", _last_position[AXIS_Y]);

    _servo[AXIS_X].startEaseToD(_last_position[AXIS_X], millis_move_x);
    _servo[AXIS_Y].startEaseToD(_last_position[AXIS_Y], millis_move_y);
    while(ServoEasing::areInterruptsActive()) {
      vTaskDelay(33);
    }
    if (auto_attach) {
        for (int i=0;i<AXIS_NUMBER;i++) {
            _servo[i].detach();
        }
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