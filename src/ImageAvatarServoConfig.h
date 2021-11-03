#ifndef _IMAGEAVATAR_SERVO_CONFIG_H_
#define _IMAGEAVATAR_SERVO_CONFIG_H_

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ESP32-Chimera-Core.h>

#define AXIS_NUMBER 2   // number of ServoAxis


typedef struct ServoInitial {
    uint8_t pin;            // servo pinno
    int position_center;    // servo center degree
    int position_upper;     // servo upper limit degree
    int position_lower;     // servo lower degree
} servo_initial_s;

class ImageAvatarServoConfig {
    private:
        const char *_filename;              // json filename
        bool _servo_enable;                  // When this flag is true, the servo moves.
        void setServoSettings(DynamicJsonDocument doc);
        servo_initial_s _servo_initial[AXIS_NUMBER];
    public:
        ImageAvatarServoConfig();
        ~ImageAvatarServoConfig();
        void loadConfig(fs::FS& fs, const char *json_filename);

        void printAllParameters();
        servo_initial_s* getServoSettings(uint8_t axis_no);

};


#endif // _IMAGEAVATAR_SERVO_CONFIG_
