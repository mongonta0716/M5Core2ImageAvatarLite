#ifndef _IMAGEAVATARLITE_H_
#define _IMAGEAVATARLITE_H_

#include "ImageAvatarConfig.h"

namespace m5imageavatar {

#define DEFAULT_STACK_SIZE 2048

class ImageAvatarLite
{
    private:
        M5GFX *_gfx;
        M5Canvas *_head_sp;     // 頭（背景）用スプライト
        M5Canvas *_eye_op_sp;   // 開いた右目のスプライト
        M5Canvas *_eye_cl_sp;   // 閉じた右目のスプライト
        M5Canvas *_mouth_op_sp; // 開いた口のスプライト
        M5Canvas *_mouth_cl_sp; // 閉じた口のスプライト
        M5Canvas *_mouth_sp;    // 口描画用スプライト
        M5Canvas *_eye_l_sp;    // 左目描画用スプライト
        M5Canvas *_eye_r_sp;    // 右目描画用スプライト
        M5Canvas *_lcd_sp;      // LCDに最終的に描画する直前のスプライト
        move_param_s _mv;

        bool _is_change;
        uint8_t _expression;    // 感情
        uint32_t _draw_time;

        fs::FS *_json_fs;   // 設定ファイルの収納場所(SD or SPIFFS)
        fs::FS *_bmp_fs;    // ビットマップファイルの収納場所（SD or SPIFFS）
        const char* _filename;
        ImageAvatarConfig _config;
        spcommon_s _spcommon;

        lgfx::rgb565_t _tp_color; // 透明色

        void initSprites(bool is_change);
        void deleteSprites();
        void loadConfig(fs::FS& fs, const char* filename);
        void execDraw();

        void drawHead();
        void drawEyes();
        void drawMouth();
        lgfx::rgb565_t convertColorCode(uint32_t code);

    public:
        // ImageAvatarLite(void);
        ImageAvatarLite(fs::FS& json_fs, fs::FS& bmp_fs);
        ~ImageAvatarLite(void);

        void start();
        void addTask(TaskFunction_t f, const char* name, uint8_t task_priority = 6, uint16_t stack_size = DEFAULT_STACK_SIZE);
        void createSprite();
        void init(M5GFX *gfx, const char* filename, bool is_change, uint8_t expression = 0);
        void drawAll();
        void changeAvatar(const char* filename, uint8_t expression = 0);
        void setMoveParameter(move_param_s mv);
        void setExpression(const char* filename, uint8_t expression);
        void setBreath(float f);
        void setBlink(float ratio);
        void setBlink(float ratio, bool isRight);
        void setAngle(float angle);
        void setMouthOpen(float ratio);

        float getMouthOpen();
        move_param_s getMoveParameter();
        uint8_t getExpressionMax();

        void readTest();
        void drawTest();
};

class DriveContext {
    private:
        ImageAvatarLite *avatar;

    public:
        DriveContext() = delete;
        explicit DriveContext(ImageAvatarLite *avatar);
        ~DriveContext() = default;
        DriveContext(const DriveContext &other) = delete;
        DriveContext &operator=(const DriveContext &other) = delete;
        ImageAvatarLite *getAvatar();
};
} // namespace m5imageavatarlite
#endif // _IMAGEAVATARLITE_H_
