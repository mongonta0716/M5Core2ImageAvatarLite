#include "ImageAvatarConfig.h"

ImageAvatarConfig::ImageAvatarConfig() {

}
ImageAvatarConfig::~ImageAvatarConfig(){

}

void ImageAvatarConfig::loadConfig(fs::FS& fs, const char *filename) {
    File file = fs.open(filename);
    int res = file.available();
    Serial.printf("file:available:%d\n", res);
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.printf("json file read error: %s", filename);
    }

    setCommonParameters(doc);
    setHeadParameters(doc);
    setMouthParameters(doc);
    setEyesParameters(doc);
    setInitParameters(doc);

    file.close();
#ifdef DEBUG
     printAllParameters();
#endif
}

void ImageAvatarConfig::setCommonParameters(DynamicJsonDocument doc) {
     // SetParameters
    JsonArray expression = doc["expression"];
    _max_expresssion = expression.size();
    for (int i=0; i<_max_expresssion; i++) {
        _expression[i] = expression[i];
    }

    JsonObject sprite_info = doc["sprite_info"];
    _spcommon.psram = sprite_info["psram"];
    _spcommon.color_depth = sprite_info["color_depth"];
    _spcommon.swap_bytes = sprite_info["swap_bytes"];

    JsonObject color_info = doc["color_info"];
    const char* char_temp = color_info["skin"];
    sscanf(char_temp, "%x", &_spcommon.colors.skin);
    char_temp = color_info["eye_white"];
    sscanf(char_temp, "%x", &_spcommon.colors.eye_white);
    char_temp = color_info["transparent"];
    sscanf(char_temp, "%x", &_spcommon.colors.transparent);

   
}

void ImageAvatarConfig::setHeadParameters(DynamicJsonDocument doc) {
    JsonObject fixed_parts_0 = doc["fixed_parts"][0];
    _head.picinfo.x = fixed_parts_0["x"];
    _head.picinfo.y = fixed_parts_0["y"];
    _head.picinfo.w = fixed_parts_0["w"];
    _head.picinfo.h = fixed_parts_0["h"];
    _head.filename = fixed_parts_0["filename"];
}
void ImageAvatarConfig::setMouthParameters(DynamicJsonDocument doc) {
    for (int i=0; i<_max_expresssion; i++) {
        JsonObject elem = doc["mouth"][i][_expression[i]];
        if (!elem.isNull()){
            _mouth[i].picinfo.x = elem["x"];
            _mouth[i].picinfo.y = elem["y"];
            _mouth[i].picinfo.w = elem["w"];
            _mouth[i].picinfo.h = elem["h"];
            _mouth[i].filename_op = elem["filename"]["open"];
            _mouth[i].filename_cl = elem["filename"]["close"];
            _mouth[i].min_scaleX = elem["minScaleX"];
            _mouth[i].max_scaleX = elem["maxScaleX"];
            _mouth[i].min_scaleY = elem["minScaleY"];
            _mouth[i].max_scaleY = elem["maxScaleY"];
        }
    }
}
void ImageAvatarConfig::setEyesParameters(DynamicJsonDocument doc) {
    for (int i=0; i<_max_expresssion; i++) {
        JsonObject elem = doc["eye"][i][_expression[i]];
        if (!elem.isNull()){
            _eyes[i].left.x = elem["lx"];
            _eyes[i].left.y = elem["ly"];
            _eyes[i].left.w = elem["w"];
            _eyes[i].left.h = elem["h"];
            _eyes[i].right.x = elem["rx"];
            _eyes[i].right.y = elem["ry"];
            _eyes[i].right.w = elem["w"];
            _eyes[i].right.h = elem["h"];
            _eyes[i].filename_op = elem["filename"]["open"];
            _eyes[i].filename_cl = elem["filename"]["close"];
            _eyes[i].min_scaleX = elem["minScaleX"];
            _eyes[i].max_scaleX = elem["maxScaleX"];
            _eyes[i].min_scaleY = elem["minScaleY"];
            _eyes[i].max_scaleY = elem["maxScaleY"];
        }
    }
}

void ImageAvatarConfig::setInitParameters(DynamicJsonDocument doc) {
     for (int i=0; i<_max_expresssion; i++) {
        JsonObject elem = doc["init_param"][i][_expression[i]];
        if (!elem.isNull()){
            _mv[i].eye_l_ratio = elem["eye_l_ratio"];
            _mv[i].eye_r_ratio = elem["eye_r_ratio"];
            _mv[i].mouth_ratio = elem["mouth_ratio"];
            _mv[i].angle = elem["angle"];
            _mv[i].breath = elem["breath"];
        }
    }   
}
spcommon_s ImageAvatarConfig::getSpriteCommonParameters() {
    return _spcommon;
}

params_fixed_s ImageAvatarConfig::getHeadParameters() {
    return _head;
}

params_mouth_s ImageAvatarConfig::getMouthParameters(uint8_t expression) {
    return _mouth[expression];
}

params_eyes_s ImageAvatarConfig::getEyesParameters(uint8_t expression) {
    return _eyes[expression];
}

move_param_s ImageAvatarConfig::getMoveParameters(uint8_t expression) {
    return _mv[expression];
}

void ImageAvatarConfig::printAllParameters() {
    Serial.println("ImageAvatarConfig:printAllParameters");
    for (int i=0; i<_max_expresssion; i++) {
        Serial.println(_expression[i]);
        Serial.println(_eyes[i].filename_op);
        Serial.println(_eyes[i].filename_cl);
        Serial.println(_mouth[i].filename_op);
        Serial.println(_mouth[i].filename_cl);
    }
    Serial.printf("sprite_info\n");
    Serial.printf("psram: %d\n", _spcommon.psram);
    Serial.printf("color_depth: %d\n", _spcommon.color_depth);
    Serial.printf("swap_bytes: %d\n", _spcommon.swap_bytes);
    Serial.printf("skin_color: %d\n", _spcommon.colors.skin);
    Serial.printf("eye_white: %d\n", _spcommon.colors.eye_white);
    Serial.printf("transparent: %d\n", _spcommon.colors.transparent);
}
