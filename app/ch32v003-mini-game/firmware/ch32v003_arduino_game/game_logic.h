#pragma once

#include <stdint.h>

namespace GameLogic {

const int16_t SCREEN_WIDTH = 128;
const int16_t SCREEN_HEIGHT = 64;

const int16_t PLAYER_WIDTH = 12;
const int16_t PLAYER_HEIGHT = 5;
const int16_t PLAYER_Y = 57;
const int16_t PLAYER_STEP = 3;

const int16_t OBSTACLE_SIZE = 6;
const int16_t OBSTACLE_START_Y = 8;

struct State {
  int16_t playerX;
  int16_t obstacleX;
  int16_t obstacleY;
  uint16_t score;
};

enum StepResult {
  STEP_CONTINUES,
  STEP_SCORED,
  STEP_HIT,
};

inline bool overlaps(
  int16_t ax,
  int16_t ay,
  int16_t aw,
  int16_t ah,
  int16_t bx,
  int16_t by,
  int16_t bw,
  int16_t bh
) {
  return (
    ax < bx + bw &&
    ax + aw > bx &&
    ay < by + bh &&
    ay + ah > by
  );
}

inline bool obstacleHitsPlayer(const State &state) {
  return overlaps(
    state.playerX,
    PLAYER_Y,
    PLAYER_WIDTH,
    PLAYER_HEIGHT,
    state.obstacleX,
    state.obstacleY,
    OBSTACLE_SIZE,
    OBSTACLE_SIZE
  );
}

inline void movePlayer(
  State &state,
  bool leftPressed,
  bool rightPressed
) {
  // Opposing inputs cancel one another.
  if (leftPressed == rightPressed) {
    return;
  }

  state.playerX += leftPressed ? -PLAYER_STEP : PLAYER_STEP;

  if (state.playerX < 0) {
    state.playerX = 0;
  }

  const int16_t maxX = SCREEN_WIDTH - PLAYER_WIDTH;

  if (state.playerX > maxX) {
    state.playerX = maxX;
  }
}

inline uint8_t obstacleSpeed(uint16_t score) {
  const uint16_t speed = 2 + score / 8;
  return speed > 5 ? 5 : speed;
}

inline StepResult stepObstacle(State &state) {
  // Check each traversed row so a faster obstacle cannot skip the player.
  if (obstacleHitsPlayer(state)) {
    return STEP_HIT;
  }

  const uint8_t speed = obstacleSpeed(state.score);

  for (uint8_t step = 0; step < speed; step++) {
    state.obstacleY++;

    if (obstacleHitsPlayer(state)) {
      return STEP_HIT;
    }

    if (state.obstacleY >= SCREEN_HEIGHT) {
      if (state.score != UINT16_MAX) {
        state.score++;
      }

      return STEP_SCORED;
    }
  }

  return STEP_CONTINUES;
}

inline int16_t obstacleSpawnX(uint32_t randomValue) {
  return randomValue % (SCREEN_WIDTH - OBSTACLE_SIZE + 1);
}

inline void resetObstacle(State &state, uint32_t randomValue) {
  state.obstacleX = obstacleSpawnX(randomValue);
  state.obstacleY = OBSTACLE_START_Y;
}

inline void resetGame(State &state, uint32_t randomValue) {
  state.playerX = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
  state.score = 0;
  resetObstacle(state, randomValue);
}

}  // namespace GameLogic
