#ifndef _IMAGEAVATAR_SERVO_H_
#define _IMAGEAVATAR_SERVO_H_

#include "ImageAvatarServoConfig.h"

enum Servos {
    AXIS_X,
    AXIS_Y
};

class ImageAvatarServo
{
    private:
        bool _servo_enable;

        ImageAvatarServoConfig _config;
        servo_initial_s* _servo_init[AXIS_NUMBER];
        fs::FS *_json_fs;   // 設定ファイルの収納場所(SD or SPIFFS)
        const char* _filename;
        int checkParam(uint8_t servo_no, int degree);
        int _last_position[AXIS_NUMBER];


    public:
        ImageAvatarServo(fs::FS& json_fs, const char* filename);
        ~ImageAvatarServo(void);
        void init();
        void attach(uint8_t servo_no);
        void attachAll();
        void detach(uint8_t servo_no);
        void detachAll();

        void move(uint8_t servo_no, int degree, uint_fast16_t millis_move);

        void moveXY(int degree_x, int degree_y,
                uint_fast16_t millis_move_x, uint_fast16_t millis_move_y, bool auto_attach=false);
        
        // void loop(void* args);

        void check();

        void setServoEnable(bool servo_enable) { _servo_enable = servo_enable; };
        servo_initial_s* getServoConfig(uint8_t axis_no) { return _servo_init[axis_no]; };
        
};

#endif // _IMAGEAVATAR_SERVO_H_
