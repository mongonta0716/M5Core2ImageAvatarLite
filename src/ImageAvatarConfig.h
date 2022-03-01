#ifndef _IMAGEAVATARCONFIG_H_
#define _IMAGEAVATARCONFIG_H_

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <M5Unified.h>

#define RIGHT true
#define LEFT  false

#define MAX_FILENAME 100

#define MAX_EXPRESSION 8

typedef struct ColorCommon {
    uint32_t transparent; // Color of transparent
    uint32_t skin; // Color of shin
    uint32_t eye_white; // Color of fill eyeball
} color_s;

// Sprite Common Settings
typedef struct SpriteCommon {
    bool    psram;
    uint8_t color_depth;
    bool    swap_bytes;
    color_s colors;
} spcommon_s;

typedef struct PictureInfo {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
} picinfo_s;


typedef struct SpriteParamsFixedParts {
    picinfo_s picinfo;
    const char* filename;
} params_fixed_s;

typedef struct SpriteParamsMouth {
    picinfo_s picinfo;
    const char* filename_op;
    const char* filename_cl;
    float min_scaleX;
    float max_scaleX;
    float min_scaleY;
    float max_scaleY;
} params_mouth_s;

typedef struct SpriteParamsEyes {
    picinfo_s left;
    picinfo_s right;
    const char* filename_op;
    const char* filename_cl;
    float min_scaleX;
    float max_scaleX;
    float min_scaleY;
    float max_scaleY;
} params_eyes_s;

typedef struct MoveParam {
    float eye_l_ratio;
    float eye_r_ratio;
    float mouth_ratio;
    float angle;
    int breath;
} move_param_s;

class ImageAvatarConfig {
    private:
        const char *_filename;
        const char *_expression[MAX_EXPRESSION];
        uint8_t _max_expresssion = 0;
        spcommon_s _spcommon;
        params_fixed_s _head;
        params_mouth_s _mouth[MAX_EXPRESSION];
        params_eyes_s _eyes[MAX_EXPRESSION];
        move_param_s _mv[MAX_EXPRESSION];
        void setCommonParameters(DynamicJsonDocument doc);
        void setHeadParameters(DynamicJsonDocument doc);
        void setMouthParameters(DynamicJsonDocument doc);
        void setEyesParameters(DynamicJsonDocument doc);
        void setInitParameters(DynamicJsonDocument doc);
    public:
        ImageAvatarConfig();
        ~ImageAvatarConfig();
        void loadConfig(fs::FS& fs, const char *json_filename);

        void printAllParameters();
        spcommon_s getSpriteCommonParameters();
        params_fixed_s getHeadParameters();
        params_mouth_s getMouthParameters(uint8_t expression);
        params_eyes_s getEyesParameters(uint8_t expression);
        move_param_s getMoveParameters(uint8_t expression);

};


#endif
