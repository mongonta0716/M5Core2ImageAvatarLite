# M5Core2ImageAvatarLite
 ImageAvatarLite for M5Stack Core2 and M5Stack Fire

# 概要

　あらかじめ用意した画像ファイル（BMP)とJSONファイルの設定を組み合わせてAvatarを作成できるアプリです。
[![M5Core2ImageAvatarLite](https://img.youtube.com/vi/gR_Rzfq-Dh8/0.jpg)](https://www.youtube.com/watch?v=gR_Rzfq-Dh8)
# 開発環境
- VSCode(Ver.1.55.1)
- PlatformIO

## ArduinoIDEで使用する場合
srcフォルダとmain.cppの名前を揃えて変更してください。

### 変更例
src -> M5Core2ImageAvatarLite<br>
main.cpp -> M5Core2ImageAvatarLite.ino

# 必要なライブラリ
<b>Arduino-esp32はVer.2.0.3で動作確認しています。</b>

バージョンについては[platformio.ini](platformio.ini)を見てください。
- [M5Unified](https://github.com/m5stack/M5Unified)
- [ArduinoJSON](https://github.com/bblanchon/ArduinoJson)

## サーボを使う場合
- [ServoEasing](https://github.com/arminjo/ServoEasing)
- [ESPServo](https://github.com/madhephaestus/ESP32Servo)

## LEDを使う場合
- [FastLED](https://github.com/FastLED/FastLED)

# 対応機種
 メモリの都合上PSRAMが必要なのでM5Stack Core2とM5Stack Fireのみを対象にしています。
 4bitBMPを使用し、カラーパレットを使用することにより他の機種でも動きますが手順が複雑なのでCore2及びFireのみとします。

# 使い方
1. SDカードのルートにdataにあるフォルダ(bmp,json)をコピー
1. jsonフォルダの下記の2ファイルを設定します。
    - M5AvatarLiteSystem.json
    - M5AvatarLiteServo.json
1. プログラムを書き込むとAvatarが起動します。
1. Bluetoothスピーカーとして機能します。「ESP32」というデバイスをペアリングすると音を再生可能です。

## ボタン操作

- ボタンA
    - クリック<br>アバターの切り替え
    - 長押し<br>サーボテスト
- ボタンB
    - クリック<br>ボリュームアップ
    - 長押し<br>サーボ駆動のON/OFF切り替え
- ボタンC
    - クリック<br>ボリュームダウン
    - 長押し<br>アバターの表情切り替え

## SDカード上に必要なファイル

dataフォルダ内にあるファイル及びフォルダをSDカードのルートにコピーしてください。
 
 1. - /bmp_slime/<br>BMPファイル(サンプルのbmp_slimeでは全部で11ファイル)
    - /bmp_puipui/<br>
    - /bmp_jacko/<br>
 1. /json/<br>
    - M5AvatarLiteSystem.json<br>一番最初に読み込まれる設定ファイル
    - M5AvatarLite00.json<br>slimeの設定ファイル
    - M5AvatarLite01.json<br>puipuiの設定ファイル
    - M5AvatarLite02.json<br>jackolanternの設定ファイル
    - M5AvatarLiteServo.json(※サーボを使う場合)

# JSONファイルとBMPファイルの置き場所について
 main.cppの下記の行を変更するとJSONファイルとBMPファイルの収納場所をSDかSPIFFSか指定できます。SPIFFSに置くと開発するときにVSCodeからUploadできるようになり、SDカードを抜き差しして書き換える手間が省けます。
```
fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
fs::FS bmp_fs  = SD; // BMPファイルの収納場所(SPIFFS or SD)
```
 ## VSCodeからのデータUpload方法（英語）
 [ESP32 with VS Code and PlatformIO: Upload Files to Filesystem (SPIFFS)](https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/)

# M5AvatarLiteSystem.jsonの内容
一番最初に読み込まれる設定ファイルです。サンプルでは3つまで定義してあります。（最大8つ）
```
{
    "volume" : 100,                                // 起動時のボリューム
    "lcd_brightness" : 50,                         // LCDの明るさ
    "avatar_json": {
        "filename" : [
            "/json/M5AvatarLite00.json",           // countで設定した数に対応するAvatar定義を作成
            "/json/M5AvatarLite01.json",
            "/json/M5AvatarLite02.json"            // 3つ以上増やすときは末尾に「,」を追加
            // .... ,
            // .
            // "/json/M5AvatarLite07.json"         // 最大8まで
        ]
    },
    "bluetooth_device_name" : "ESP32",                   // Bluetoothスピーカーのデバイス名
    "bluetooth_reconnect" : false,                       // 起動時にBluetoothを再接続するかどうか（接続先が変わる場合はfalse推奨）
    "servo_json" : "/json/M5AvatarLiteServo.json",       // サーボの設定ファイル
    "servo_random_mode" : true,                          // 起動時にサーボを動かすかどうか
    "auto_power_off_time" : 0,                           // USBからの電源供給が止まった後にCore2の電源を切るまでの時間（0は電源OFFしません。）
     "led_lr"                                             // GoBottomのLEDBarをステレオで表示するかの切り替え（0:Stereo, 1:LeftOnly, 2:RightOnly）
}
```

 # カスタマイズ方法
 自分で24bitか16bitのBMPファイルを用意すると好きな画像をAvatar化することが可能です。

 ## 用意する画像ファイル
 サンプルの画像ファイルは全て24bitBMPファイルでWindows標準のペイントソフトを利用しています。
 ### 固定パーツ（頭の画像ファイル）
 1. 頭用BMP<br>背景となる画像ファイル320x240もしくは傾けるのであれば少し余裕が必要です。
 ![image](data/bmp_slime/head.bmp)
 ### 表情で変わるパーツ(右目と口)
 開いた状態と閉じた状態の<b>２種類×表情の数</b>だけ必要です。(同じパーツを流用も可能)
 1. 開いた目と閉じた「右目」のパーツ（左目は右目を反転させて表示します。）<br>サンプルではnormal,sad,angryの3種類用意してあります。<br>
 ![image](data/bmp_slime/eye_op_normal.bmp) ![image](data/bmp_slime/eye_cl_normal.bmp)
 1. 開いた口と閉じた口のパーツ<br>サンプルでは開いた口normal,sad,angryの3種類と閉じた口は共通パーツとして用意してあります。<br>
 ![image](data/bmp_slime/mouth_op_normal.bmp) ![image](data/bmp_slime/mouth_cl_normal.bmp)

目と口の透明化したい部分は透明色(M5AvatarLiteConfig.json)で塗りつぶします。サンプルでは緑（0x00FF00）になっています。

## JSONファイルの編集
下記を参考にして、JSONファイルを書き換えてください。
``` 
{
    "expression": [  // 表情（デフォルトは最大8パターンまで）
        "normal",
        "sad",
        "angry"
    ],

    "sprite_info": {
        "psram": "true",      // PSRAMの仕様有無（8bit以上ではtrueのまま）
        "color_depth": 16,    // 使用するBMPのカラー(4,8,16)
        "swap_bytes": 0
    },
    "color_info": {
        "skin"  : "0xFF5B00",             // 未使用
        "eye_white" : "0xFFFFFF",         // 未使用
        "transparent"    : "0x00FF00",    // 透明色
    },
    "fixed_parts": [
        {
            "parts": "head",
            "x": -10,                             // 頭部パーツの開始X座標
            "y": -10,                             // 頭部パーツの開始y座標
            "w": 340,                             // 頭部パーツの幅
            "h": 260,                             // 頭部パーツの高さ
            "filename": "/bmp_slime/head.bmp"        // 頭部パーツのファイル名
        }
    ],
    "mouth": [                                    // 口のパーツ設定
        {
            "normal": {     // 表情"normal"の口設定
                "x": 160,   // 口パーツの開始x座標
                "y": 200,   // 口パーツの開始y座標
                "w": 60,    // 口パーツの幅
                "h": 60,    // 口パーツの高さ
                "filename": {
                    "open": "/bmp_slime/mouth_op_normal.bmp",  // 開いた口
                    "close": "/bmp_slime/mouth_cl_normal.bmp"  // 閉じた口
                },
                "minScaleX": 1.0,     // 未使用
                "maxScaleX": 1.0,     // 未使用
                "minScaleY": 0.3,     // Y軸の拡大率がこれより小さくなると閉じる
                "maxScaleY": 1.0      // 未使用
            }
        },
        {
            "sad": {                              // 表情"sad"の口設定
                "x": 160,
                "y": 200,
                "w": 60,
                "h": 60,
                "filename": {
                    "open": "/bmp_slime/mouth_op_sad.bmp",
                    "close": "/bmp_slime/mouth_cl_normal.bmp"
                },
                "minScaleX": 1.0,
                "maxScaleX": 1.0,
                "minScaleY": 0.3,
                "maxScaleY": 1.0
            }
        },
        {
            "angry": {                            // 表情"angry"の口設定
                "x": 160,
                "y": 200,
                "w": 60,
                "h": 60,
                "filename": {
                    "open": "/bmp_slime/mouth_op_angry.bmp",
                    "close": "/bmp_slime/mouth_cl_normal.bmp"
                },
                "minScaleX": 1.0,
                "maxScaleX": 1.0,
                "minScaleY": 0.3,
                "maxScaleY": 1.0
            }
        }
    ],
    "eye": [ // -------------------------------- 目の設定
        {
            "normal": {        // 表情"normal"の目設定
                "rx": 120,     //       右目開始X座標
                "ry": 100,     //       右目開始Y座標
                "lx": 200,     //       左目開始X座標
                "ly": 100,     //       左目開始Y座標
                "w": 40,       //       目の幅
                "h": 60,       //       目の高さ      
                "filename": {
                    "open": "/bmp_slime/eye_op_normal.bmp", // 開いた目のファイル名
                    "close": "/bmp_slime/eye_cl_normal.bmp" // 閉じた目のファイル名
                },
                "minScaleX": 1.0,     // 未使用
                "maxScaleX": 1.0,     // 未使用
                "minScaleY": 0.3,     // 拡大倍率がこれより小さくなると閉じる
                "maxScaleY": 1.0,     // 未使用
                "invert"   : true     // 左目表示の際に反転するかどうかを指定(true: 反転する。false: 反転しない)
            }
        },
        {
            "sad": {           // 表情"sad"の目設定
                "rx": 120,
                "ry": 100,
                "lx": 200,
                "ly": 100,
                "w": 40,
                "h": 60,
                "filename": {
                    "open": "/bmp_slime/eye_op_sad.bmp",
                    "close": "/bmp_slime/eye_cl_sad.bmp"
                },
                "minScaleX": 1.0,
                "maxScaleX": 1.0,
                "minScaleY": 0.3,
                "maxScaleY": 1.0,
                "invert"   : false
            }
        },
        {
            "angry": {         // 表情"angry"の目設定
                "rx": 120,
                "ry": 100,
                "lx": 200,
                "ly": 100,
                "w": 40,
                "h": 60,
                "filename": {
                    "open": "/bmp_slime/eye_op_angry.bmp",
                    "close": "/bmp_slime/eye_cl_angry.bmp"
                },
                "minScaleX": 1.0,
                "maxScaleX": 1.0,
                "minScaleY": 0.3,
                "maxScaleY": 1.0,
                "invert"   : true
            }
        }
    ],
    "init_param": [                   // 初期設定（特に変更は必要ないですが表情の数必要です。）
        {
            "normal": {
                "eye_l_ratio": 0.0,  // 左目を開く倍率
                "eye_r_ratio": 0.0,  // 右目を開く倍率
                "mouth_ratio": 0.0,  // 口を開く倍率
                "angle": 0.0,        // Avatarの角度
                "breath": 0          // Avatarが息使いする高さ
            }
        },
        {
            "sad": {
                "eye_l_ratio": 0.0,
                "eye_r_ratio": 0.0,
                "mouth_ratio": 0.0,
                "angle": 0.0,
                "breath": 0
            }
        },
        {
            "angry": {
                "eye_l_ratio": 0.0,
                "eye_r_ratio": 0.0,
                "mouth_ratio": 0.0,
                "angle": 0.0,
                "breath": 0
            }
        }
    ]
}
```



# サーボについて
main.cpp冒頭の#define USE_SERVOのコメントを外すとサーボを利用できます。2021/11現在では[ｽﾀｯｸﾁｬﾝ](https://github.com/meganetaaan/stack-chan)での利用を想定しています。

## サーボの初期設定
X軸とY軸の2軸（ｽﾀｯｸﾁｬﾝのパン(x)とチルト(y)）で利用できます。/json/フォルダにM5AvatarLiteServo.jsonを置いてください。

[![M5Core2ImageAvatarLite](https://img.youtube.com/vi/07fEke_r3Xc/0.jpg)](https://www.youtube.com/watch?v=07fEke_r3Xc)


## M5AvatarLiteServoConfig.jsonの内容
```
{
    "initial_settings": {
        "x_axis": {            // パン
            "pin"    : 13,     // 水平方向のサーボピン番号
            "center" : 85,     // サーボの中心（初期位置）
            "lower"  : 0,      // サーボの下限角度
            "uppder"  : 180,   // サーボの上限角度
            "offset" : 0       // サーボのオフセット
        },
        "y_axis": {            // チルト
            "pin"    : 14,     // 垂直方向のサーボ品番号
            "center" : 60,     // サーボの中心(初期位置)
            "lower"  : 30,     // サーボの下限角度
            "upper"  : 90,     // サーボの上限角度
            "offset" : 0       // サーボのオフセット
        }

    },
    "servo_enable" : "false"   // 起動時にサーボを動かすかどうかのフラグ（未使用）
}
```

# 参考にしたリポジトリ
- [m5stack-avatar](https://github.com/meganetaaan/m5stack-avatar)
- [M5Stack_WebRadio_Avator](https://github.com/robo8080/M5Stack_WebRadio_Avator)

# 謝辞
このソフトを作成するにあたり、動きや構造の元となった[M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)を作成・公開してくださった[meganetaaan](https://github.com/meganetaaan)氏に感謝いたします。

ImageAvatarを実現するにあたり優れたパフォーマンス、機能を持ったLovyanGFX,M5Unifiedの作者[lovyan03](https://github.com/lovyan03)氏に感謝いたします。

ImageAvatar作成するにあたり、 初期の頃からたくさんのアドバイスを頂き、参考にさせていただいた[M5Stack_WebRadio_Avatar](https://github.com/robo8080/M5Stack_WebRadio_Avator)の作者[robo8080](https://github.com/robo8080)氏に感謝いたします。

色々なアドバイスを頂いた[tobozo](https;//github.com/tobozo)氏に感謝いたします。

# Credit
- [meganetaaan](https://github.com/meganetaaan)
- [lovyan03](https://github.com/lovyan03/LovyanGFX)
- [robo8080](https://github.com/robo8080)
- [tobozo](https://github.com/tobozo)

# LICENSE
[MIT](LICENSE)

# Author
[Takao Akaki](https://github.com/mongonta0716)
