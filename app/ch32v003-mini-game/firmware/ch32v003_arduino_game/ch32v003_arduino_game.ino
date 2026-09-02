#include <Wire.h>
#include <string.h>

#include "game_logic.h"

const uint8_t PIN_LEFT = 8;
const uint8_t PIN_RIGHT = 9;
const uint8_t PIN_ACTION = 10;
const uint8_t PIN_BUZZER = 5;

const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 64;
const uint8_t FRAME_DURATION_MS = 50;
const uint8_t ACTION_DEBOUNCE_MS = 20;

uint8_t pageBuffer[SCREEN_WIDTH];
uint8_t oledAddress = 0;

GameLogic::State game = {58, 20, 8, 0};
uint32_t randomState = 1;
uint32_t lastFrameAt = 0;
uint32_t actionChangedAt = 0;

bool running = false;
bool gameOver = false;
bool actionRawPressed = false;
bool actionStablePressed = false;

void oledCommand(uint8_t command) {
  Wire.beginTransmission(oledAddress);
  Wire.write(0x00);
  Wire.write(command);
  Wire.endTransmission();
}

void oledCommands(const uint8_t *commands, uint8_t length) {
  Wire.beginTransmission(oledAddress);
  Wire.write(0x00);
  Wire.write(commands, length);
  Wire.endTransmission();
}

void oledInit() {
  const uint8_t commands[] = {
    0xAE,
    0xD5, 0x80,
    0xA8, 0x3F,
    0xD3, 0x00,
    0x40,
    0x8D, 0x14,
    0x20, 0x02,
    0xA1,
    0xC8,
    0xDA, 0x12,
    0x81, 0x7F,
    0xD9, 0xF1,
    0xDB, 0x40,
    0xA4,
    0xA6,
    0xAF,
  };

  oledCommands(commands, sizeof(commands));
}

bool findOled() {
  const uint8_t addresses[] = {0x3C, 0x3D};

  for (uint8_t i = 0; i < sizeof(addresses); i++) {
    Wire.beginTransmission(addresses[i]);

    if (Wire.endTransmission() == 0) {
      oledAddress = addresses[i];
      return true;
    }
  }

  return false;
}

void oledWritePage(uint8_t page) {
  oledCommand(0xB0 + page);
  oledCommand(0x00);
  oledCommand(0x10);

  for (uint8_t offset = 0; offset < SCREEN_WIDTH; offset += 16) {
    Wire.beginTransmission(oledAddress);
    Wire.write(0x40);
    Wire.write(pageBuffer + offset, 16);
    Wire.endTransmission();
  }
}

void drawFilledRectToPage(
  int16_t x,
  int16_t y,
  int16_t width,
  int16_t height,
  uint8_t page
) {
  const int16_t pageTop = page * 8;
  const int16_t pageBottom = pageTop + 7;
  const int16_t rectBottom = y + height - 1;

  if (
    x >= SCREEN_WIDTH ||
    y > pageBottom ||
    rectBottom < pageTop
  ) {
    return;
  }

  const int16_t left = x < 0 ? 0 : x;
  const int16_t right =
    x + width > SCREEN_WIDTH ? SCREEN_WIDTH : x + width;
  const int16_t top = y < pageTop ? pageTop : y;
  const int16_t bottom =
    rectBottom > pageBottom ? pageBottom : rectBottom;

  for (int16_t px = left; px < right; px++) {
    for (int16_t py = top; py <= bottom; py++) {
      pageBuffer[px] |= 1 << (py - pageTop);
    }
  }
}

void drawScoreToPage(uint8_t page) {
  const uint8_t visibleScore = game.score > 30 ? 30 : game.score;

  for (uint8_t i = 0; i < visibleScore; i++) {
    drawFilledRectToPage(2 + i * 4, 2, 2, 2, page);
  }
}

void render() {
  if (oledAddress == 0) {
    return;
  }

  for (uint8_t page = 0; page < 8; page++) {
    memset(pageBuffer, 0, sizeof(pageBuffer));

    drawScoreToPage(page);

    drawFilledRectToPage(
      game.playerX,
      GameLogic::PLAYER_Y,
      GameLogic::PLAYER_WIDTH,
      GameLogic::PLAYER_HEIGHT,
      page
    );

    if (running) {
      drawFilledRectToPage(
        game.obstacleX,
        game.obstacleY,
        GameLogic::OBSTACLE_SIZE,
        GameLogic::OBSTACLE_SIZE,
        page
      );
    }

    if (gameOver) {
      drawFilledRectToPage(0, 0, SCREEN_WIDTH, 1, page);
      drawFilledRectToPage(
        0,
        SCREEN_HEIGHT - 1,
        SCREEN_WIDTH,
        1,
        page
      );
      drawFilledRectToPage(0, 0, 1, SCREEN_HEIGHT, page);
      drawFilledRectToPage(
        SCREEN_WIDTH - 1,
        0,
        1,
        SCREEN_HEIGHT,
        page
      );
    }

    oledWritePage(page);
  }
}

void beep(uint16_t frequency, uint16_t durationMs) {
  const uint32_t halfPeriodUs = 500000UL / frequency;
  const uint32_t cycles =
    (durationMs * 1000UL) / (halfPeriodUs * 2);

  for (uint32_t i = 0; i < cycles; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delayMicroseconds(halfPeriodUs);

    digitalWrite(PIN_BUZZER, LOW);
    delayMicroseconds(halfPeriodUs);
  }
}

uint32_t nextRandom() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;

  return randomState;
}

void resetObstacle() {
  GameLogic::resetObstacle(game, nextRandom());
}

void startGame() {
  randomState = millis() | 1;
  GameLogic::resetGame(game, nextRandom());

  gameOver = false;
  running = true;

  beep(1000, 40);
}

void updateGame() {
  GameLogic::movePlayer(
    game,
    digitalRead(PIN_LEFT) == LOW,
    digitalRead(PIN_RIGHT) == LOW
  );

  const GameLogic::StepResult result =
    GameLogic::stepObstacle(game);

  if (result == GameLogic::STEP_SCORED) {
    resetObstacle();
    beep(1400, 20);
  }

  if (result == GameLogic::STEP_HIT) {
    running = false;
    gameOver = true;

    beep(300, 160);
  }
}

bool actionPressedEvent(uint32_t now) {
  const bool pressed = digitalRead(PIN_ACTION) == LOW;

  if (pressed != actionRawPressed) {
    actionRawPressed = pressed;
    actionChangedAt = now;
  }

  if (
    actionStablePressed != actionRawPressed &&
    now - actionChangedAt >= ACTION_DEBOUNCE_MS
  ) {
    actionStablePressed = actionRawPressed;
    return actionStablePressed;
  }

  return false;
}

void setup() {
  // UIAPduino Seamless Switch.
  if (FLASH->STATR & (1 << 14)) {
    NVIC_SystemReset();
  }

  SystemReset_StartMode(Start_Mode_BOOT);
  pinMode(PD4, OUTPUT);

  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ACTION, INPUT_PULLUP);

  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Wire.begin();
  Wire.setClock(100000);

  // Let the OLED power rail and controller finish starting up.
  delay(100);

  if (findOled()) {
    oledInit();
    render();
    beep(1800, 30);
  } else {
    // Three low beeps mean neither common OLED address acknowledged.
    for (uint8_t i = 0; i < 3; i++) {
      beep(250, 70);
      delay(70);
    }
  }

  lastFrameAt = millis();
}

void loop() {
  const uint32_t now = millis();

  if (
    actionPressedEvent(now) &&
    !running
  ) {
    startGame();
  }

  if (now - lastFrameAt < FRAME_DURATION_MS) {
    delay(1);
    return;
  }

  if (running) {
    updateGame();
  }

  render();
  lastFrameAt = millis();
}
