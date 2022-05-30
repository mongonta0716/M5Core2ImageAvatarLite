#ifndef _IMAGEAVATAR_SYSTEM_CONFIG_H_
#define _IMAGEAVATAR_SYSTEM_CONFIG_H_

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <M5Unified.h>

#define MAX_AVATAR_NUM 8  // 最大8個のAvatar

class ImageAvatarSystemConfig {
    protected:
        uint8_t _volume;                                       // Initial volume
        uint8_t _lcd_brightness;                               // brightness of lcd
        uint8_t _avatar_count;                                 // avatar count
        String _avatar_jsonfiles[MAX_AVATAR_NUM];              // json filename
        String _bluetooth_device_name;                         // bluetooth_device_name
        bool _bluetooth_reconnect;                             // 起動時にBluetoothを再接続するかどうか
        String _servo_jsonfile;                                // servo json filename
        bool _servo_random_mode;                               // servo random mode flag
        uint32_t _auto_power_off_time;                         // USB給電が停止後、電源OFFするまでの時間(msec)。0だと電源OFFしない。
        void setSystemConfig(DynamicJsonDocument doc);
    public:
        ImageAvatarSystemConfig();
        ~ImageAvatarSystemConfig();
        void loadConfig(fs::FS& fs, const char *json_filename);

        void printAllParameters();
        uint8_t getVolume() { return _volume; }
        uint8_t getLcdBrightness() { return _lcd_brightness; };
        uint8_t getAvatarMaxCount() { return _avatar_count; }
        String getBluetoothDeviceName() { return _bluetooth_device_name; }
        bool getBluetoothReconnect() { return _bluetooth_reconnect; }
        String getAvatarJsonFilename(uint8_t avatar_no) { return _avatar_jsonfiles[avatar_no]; }
        String getServoJsonFilename() { return _servo_jsonfile; }
        bool getServoRandomMode() { return _servo_random_mode; }
        uint32_t getAutoPowerOffTime() { return _auto_power_off_time; }
};


#endif // _IMAGEAVATAR_SERVO_CONFIG_
