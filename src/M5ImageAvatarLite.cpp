#include "M5ImageAvatarLite.h"

namespace m5imageavatar {


DriveContext::DriveContext(ImageAvatarLite *avatar) : avatar{ avatar } {}
ImageAvatarLite *DriveContext::getAvatar() { return avatar; }

TaskHandle_t drawTaskHandle;
TaskHandle_t blinkTaskHandle;
TaskHandle_t breathTaskHandle;


void drawLoop(void *args) {

    DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
    ImageAvatarLite *avatar = ctx->getAvatar();
    for(;;) {
        avatar->drawTest();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void breath(void *args) {
    DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
    ImageAvatarLite *avatar = ctx->getAvatar();
    uint32_t c = 0;
    for(;;) {
        c = c + 1 % 100;
        float f = sin(c) * 2;
        avatar->setBreath(f);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
void blink(void *args) {
    DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
    ImageAvatarLite *avatar = ctx->getAvatar();
    for(;;) {
        // まぶたの動きをリアルにしたいのでfor文を使用
        for(float f=0.0; f<=1; f=f+0.1) {
            avatar->setBlink(f);
            delay(10/portTICK_PERIOD_MS);
        }
        vTaskDelay(2000 + 100 * random(20));
        for(float f=1.0; f>=0; f=f-0.1) {
            avatar->setBlink(f);
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
        vTaskDelay(300 + 10 * random(20));
    }
}

ImageAvatarLite::ImageAvatarLite(fs::FS& json_fs, fs::FS& bmp_fs) {
    _expression = 0;
    _json_fs = &json_fs;
    _bmp_fs = &bmp_fs;
}
ImageAvatarLite::~ImageAvatarLite(void) {
    deleteSprites();
}


void ImageAvatarLite::loadConfig(fs::FS& fs, const char* filename) {
    _config.loadConfig(fs, filename);
}

lgfx::rgb565_t ImageAvatarLite::convertColorCode(uint32_t code) {
    uint8_t r = code >> 16;
    uint8_t g = code >> 8 & 0xff;
    uint8_t b = code & 0xff;
    Serial.printf("rgb, %d,%d,%d\n", r, g, b);
    return lgfx::color565(r, g, b);
}

void ImageAvatarLite::init(M5GFX *gfx, const char* filename, bool is_change,
                           uint8_t expression) {
    loadConfig(*_json_fs, filename);
    this->_gfx = gfx;
    this->_filename = filename;
    _lcd_sp      = new M5Canvas(_gfx);
    _head_sp     = new M5Canvas(_lcd_sp);
    _mouth_sp    = new M5Canvas(_lcd_sp);
    _mouth_op_sp = new M5Canvas(_lcd_sp);
    _mouth_cl_sp = new M5Canvas(_lcd_sp);
    _eye_l_sp    = new M5Canvas(_lcd_sp);
    _eye_r_sp    = new M5Canvas(_lcd_sp);
    _eye_op_sp   = new M5Canvas(_lcd_sp);
    _eye_cl_sp   = new M5Canvas(_lcd_sp);
    _expression = expression;
    _mv = _config.getMoveParameters(_expression);
    initSprites(is_change);

    // JSONから取り込むとカラーコードがうまく認識できないので変換
    _tp_color = convertColorCode(_spcommon.colors.transparent);
}

void ImageAvatarLite::initSprites(bool is_change) {
    if (is_change) {
        Serial.println("DeleteSprites");
        deleteSprites();
    }
    Serial.printf("initSprites:%s\n", _filename);
    loadConfig(*_json_fs, _filename);
    _config.printAllParameters();
    _spcommon = _config.getSpriteCommonParameters();
    params_fixed_s p_head = _config.getHeadParameters();
    _head_sp->setPsram(_spcommon.psram);
    _head_sp->setColorDepth(_spcommon.color_depth);
    _head_sp->setSwapBytes(_spcommon.swap_bytes);
    _head_sp->createSprite(p_head.picinfo.w, p_head.picinfo.h);
    _head_sp->drawBmpFile(*_bmp_fs, p_head.filename, 0, 0);

    params_mouth_s p_mouth = _config.getMouthParameters(_expression);
    params_eyes_s p_eyes = _config.getEyesParameters(_expression);
    // 固定パーツ
    _eye_op_sp->setPsram(_spcommon.psram);
    _eye_op_sp->setColorDepth(_spcommon.color_depth);
    _eye_op_sp->setSwapBytes(_spcommon.swap_bytes);
    _eye_op_sp->createFromBmpFile(*_bmp_fs, p_eyes.filename_op);
    Serial.printf("eye_sp_width:%d\n", _eye_op_sp->width());
    _eye_cl_sp->setPsram(_spcommon.psram);
    _eye_cl_sp->setColorDepth(_spcommon.color_depth);
    _eye_cl_sp->setSwapBytes(_spcommon.swap_bytes);
    _eye_cl_sp->createFromBmpFile(*_bmp_fs, p_eyes.filename_cl);
    Serial.printf("eye_sp_width:%d\n", _eye_cl_sp->width());

    _mouth_op_sp->setPsram(_spcommon.psram);
    _mouth_op_sp->setColorDepth(_spcommon.color_depth);
    _mouth_op_sp->setSwapBytes(_spcommon.swap_bytes);
    _mouth_op_sp->createFromBmpFile(*_bmp_fs, p_mouth.filename_op);

    _mouth_cl_sp->setPsram(_spcommon.psram);
    _mouth_cl_sp->setColorDepth(_spcommon.color_depth);
    _mouth_cl_sp->setSwapBytes(_spcommon.swap_bytes);
    _mouth_cl_sp->createFromBmpFile(*_bmp_fs, p_mouth.filename_cl);

    // 描画用スプライトの準備（createSpriteのみ）
    _mouth_sp->setPsram(_spcommon.psram);
    _mouth_sp->setColorDepth(_spcommon.color_depth);
    _mouth_sp->setSwapBytes(_spcommon.swap_bytes);
    _mouth_sp->createSprite(p_mouth.picinfo.w, p_mouth.picinfo.h);

    _eye_l_sp->setPsram(_spcommon.psram);
    _eye_l_sp->setColorDepth(_spcommon.color_depth);
    _eye_l_sp->setSwapBytes(_spcommon.swap_bytes);
    _eye_l_sp->createSprite(p_eyes.left.w, p_eyes.left.h);

    _eye_r_sp->setPsram(_spcommon.psram);
    _eye_r_sp->setColorDepth(_spcommon.color_depth);
    _eye_r_sp->setSwapBytes(_spcommon.swap_bytes);
    _eye_r_sp->createSprite(p_eyes.right.w, p_eyes.right.h);

    _lcd_sp->setPsram(_spcommon.psram);
    _lcd_sp->setColorDepth(_spcommon.color_depth);
    _lcd_sp->setSwapBytes(_spcommon.swap_bytes);
    _lcd_sp->createSprite(_gfx->width(), _gfx->height());


}

void ImageAvatarLite::execDraw() {
    _gfx->startWrite();
    _lcd_sp->pushSprite(_gfx, 0, 0);
    _gfx->endWrite();
}

void ImageAvatarLite::drawHead() {
    params_fixed_s p_head = _config.getHeadParameters();
    _head_sp->pushRotateZoom(_lcd_sp,
                            p_head.picinfo.x + (uint16_t)(p_head.picinfo.w/2),
                            p_head.picinfo.y + (uint16_t)(p_head.picinfo.h/2) + _mv.breath,
                            0, 1, 1, _tp_color);

}

void ImageAvatarLite::drawEyes() {
    params_eyes_s p_eyes = _config.getEyesParameters(_expression);
    // 左目の描画
    // 左目はinvertパラメータにより左右反転する。
    float left_eye_invert = 1.0; // invert false
    if (p_eyes.invert) {
        left_eye_invert = -1.0; // invert true
    }
    if (_mv.eye_l_ratio < p_eyes.min_scaleY) {
        _eye_cl_sp->pushRotateZoom(_eye_l_sp,
            p_eyes.left.w / 2, p_eyes.left.h / 2,
            0, 1.0, 1.0);
    } else {
        _eye_op_sp->pushRotateZoom(_eye_l_sp,
            p_eyes.left.w / 2  , p_eyes.left.h / 2,
            0, 1.0, _mv.eye_l_ratio);
    }
    _eye_l_sp->pushRotateZoom(_lcd_sp,
                             p_eyes.left.x, p_eyes.left.y + _mv.breath,
                             0, left_eye_invert, 1.0, _tp_color);

    // 右目の描画
    if (_mv.eye_r_ratio < p_eyes.min_scaleY) {
        _eye_cl_sp->pushRotateZoom(_eye_r_sp,
            p_eyes.right.w / 2, p_eyes.right.h / 2,
            0, 1.0, 1.0);
    } else {
        _eye_op_sp->pushRotateZoom(_eye_r_sp,
            p_eyes.right.w / 2, p_eyes.right.h / 2,
            0, 1.0, _mv.eye_r_ratio);

    }
    _eye_r_sp->pushRotateZoom(_lcd_sp,
                             p_eyes.right.x, p_eyes.right.y + _mv.breath,
                             0, 1.0, 1.0, _tp_color);
}

void ImageAvatarLite::drawMouth() {
    params_mouth_s p_mouth = _config.getMouthParameters(_expression);
    if (_mv.mouth_ratio <= p_mouth.min_scaleY) {
        _mouth_cl_sp->pushRotateZoom(_lcd_sp,
                                    p_mouth.picinfo.x,
                                    p_mouth.picinfo.y + _mv.breath,
                                    0, 1.0, 1.0, _tp_color);
    } else {
        _mouth_op_sp->pushRotateZoom(_lcd_sp,
                                    p_mouth.picinfo.x,
                                    p_mouth.picinfo.y + _mv.breath,
                                    0, 1.0, _mv.mouth_ratio, _tp_color);
    }
}

void ImageAvatarLite::deleteSprites() {
    _lcd_sp->deleteSprite();
    _head_sp->deleteSprite();
    _eye_op_sp->deleteSprite();
    _eye_cl_sp->deleteSprite();
    _mouth_op_sp->deleteSprite();
    _mouth_cl_sp->deleteSprite();
    _mouth_sp->deleteSprite();
    _eye_l_sp->deleteSprite();
    _eye_r_sp->deleteSprite();
}

void ImageAvatarLite::drawAll() {
    drawHead();
    drawEyes();
    drawMouth();
    execDraw();
}

void ImageAvatarLite::drawTest() {
    drawHead();
    drawEyes();
    drawMouth();
    execDraw();
}

void ImageAvatarLite::addTask(TaskFunction_t f, const char* task_name, uint8_t task_priority, uint16_t stack_size) {
    DriveContext * ctx = new DriveContext(this);
    xTaskCreateUniversal(f,
                         task_name,
                         stack_size,
                         ctx,
                         task_priority,
                         NULL,
                         APP_CPU_NUM);
}

void ImageAvatarLite::start() {
    DriveContext *ctx = new DriveContext(this);
    xTaskCreateUniversal(drawLoop,
                         "drawLoop",
                         4096,
                         ctx,
                         2,
                         &drawTaskHandle,
                         APP_CPU_NUM); //tskNO_AFFINITY); // Core 1を指定しないと不安定
//    xTaskCreateUniversal(breath,
                         //"breath",
                         //2048,
                         //ctx,
                         //6,
                         //&breathTaskHandle,
                         //APP_CPU_NUM);
    xTaskCreateUniversal(blink,
                         "blink",
                         2048,
                         ctx,
                         7,
                         &blinkTaskHandle,
                         APP_CPU_NUM);

}

void ImageAvatarLite::changeAvatar(const char* filename, uint8_t expression) {
    vTaskSuspend(blinkTaskHandle); // blinkを先に止めないと画像が乱れる。
    vTaskSuspend(drawTaskHandle);
    _mv.eye_l_ratio = 1.0f;
    _mv.eye_r_ratio = 1.0f;
    _filename = filename;
    _expression = expression;
    initSprites(true);
    vTaskResume(drawTaskHandle);
    vTaskResume(blinkTaskHandle);
}

void ImageAvatarLite::setBreath(float f) {
    _mv.breath = f;
}
void ImageAvatarLite::setBlink(float ratio) {
    setBlink(ratio, RIGHT);
    setBlink(ratio, LEFT);
}

void ImageAvatarLite::setBlink(float ratio, bool is_right) {
    if (is_right) {
        _mv.eye_r_ratio = ratio;
    } else {
        _mv.eye_l_ratio = ratio;
    }
}

void ImageAvatarLite::setExpression(const char* filename, uint8_t expression) {
    if (_expression == expression) return;
    vTaskSuspend(blinkTaskHandle); // blinkを先に止めないと画像が乱れる。
    vTaskSuspend(drawTaskHandle);
    _mv.eye_l_ratio = 1.0f;
    _mv.eye_r_ratio = 1.0f;
    _filename = filename;
    _expression = expression;
    initSprites(true);
    vTaskResume(drawTaskHandle);
    vTaskResume(blinkTaskHandle);
    if (_expression == expression) return;
}

void ImageAvatarLite::setMouthOpen(float ratio) {
    _mv.mouth_ratio = ratio;
}

uint8_t ImageAvatarLite::getExpressionMax() {
    return _config.getExpressionMax();
}

} // namespace m5imageavatar