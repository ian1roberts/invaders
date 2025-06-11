#!/usr/bin/env python3
# Space Invaders Constants
import os

# Screen dimensions (based on original aspect ratio)
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
SCREEN_TITLE = "Space Invaders (Pygame Version)"
FPS = 60

# Colors (Atari-style palette)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 255, 0) 
RED = (255, 0, 0)
BLUE = (0, 0, 255)
YELLOW = (255, 255, 0)

# Game area
GAME_AREA_MARGIN_X = 50
GAME_AREA_MARGIN_Y = 50
GAME_AREA_WIDTH = SCREEN_WIDTH - (2 * GAME_AREA_MARGIN_X)
GAME_AREA_HEIGHT = SCREEN_HEIGHT - (2 * GAME_AREA_MARGIN_Y)

# Player settings
PLAYER_WIDTH = 60
PLAYER_HEIGHT = 30
PLAYER_SPEED = 5
PLAYER_LIVES = 3

# Invader settings
INVADER_ROWS = 5
INVADERS_PER_ROW = 11
INVADER_WIDTH = 40
INVADER_HEIGHT = 35
INVADER_H_SPACING = 15
INVADER_V_SPACING = 15
INVADER_H_PADDING = 50
INVADER_V_PADDING = 50
INVADER_MOVE_SPEED_H = 1  # Starting horizontal speed
INVADER_MOVE_DOWN = 20    # How far down invaders move when reaching edge

# Player bullet settings
PLAYER_BULLET_WIDTH = 3
PLAYER_BULLET_HEIGHT = 15
PLAYER_BULLET_SPEED = 10
PLAYER_BULLET_COOLDOWN = 500  # Milliseconds

# Invader bullet settings
INVADER_BULLET_WIDTH = 3
INVADER_BULLET_HEIGHT = 15
INVADER_BULLET_SPEED = 5
INVADER_FIRING_CHANCE = 0.01  # Chance of an invader firing each frame

# Barrier settings
BARRIER_COUNT = 4
BARRIER_WIDTH = 80
BARRIER_HEIGHT = 60
BARRIER_Y_POS = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - 100
BARRIER_DAMAGE_LEVELS = 4

# Mystery ship
MYSTERY_SHIP_WIDTH = 60
MYSTERY_SHIP_HEIGHT = 30
MYSTERY_SHIP_SPEED = 3
MYSTERY_SHIP_POINTS = [50, 100, 150, 300]  # Random point values
MYSTERY_SHIP_APPEAR_CHANCE = 0.001  # Chance per frame

# Score points
SCORE_INVADER_TOP_ROW = 30
SCORE_INVADER_MIDDLE_ROW = 20
SCORE_INVADER_BOTTOM_ROW = 10

# Font sizes
FONT_LARGE = 36
FONT_MEDIUM = 24
FONT_SMALL = 16

# Game states
STATE_ATTRACT = 0
STATE_PLAYING = 1
STATE_GAME_OVER = 2
STATE_LEVEL_TRANSITION = 3  # New state specifically for level transitions
STATE_HIGH_SCORE_ENTRY = 4  # New state for high score name entry
STATE_HALL_OF_FAME = 5  # New state for hall of fame display

# High score settings
HIGH_SCORE_COUNT = 10  # Number of high scores to keep
HIGH_SCORE_NAME_LENGTH = 5  # Max length of name
HIGH_SCORE_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=<>?."  # Valid characters for name entry
HIGH_SCORE_FILE = os.path.expanduser("~/.space_invaders_scores")  # Path to high score file