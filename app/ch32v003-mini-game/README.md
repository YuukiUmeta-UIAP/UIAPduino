# CH32V003 Mini Game / CH32V003 ミニゲーム

A small dodge game for the UIAPduino Pro Micro CH32V003 V1.4. Move the player
left and right on a 128×64 I2C OLED, avoid falling obstacles, and use the piezo
buzzer for sound and startup diagnostics.

UIAPduino Pro Micro CH32V003 V1.4、128×64 I2C OLED、3 個のボタン、
圧電ブザーで動く落下物よけゲームです。プレイヤーを左右に動かして落下物を避け、
ブザーを効果音と起動時の診断に使用します。

# Status / ステータス

- Confirmed working on actual hardware, including display, input, sound, game
  progression, restart, compilation, and upload. / 表示、入力、サウンド、ゲーム進行、
  再開、コンパイル、書き込みまで実機で動作確認済みです。
- Target: UIAPduino Pro Micro CH32V003 V1.4 / 対象ボード: UIAPduino Pro Micro
  CH32V003 V1.4
- Flash: 14,648 / 16,384 bytes (89%)
- RAM: 880 / 2,048 bytes (42%)
- Tested with Arduino IDE 2.3.10 and UIAPduino board package 1.0.42. /
  Arduino IDE 2.3.10、UIAPduino ボードパッケージ 1.0.42 で確認しました。

# How to Play / 遊び方

- LEFT: move left / 左へ移動
- RIGHT: move right / 右へ移動
- ACTION: start or restart / ゲーム開始または再開
- Each avoided obstacle adds one score dot. / 落下物を避けるたびに画面上部の
  得点ドットが増えます。
- The obstacle speeds up every 8 points, up to a fixed maximum. / 8 点ごとに
  落下速度が上がります。速度には上限があります。
- A collision plays a low tone and draws a border around the screen. / 衝突すると
  低い音が鳴り、画面に枠が表示されます。

# Parts / 部品

- UIAPduino Pro Micro CH32V003 V1.4
- 128×64 I2C OLED (SSD1306/SSD1315, address `0x3C` or `0x3D`)
- 3 tactile switches / タクトスイッチ × 3
- 1 piezo buzzer / 圧電ブザー × 1
- Breadboard and jumper wires / ブレッドボードとジャンパ線

# Circuit / 回路

| UIAPduino | GPIO | Wire color / 配線色 | Connection / 接続先 |
| --- | --- | --- | --- |
| `3V3` | - | Red / 赤 | OLED `VDD` |
| `GND` | - | Black / 黒 | Common GND rail / GND 共通線 |
| `D3` | `PC1` | Blue / 青 | OLED `SDA` |
| `D4` | `PC2` | Yellow / 黄 | OLED `SCK/SCL` |
| `D5` | `PC3` | Purple / 紫 | Buzzer `+` / ブザー `+` |
| `D8` | `PC6` | Green / 緑 | LEFT |
| `D9` | `PC7` | Orange / 橙 | RIGHT |
| `D10` | `PD0` | White / 白 | ACTION |

```text
                         UIAPduino CH32V003
                      ┌──────────────────────┐
 OLED SDA ── blue ────┤ D3  PC1             │
 OLED SCL ── yellow ───┤ D4  PC2             │
 BUZZER + ── purple ───┤ D5  PC3             │
 LEFT ───── green ─────┤ D8  PC6             │
 RIGHT ──── orange ────┤ D9  PC7             │
 ACTION ─── white ─────┤ D10 PD0             │
 OLED VDD ─ red ───────┤ 3V3                 │
 GND rail ─ black ─────┤ GND                 │
                      └──────────────────────┘

                       GND rail
                          │
             ┌────────────┼────────────┐
             │            │            │
          OLED GND     BUZZER -     BUTTONS
                                      │
                               LEFT / RIGHT / ACTION
```

Connect the OLED GND, buzzer `-`, the other side of every button, and UIAPduino
GND to the same ground rail. / OLED の GND、ブザーの `-`、各ボタンの反対側、
UIAPduino の GND を同じ GND 共通線へ接続します。

Each button connects a GPIO directly to GND. The firmware uses the internal
pull-up resistors, so no external pull-up is needed. / 各ボタンは GPIO と GND の
間に接続します。内部プルアップを使用するため、外付け抵抗は不要です。

```text
GPIO ──── button ──── GND

released / 未押下: HIGH
pressed  / 押下:   LOW
```

`PD1 / D11` is reserved for SWIO and is not used by the game. /
`PD1 / D11` は SWIO 用なので、ゲームの配線には使用しません。

# Program / プログラム

[`firmware/ch32v003_arduino_game/ch32v003_arduino_game.ino`](firmware/ch32v003_arduino_game/ch32v003_arduino_game.ino)
and [`firmware/ch32v003_arduino_game/game_logic.h`](firmware/ch32v003_arduino_game/game_logic.h)
are the complete firmware. /
この 2 ファイルが完全なファームウェアです。

The SSD1306/SSD1315 is controlled directly through `Wire`; no external display
library is required. A 128-byte page buffer is used instead of a 1 KiB full
frame buffer to fit the CH32V003 memory limits. / 外部表示ライブラリは使わず、
`Wire` から OLED を直接制御します。CH32V003 のメモリに収めるため、1 KiB の
全画面バッファではなく 128 バイトのページバッファを使用します。

## Build and Upload / コンパイルと書き込み

1. Install the official UIAPduino board package in Arduino IDE. / Arduino IDE に
   公式 UIAPduino ボードパッケージをインストールします。
2. Select **Tools > Board > UIAPduino > Pro Micro CH32V003**. / 同ボードを
   選択します。
3. Open `firmware/ch32v003_arduino_game/ch32v003_arduino_game.ino` and click
   **Verify**. / スケッチを開き、**Verify** でコンパイルします。
4. Put the board into write-standby mode, click **Upload**, and wait for
   `Image written.` / ボードを write-standby モードにして **Upload** を押し、
   `Image written.` を確認します。
5. Reset the board to run the game. / リセットしてゲームを起動します。

The official Boards Manager URL is:

```text
https://github.com/YuukiUmeta-UIAP/board_manager_files/raw/main/package_uiap.jp_index.json
```

Detailed Linux setup, write-standby instructions, troubleshooting, and host-side
game-logic tests are available in the
[development repository](https://github.com/mnishiguchi/hello-uiap/tree/main/ch32v003_arduino_game). /
Linux のセットアップ、書き込み待機、トラブルシューティング、ゲームロジックの
ホストテストは開発リポジトリにあります。

## Startup Diagnostics / 起動時の診断

- One short high beep: OLED detected at `0x3C` or `0x3D`. /
  高い短音 1 回: OLED を検出しました。
- Three low beeps: no OLED response; check `3V3`, GND, SDA (`D3/PC1`), and SCL
  (`D4/PC2`). / 低い音 3 回: OLED から応答がありません。電源、GND、SDA、
  SCL を確認してください。

The I2C bus runs at 100 kHz for reliable breadboard operation. /
ブレッドボードでも安定するよう、I2C は 100 kHz で動作します。
