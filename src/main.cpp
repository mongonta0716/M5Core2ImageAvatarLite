#include <M5Unified.h>

#define SDU_APP_PATH "/M5Core2AvatarLite.bin" // title for SD-Updater UI
#define SDU_APP_NAME "Image Avater Lite" // title for SD-Updater UI
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/

#include "M5ImageAvatarLite.h"

// サーボを利用しない場合は下記の1行をコメントにしてください。
#define USE_SERVO

// デバッグしたいときは下記の１行コメントアウトしてください。
//#define DEBUG

M5GFX &gfx( M5.Lcd ); // aliasing is better than spawning two instances of LGFX

// JSONファイルとBMPファイルを置く場所を切り替え
// 開発時はSPIFFS上に置いてUploadするとSDカードを抜き差しする手間が省けます。
fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
fs::FS bmp_fs  = SD; // BMPファイルの収納場所(SPIFFS or SD)

using namespace m5imageavatar;
const char* avatar_json[] = { "/json/M5AvatarLite00.json",  // ファイル名は32バイトを超えると不具合が起きる場合あり。
                              "/json/M5AvatarLite01.json",
                              "/json/M5AvatarLite02.json" 
                            }; //
uint8_t avatar_count_max = 3;
uint8_t avatar_count = 0;
ImageAvatarLite avatar(json_fs, bmp_fs);
#ifdef USE_SERVO
  const char* servo_json = "/json/M5AvatarLiteServo.json"; // ファイル名は32バイトを超えると不具合が起きる場合あり。
  #include "ImageAvatarServo.h"
  ImageAvatarServo servo(json_fs, servo_json);
  bool servo_enable = true; // サーボを動かすかどうか
  TaskHandle_t servoloopTaskHangle;
#endif

#include "BluetoothA2DPSink_M5Speaker.hpp"
#define LIPSYNC_LEVEL_MAX 10.0f
static float lipsync_level_max = LIPSYNC_LEVEL_MAX;
uint8_t expression = 0;
float mouth_ratio = 0.0f;
bool sing_happy = true;

// ----- あまり間隔を短くしすぎるとサーボが壊れやすくなるので注意(単位:msec)
static long interval_min      = 1000;        // 待機時インターバル最小
static long interval_max      = 10000;       // 待機時インターバル最大
static long interval_move_min = 500;         // 待機時のサーボ移動時間最小
static long interval_move_max = 1500;        // 待機時のサーボ移動時間最大
static long sing_interval_min = 500;         // 歌うモードのインターバル最小
static long sing_interval_max = 1500;        // 歌うモードのインターバル最大
static long sing_move_min     = 500;         // 歌うモードのサーボ移動時間最小
static long sing_move_max     = 1500;        // 歌うモードのサーボ移動時間最大
// サーボ関連の設定 end
// --------------------

// --------------------
// Bluetoothのデバイス名
/// set ESP32-A2DP device name
static constexpr char bt_device_name[] = "ESP32";
// --------------------

/// set M5Speaker virtual channel (0-7)
static constexpr uint8_t m5spk_virtual_channel = 0;

static BluetoothA2DPSink_M5Speaker a2dp_sink = { &M5.Speaker, m5spk_virtual_channel };
static fft_t fft;
static constexpr size_t WAVE_SIZE = 320;
static int16_t raw_data[WAVE_SIZE * 2];

// Multi Threads Settings
TaskHandle_t lipsyncTaskHandle;
SemaphoreHandle_t xMutex = NULL;

void printDebug(const char *str) {
#ifdef DEBUG
  Serial.println(str);
#ifdef USE_WIFI
  uint8_t buf[BUFFER_LEN];
  memcpy(buf, str, BUFFER_LEN);
  peerClients();
  sendData(buf);
#endif
#endif
}

void printFreeHeap() {
    char buf[250];
    sprintf(buf, "Free Heap Size = %d\n", esp_get_free_heap_size());
    printDebug(buf);
    sprintf(buf, "Total PSRAM Size = %d\n", ESP.getPsramSize());
    printDebug(buf);
    sprintf(buf, "Free PSRAM Size = %d\n", ESP.getFreePsram());
    printDebug(buf);

}

// Start----- Task functions ----------
void lipsync(void *args) {
  // スレッド内でログを出そうとすると不具合が起きる場合があります。
  DriveContext * ctx = reinterpret_cast<DriveContext *>(args);
  ImageAvatarLite *avatar = ctx->getAvatar();
  for(;;) {
     uint64_t level = 0;
    auto buf = a2dp_sink.getBuffer();
    if (buf) {
      memcpy(raw_data, buf, WAVE_SIZE * 2 * sizeof(int16_t));
      fft.exec(raw_data);
      for (size_t bx = 5; bx <= 64; ++bx) { // リップシンクで抽出する範囲はここで指定(低音)0〜64（高音）
        int32_t f = fft.get(bx);
        level += abs(f);
        //Serial.printf("bx:%d, f:%d\n", bx, f) ;
      }
      //Serial.printf("level:%d\n", level >> 16);
    }

    //Serial.printf("data=%d\n\r", level >> 16);
    mouth_ratio = (float)(level >> 16)/lipsync_level_max;
    if (mouth_ratio > 1.2f) {
      if (mouth_ratio > 1.5f) {
        lipsync_level_max += 10.0f; // リップシンク上限を大幅に超えるごとに上限を上げていく。
      }
      mouth_ratio = 1.2f;
    }
    avatar->setMouthOpen(mouth_ratio);
    vTaskDelay(1/portTICK_PERIOD_MS);
  }   
}

#ifdef USE_SERVO
void servoloop(void *args) {
  long move_time = 0;
  long interval_time = 0;
  long move_x = 0;
  long move_y = 0;
  bool sing_mode = false;
  for (;;) {
    if (mouth_ratio == 0.0f) {
      // 待機時の動き
      interval_time = random(interval_min, interval_max);
      move_time = random(interval_move_min, interval_move_max);
      lipsync_level_max = LIPSYNC_LEVEL_MAX; // リップシンク上限の初期化
      sing_mode = false;

    } else {
      // 歌うモードの動き
      interval_time = random(sing_interval_min, sing_interval_max);
      move_time = random(sing_move_min, sing_move_max);
      sing_mode = true;
    } 
    
//    Serial.printf("x:%f:y:%f\n", gaze_x, gaze_y);
    // X軸は90°から+-でランダムに左右にスイング
    int direction = random(2);
    if (direction == 0) {
      move_x = 90 - mouth_ratio * 30;
    } else {
      move_x = 90 + mouth_ratio * 30;
    }
    // Y軸は90°から上にスイング（最大35°）
    move_y = 90 - mouth_ratio * 20;
    servo.moveXY(move_x, move_y, move_time, move_time);
    if (sing_mode) {
      // 歌っているときはうなずく
      servo.moveXY(move_x, move_y + 10, 400, 400);
    }
    vTaskDelay(interval_time/portTICK_PERIOD_MS);

  }
}
#endif

void startThreads() {
#ifdef USE_SERVO
  //servo.check();
  delay(2000);
  xTaskCreateUniversal(servoloop,
                        "servoloop",
                        4096,
                        NULL,
                        9,
                        &servoloopTaskHangle,
                        APP_CPU_NUM);
  // サーボの動きはservo_enableで管理
  if (servo_enable) {
    vTaskResume(servoloopTaskHangle);
  } else {
    vTaskSuspend(servoloopTaskHangle);
  }
#endif
}

void hvt_event_callback(int avatar_expression) {
  avatar.setExpression(0);
  //avatar.setExpression(avatar_expression);
}

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2)
{
  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
  if (sing_happy) {
    avatar.setExpression(0);
  } else {
    avatar.setExpression(0);
  }
  sing_happy = !sing_happy;

}

void setup() {
  auto cfg = M5.config();
#ifdef ARDUINO_M5STACK_FIRE
  cfg.internal_imu = false; // サーボの誤動作防止(Fireは21,22を使うので干渉するため)
#endif
  M5.begin(cfg);

  { /// custom setting
    auto spk_cfg = M5.Speaker.config();
    /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
    spk_cfg.sample_rate = 96000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
    spk_cfg.task_pinned_core = PRO_CPU_NUM;//APP_CPU_NUM;
    spk_cfg.task_priority = 1;//configMAX_PRIORITIES - 2;
    spk_cfg.dma_buf_count = 8;
    //spk_cfg.stereo = true;
    spk_cfg.dma_buf_len = 256;
    M5.Speaker.config(spk_cfg);
  }
  M5.Speaker.setChannelVolume(m5spk_virtual_channel, 200);
  //checkSDUpdater( SD, MENU_BIN, 2000, TFCARD_CS_PIN ); // Filesystem, Launcher bin path, Wait delay
  xMutex = xSemaphoreCreateMutex();
  SPIFFS.begin();
  SD.begin(GPIO_NUM_4, SPI, 25000000);
  M5.Lcd.setBrightness(100);

#ifdef USE_SERVO
  // 2022.4.26 ServoConfig.jsonを先に読まないと失敗する。（原因不明）
  Serial.println("----- servo init");
  servo.init();
  servo.attachAll();
  Serial.println("----- servo checked");
#endif
  avatar.init(&gfx, avatar_json[avatar_count], false, 0);
  avatar.start();
  avatar.addTask(lipsync, "lipsync");
  //a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  //a2dp_sink.setHvtEventCallback(hvt_event_callback);
  M5.Speaker.setChannelVolume(m5spk_virtual_channel, 200);
  a2dp_sink.start(bt_device_name, false);
  startThreads();

}

void loop() {
  M5.update();
  printFreeHeap();
#ifdef USE_SERVO
  if (M5.BtnA.wasHold() and !servo_enable) {
    servo.check();
  }
  if (M5.BtnA.wasClicked()) {
    avatar_count++;
    if (avatar_count >= avatar_count_max) {
      avatar_count = 0;
    }
    Serial.printf("Avatar No:%d\n", avatar_count);
    avatar.changeAvatar(avatar_json[avatar_count]);
  }
  if (M5.BtnB.wasPressed()) {
    servo_enable = !servo_enable;
    Serial.printf("BtnB was pressed servo_enable:%d", servo_enable);
    if (servo_enable) {
      vTaskResume(servoloopTaskHangle);
    } else {
      vTaskSuspend(servoloopTaskHangle);
    }
  }
#endif
  if (M5.BtnC.wasPressed()) {
    expression++;
    if (expression > 2) {
      expression = 0;
    }
    Serial.printf("----------Expression: %d----------\n", expression);
    avatar.setExpression(expression);
    Serial.printf("Resume\n");
  }
  vTaskDelay(100);
}
