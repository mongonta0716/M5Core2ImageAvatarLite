# M5Core2ImageAvatarLite
 ImageAvatarLite for M5Stack Core2

# 開発途中です。
ドキュメントも行き届いていません。その点はご了承ください。

# 概要

　あらかじめ用意した画像ファイル（BMP)とJSONファイルの設定を組み合わせてAvatarを作成できるアプリです。
[![M5Core2ImageAvatarLite](https://img.youtube.com/vi/gR_Rzfq-Dh8/0.jpg)](https://www.youtube.com/watch?v=gR_Rzfq-Dh8)
# 開発環境
- VSCode(Ver.1.55.1)
- PlatformIO

# 必要なライブラリ
バージョンについては[platformio.ini](platformio.ini)を見てください。
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)
- [ArduinoJSON](https://github.com/bblanchon/ArduinoJson)
- [ESP32-Chimera-Core](https://github.com/tobozo/ESP32-Chimera-Core)

★ESP32-Chimera-CoreはボタンCの制御しか使っていないので必須ではありません。

# 対応機種
 メモリの都合上PSRAMが必要なのでM5Stack Core2のみを対象にしています。
 4bitBMPを使用し、カラーパレットを使用することにより他の機種でも動きますが手順が複雑なのでCore2のみとします。

# 使い方
1. SDカードのルートにdataにあるフォルダ(bmp,json)をコピー
1. プログラムを書き込むとAvatarが起動します。
1. 口は開閉を繰り返すだけです。BtnCを押すと表情が切り替わります。

## SDカード上に必要なファイル
 /bmp/にはBMPファイル(サンプルでは全部で11ファイル)<br>
 /json/にはM5AvatarConfig.json

# JSONファイルとBMPファイルの置き場所について
 main.cppの下記の行を変更するとJSONファイルとBMPファイルの収納場所をSDかSPIFFSか指定できます。SPIFFSに置くと開発するときにVSCodeからUploadできるようになり、SDカードを抜き差しして書き換える手間が省けます。
```
fs::FS json_fs = SD; // JSONファイルの収納場所(SPIFFS or SD)
fs::FS bmp_fs  = SD; // BMPファイルの収納場所(SPIFFS or SD)
```
 ## VSCodeからのデータUpload方法（英語）
 [ESP32 with VS Code and PlatformIO: Upload Files to Filesystem (SPIFFS)](https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/)

 # カスタマイズ方法
 自分で24bitか16bitのBMPファイルを用意すると好きな画像をAvatar化することが可能です。

 ## 用意する画像ファイル
 サンプルの画像ファイルは全て24bitBMPファイルでWindows標準のペイントソフトを利用しています。
 ### 固定パーツ（頭の画像ファイル）
 1. 頭用BMP<br>背景となる画像ファイル320x240もしくは傾けるのであれば少し余裕が必要です。
 ![image](data/bmp/head.bmp)
 ### 表情で変わるパーツ(右目と口)
 開いた状態と閉じた状態の<b>２種類×表情の数</b>だけ必要です。(同じパーツを流用も可能)
 1. 開いた目と閉じた「右目」のパーツ（左目は右目を反転させて表示します。）<br>サンプルではnormal,sad,angryの3種類用意してあります。<br>
 ![image](data/bmp/eye_op_normal.bmp) ![image](data/bmp/eye_cl_normal.bmp)
 1. 開いた口と閉じた口のパーツ<br>サンプルでは開いた口normal,sad,angryの3種類と閉じた口は共通パーツとして用意してあります。<br>
 ![image](data/bmp/mouth_op_normal.bmp) ![image](data/bmp/mouth_cl_normal.bmp)

目と口の透明化したい部分は透明色(M5ImageAvatar.json)で塗りつぶします。サンプルでは緑（0x00FF00）になっています。

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
            "filename": "/bmp/head.bmp"        // 頭部パーツのファイル名
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
                    "open": "/bmp/mouth_op_normal.bmp",  // 開いた口
                    "close": "/bmp/mouth_cl_normal.bmp"  // 閉じた口
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
                    "open": "/bmp/mouth_op_sad.bmp",
                    "close": "/bmp/mouth_cl_normal.bmp"
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
                    "open": "/bmp/mouth_op_angry.bmp",
                    "close": "/bmp/mouth_cl_normal.bmp"
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
                    "open": "/bmp/eye_op_normal.bmp", // 開いた目のファイル名
                    "close": "/bmp/eye_cl_normal.bmp" // 閉じた目のファイル名
                },
                "minScaleX": 1.0,     // 未使用
                "maxScaleX": 1.0,     // 未使用
                "minScaleY": 0.3,     // 拡大倍率がこれより小さくなると閉じる
                "maxScaleY": 1.0      // 未使用
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
                    "open": "/bmp/eye_op_sad.bmp",
                    "close": "/bmp/eye_cl_sad.bmp"
                },
                "minScaleX": 1.0,
                "maxScaleX": 1.0,
                "minScaleY": 0.3,
                "maxScaleY": 1.0
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
                    "open": "/bmp/eye_op_angry.bmp",
                    "close": "/bmp/eye_cl_angry.bmp"
                },
                "minScaleX": 1.0,
                "maxScaleX": 1.0,
                "minScaleY": 0.3,
                "maxScaleY": 1.0
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

## カスタマイズ例
head.bmpをhead_potato.bmpに変更すると、〇ルカーっぽい何かになります。

# 参考にしたリポジトリ
- [m5stack-avatar](https://github.com/meganetaaan/m5stack-avatar)
- [M5Stack_WebRadio_Avator](https://github.com/robo8080/M5Stack_WebRadio_Avator)

# 謝辞
このソフトを作成するにあたり、動きや構造の元となった[M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar)を作成・公開してくださった[meganetaaan](https://github.com/meganetaaan)氏に感謝いたします。

ImageAvatarを実現するにあたり優れたパフォーマンス、機能を持ったLovyanGFXの作者[lovyan03](https://github.com/lovyan03)氏に感謝いたします。

ImageAvatar作成するにあたり、 初期の頃からたくさんのアドバイスを頂き、参考にさせていただいた[M5Stack_WebRadio_Avatar](https://github.com/robo8080/M5Stack_WebRadio_Avator)の作者[robo8080](https://github.com/robo8080)氏に感謝いたします。


# Credit
- [meganetaaan](https://github.com/meganetaaan)
- [lovyan03](https://github.com/lovyan03/LovyanGFX)
- [robo8080](https://github.com/robo8080)

# LICENSE
[MIT](LICENSE)

# Author
[Takao Akaki](https://github.com/mongonta0716)
