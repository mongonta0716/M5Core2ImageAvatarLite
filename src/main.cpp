#include <ESP32-Chimera-Core.h>

#define SDU_APP_PATH "/M5Core2AvatarLite.bin" // title for SD-Updater UI
#define SDU_APP_NAME "Image Avater Lite" // title for SD-Updater UI
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/

#include "M5ImageAvatarLite.h"
#include "ImageAvatarServo.h"

// サーボを利用しない場合は下記の1行をコメントにしてください。
#define USE_SERVO

// デバッグしたいときは下記の１行コメントアウトしてください。
//#define DEBUG

LGFX &gfx( M5.Lcd ); // aliasing is better than spawning two instances of LGFX

// JSONファイルとBMPファイルを置く場所を切り替え
// 開発時はSPIFFS上に置いてUploadするとSDカードを抜き差しする手間が省けます。
fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
fs::FS bmp_fs  = SD; // BMPファイルの収納場所(SPIFFS or SD)

const char* avatar_json = "/json/M5AvatarLiteConfig.json";
const char* servo_json = "/json/M5AvatarLiteServoConfig.json"; 
ImageAvatarLite avatar(json_fs, bmp_fs);
#ifdef USE_SERVO
  ImageAvatarServo servo(json_fs, servo_json);
  bool servo_enable = false; // サーボを動かすかどうか
  TaskHandle_t servoloopTaskHangle;
#endif

uint8_t expression = 0;

// Multi Threads Settings
TaskHandle_t drawTaskHandle;
TaskHandle_t blinkTaskHandle;
TaskHandle_t breathTaskHandle;
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
void drawLoop(void *args) {
  BaseType_t xStatus;
  const TickType_t xTicksToWait = 1000UL;
  xSemaphoreGive(xMutex);
  for(;;) {
    xStatus = xSemaphoreTake(xMutex, xTicksToWait);
    if (xStatus == pdTRUE) {
        avatar.drawTest();
    }
    xSemaphoreGive(xMutex);
    vTaskDelay(33);
  }
}

void breath(void *args) {
  uint32_t c = 0;
  for(;;) {
    c = c + 1 % 100;
    float f = sin(c) * 2;
    avatar.setBreath(f);
    vTaskDelay(1000);
  }
}
void blink(void *args) {
  for(;;) {
    // まぶたの動きをリアルにしたいのでfor文を使用
    for(float f=0.0; f<=1; f=f+0.1) {
        avatar.setBlink(f);
        delay(10);
    }
    vTaskDelay(2000 + 100 * random(20));
    for(float f=1.0; f>=0; f=f-0.1) {
        avatar.setBlink(f);
        delay(10);
    }
    vTaskDelay(300 + 10 * random(20));
  }
}
void lipsync(void *args) {
  for(;;) {
      for(float f=0.0; f<=1.0; f=f+0.1) {
        avatar.setMouthOpen(f);
        vTaskDelay(200);
      }
      vTaskDelay(500);
      for(float f=1.0; f>=0.0; f=f-0.1) {
        avatar.setMouthOpen(f);
        vTaskDelay(200);
      }
  }
}

#ifdef USE_SERVO
void servoloop(void *args) {
  
  vTaskDelay(33);
}
#endif

void startThreads() {
  printDebug("----- startThreads -----");
  if (xMutex != NULL) {
    xTaskCreateUniversal(drawLoop,
                         "drawLoop",
                         4096,
                         NULL,
                         5,
                         &drawTaskHandle,
                         1); //tskNO_AFFINITY); // Core 1を指定しないと不安定
    xTaskCreateUniversal(breath,
                         "breath",
                         2048,
                         NULL,
                         6,
                         &breathTaskHandle,
                         tskNO_AFFINITY);
    xTaskCreateUniversal(blink,
                         "blink",
                         2048,
                         NULL,
                         7,
                         &blinkTaskHandle,
                         tskNO_AFFINITY);
    xTaskCreateUniversal(lipsync,
                         "lipsync",
                         2048,
                         NULL,
                         8,
                         &lipsyncTaskHandle,
                         tskNO_AFFINITY);
#ifdef USE_SERVO
    Serial.println("----- servo init");
    servo.init();
    servo.attachAll();
    servo.check();
    Serial.println("----- servo checked");

//    xTaskCreateUniversal(servoloop,
                         //"servoloop",
                         //4096,
                         //NULL,
                         //9,
                         //&servoloopTaskHangle,
                         //tskNO_AFFINITY);
    // サーボの動きはservo_enableで管理
//    if (servo_enable) {
      //vTaskResume(servoloopTaskHangle);
    //} else {
      //vTaskSuspend(servoloopTaskHangle);
    //}
#endif
  }
}

void setup() {
  M5.begin(true, true, true, false, false);
  // checkSDUpdater( SD, MENU_BIN, 5000, TFCARD_CS_PIN ); // Filesystem, Launcher bin path, Wait delay
  xMutex = xSemaphoreCreateMutex();
  SPIFFS.begin();

  avatar.init(&gfx, avatar_json, false, 0);
  startThreads();

#ifdef USE_SERVO

#endif
}

void loop() {
  M5.update();
  printFreeHeap();
#ifdef USE_SERVO
  if (M5.BtnA.wasPressed()) {
    servo.check();
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
    vTaskSuspend(drawTaskHandle);
    Serial.printf("----------Expression: %d----------\n", expression);
    avatar.setExpression(expression);
    vTaskResume(drawTaskHandle);
    Serial.printf("Resume\n");
  }
  vTaskDelay(100);
}
