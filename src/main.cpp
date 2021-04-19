#include <Arduino.h>
#include <lgfx.h>
#include <ESP32-Chimera-Core.h>
#include "M5ImageAvatarLite.h"

// デバッグしたいときは下記の１行コメントアウトしてください。
// #define DEBUG


static LGFX gfx;

// JSONファイルとBMPファイルを置く場所を切り替え
// 開発時はSPIFFS上に置いてUploadするとSDカードを抜き差しする手間が省けます。
fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
fs::FS bmp_fs  = SD; // BMPファイルの収納場所(SPIFFS or SD)

const char* json_file = "/json/M5AvatarConfig.json";
ImageAvatarLite avatar(json_fs, bmp_fs);

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
void startThreads() {
  printDebug("----- startThreads -----");
  if (xMutex != NULL) {
    xTaskCreateUniversal(drawLoop,
                         "drawLoop",
                         4096,
                         NULL,
                         5,
                         &drawTaskHandle,
                         1);// tskNO_AFFINITY); // Core 1を指定しないと不安定
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
  }
}

void setup() {
  gfx.init();
  M5.begin(true, true, true, false, false);
  xMutex = xSemaphoreCreateMutex();
  gfx.fillScreen(TFT_BLACK);
  gfx.println("HelloWorld");

  SPIFFS.begin();
  avatar.init(&gfx, json_file, false, 0);
  startThreads();
}

void loop() {
  M5.update();
  printFreeHeap();
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
