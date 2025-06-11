#pragma once

#include "Constants.hpp"
#include "Rectangle.hpp"
#include "Player.hpp"
#include "Invader.hpp"
#include "Barrier.hpp"
#include "MysteryShip.hpp"
#include "Bullet.hpp"
#include "GraphicsGenerator.hpp"
#include "SoundGenerator.hpp"
#include "HighScoreManager.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <vector>
#include <string>
#include <utility>

namespace SpaceInvaders {

struct Explosion {
    std::shared_ptr<SDL_Texture> sprite;
    int x;
    int y;
    uint32_t startTime;
};

class Game {
public:
    Game();
    ~Game();
    
    bool initialize();
    void run();
    
private:
    // SDL Components
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    uint32_t lastFrameTime;
    
    // Game state
    GameState gameState;
    Rectangle gameArea;
    int score;
    int highScore;
    int level;
    uint32_t lastInvaderMovementSound;
    int invaderMovementSoundIndex;
    std::vector<Explosion> explosions;
    uint32_t gameOverTimer;
    std::vector<std::shared_ptr<SDL_Texture>> digitSprites;
    
    // Level transition
    uint32_t transitionTimer;
    int transitionDelay;
    
    // High score entry
    std::vector<char> playerName;
    int currentChar;
    int charIndex;
    uint32_t nameEntryCooldown;
    int nameEntryDelay;
    
    // Hall of Fame
    int scrollPosition;
    uint32_t scrollTimer;
    int scrollSpeed;
    
    // Game objects
    std::shared_ptr<Player> player;
    std::shared_ptr<InvaderGroup> invaderGroup;
    std::vector<std::shared_ptr<Barrier>> barriers;
    std::shared_ptr<MysteryShip> mysteryShip;
    std::vector<std::shared_ptr<Bullet>> playerBullets;
    std::vector<std::shared_ptr<Bullet>> invaderBullets;
    
    // Helper classes
    SoundGenerator soundGenerator;
    HighScoreManager highScoreManager;
    
    // Game loop methods
    void handleEvents();
    void update(float deltaTime);
    void draw();
    
    // Game state methods
    void initGameObjects();
    void startNewGame();
    void gameOver();
    void startLevelTransition();
    void completeLevelTransition();
    void nextLevel();
    void tryPlayerShoot();
    
    // Collision methods
    void checkCollisions();
    void addExplosion(int x, int y);
    
    // High score methods
    void updateHighScoreEntry();
    void submitHighScore();
    void updateHallOfFame();
    
    // Drawing methods
    void drawScore();
    void drawDigits(const std::string& numberStr, int x, int y);
    void drawLives();
};

} // namespace SpaceInvaders
