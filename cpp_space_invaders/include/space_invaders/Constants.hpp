#pragma once

#include <string>
#include <array>
#include <filesystem>

namespace SpaceInvaders {

// Screen dimensions
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
const std::string SCREEN_TITLE = "Space Invaders (C++ Version)";
constexpr int FPS = 60;
constexpr float FRAME_TIME = 1.0f / static_cast<float>(FPS);

// Colors (RGBA format)
struct Color {
    uint8_t r, g, b, a;
};

constexpr Color BLACK = {0, 0, 0, 255};
constexpr Color WHITE = {255, 255, 255, 255};
constexpr Color GREEN = {0, 255, 0, 255};
constexpr Color RED = {255, 0, 0, 255};
constexpr Color BLUE = {0, 0, 255, 255};
constexpr Color YELLOW = {255, 255, 0, 255};

// Game area
constexpr int GAME_AREA_MARGIN_X = 50;
constexpr int GAME_AREA_MARGIN_Y = 50;
constexpr int GAME_AREA_WIDTH = SCREEN_WIDTH - (2 * GAME_AREA_MARGIN_X);
constexpr int GAME_AREA_HEIGHT = SCREEN_HEIGHT - (2 * GAME_AREA_MARGIN_Y);

// Player settings
constexpr int PLAYER_WIDTH = 60;
constexpr int PLAYER_HEIGHT = 30;
constexpr int PLAYER_SPEED = 5;
constexpr int PLAYER_LIVES = 3;

// Invader settings
constexpr int INVADER_ROWS = 5;
constexpr int INVADERS_PER_ROW = 11;
constexpr int INVADER_WIDTH = 40;
constexpr int INVADER_HEIGHT = 35;
constexpr int INVADER_H_SPACING = 15;
constexpr int INVADER_V_SPACING = 15;
constexpr int INVADER_H_PADDING = 50;
constexpr int INVADER_V_PADDING = 50;
constexpr int INVADER_MOVE_SPEED_H = 1;
constexpr int INVADER_MOVE_DOWN = 20;

// Player bullet settings
constexpr int PLAYER_BULLET_WIDTH = 3;
constexpr int PLAYER_BULLET_HEIGHT = 15;
constexpr int PLAYER_BULLET_SPEED = 10;
constexpr int PLAYER_BULLET_COOLDOWN = 500;  // Milliseconds

// Invader bullet settings
constexpr int INVADER_BULLET_WIDTH = 3;
constexpr int INVADER_BULLET_HEIGHT = 15;
constexpr int INVADER_BULLET_SPEED = 5;
constexpr float INVADER_FIRING_CHANCE = 0.01f;

// Barrier settings
constexpr int BARRIER_COUNT = 4;
constexpr int BARRIER_WIDTH = 80;
constexpr int BARRIER_HEIGHT = 60;
constexpr int BARRIER_Y_POS = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - 100;
constexpr int BARRIER_DAMAGE_LEVELS = 4;
constexpr int BARRIER_PIECE_SIZE = 5;

// Mystery ship
constexpr int MYSTERY_SHIP_WIDTH = 60;
constexpr int MYSTERY_SHIP_HEIGHT = 30;
constexpr int MYSTERY_SHIP_SPEED = 3;
constexpr std::array<int, 4> MYSTERY_SHIP_POINTS = {50, 100, 150, 300};
constexpr float MYSTERY_SHIP_APPEAR_CHANCE = 0.001f;

// Score points
constexpr int SCORE_INVADER_TOP_ROW = 30;
constexpr int SCORE_INVADER_MIDDLE_ROW = 20;
constexpr int SCORE_INVADER_BOTTOM_ROW = 10;

// Font sizes
constexpr int FONT_LARGE = 36;
constexpr int FONT_MEDIUM = 24;
constexpr int FONT_SMALL = 16;

// Game states
enum class GameState {
    ATTRACT,
    PLAYING,
    GAME_OVER,
    LEVEL_TRANSITION,
    HIGH_SCORE_ENTRY,
    HALL_OF_FAME
};

// High score settings
constexpr int HIGH_SCORE_COUNT = 10;
constexpr int HIGH_SCORE_NAME_LENGTH = 5;
const std::string HIGH_SCORE_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=<>?.";
const std::string HIGH_SCORE_FILE = ".space_invaders_scores";

} // namespace SpaceInvaders
