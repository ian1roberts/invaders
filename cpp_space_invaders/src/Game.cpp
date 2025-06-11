#include "space_invaders/Game.hpp"
#include <iostream>
#include <algorithm>
#include <random>

namespace SpaceInvaders {

Game::Game() :
    window(nullptr),
    renderer(nullptr),
    running(false),
    lastFrameTime(0),
    gameState(GameState::ATTRACT),
    score(0),
    highScore(0),
    level(1),
    lastInvaderMovementSound(0),
    invaderMovementSoundIndex(0),
    gameOverTimer(0),
    transitionTimer(0),
    transitionDelay(2000),  // 2 seconds
    nameEntryCooldown(0),
    nameEntryDelay(150),  // ms between character changes
    scrollPosition(0),
    scrollTimer(0),
    scrollSpeed(1)  // pixels per frame
{
    // Initialize player name with default values
    playerName.resize(HIGH_SCORE_NAME_LENGTH, 'A');
    currentChar = 0;
    charIndex = 0;
}

Game::~Game() {
    // Clean up SDL resources
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    // Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool Game::initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // Create window
    window = SDL_CreateWindow(
        SCREEN_TITLE.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set render draw color for background (black)
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    
    // Initialize graphics generator
    GraphicsGenerator::initialize(renderer);
    
    // Initialize sound generator
    try {
        soundGenerator.initialize();
        soundGenerator.generateAllSounds();
    } catch (const std::exception& e) {
        std::cerr << "Sound initialization error: " << e.what() << std::endl;
        // Continue without sound - it's not critical
    }
    
    // Initialize game area
    gameArea = Rectangle(
        GAME_AREA_MARGIN_X,
        GAME_AREA_MARGIN_Y,
        GAME_AREA_WIDTH,
        GAME_AREA_HEIGHT
    );
    
    // Get the high score
    const auto& scores = highScoreManager.getHighScores();
    if (!scores.empty()) {
        highScore = scores[0].score;
    }
    
    // Create digit sprites for score display
    digitSprites = GraphicsGenerator::createDigitSprites();
    
    // Initialize game objects
    initGameObjects();
    
    return true;
}

void Game::initGameObjects() {
    // Create player
    int playerX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
    int playerY = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - PLAYER_HEIGHT - 20;
    player = std::make_shared<Player>(playerX, playerY);
    
    // Create invader group
    invaderGroup = std::make_shared<InvaderGroup>();
    invaderGroup->createInvaders();
    
    // Create barriers
    barriers.clear();
    int barrierSpacing = GAME_AREA_WIDTH / (BARRIER_COUNT + 1);
    for (int i = 0; i < BARRIER_COUNT; i++) {
        int barrierX = GAME_AREA_MARGIN_X + barrierSpacing * (i + 1) - (BARRIER_WIDTH / 2);
        int barrierY = BARRIER_Y_POS;
        barriers.push_back(std::make_shared<Barrier>(barrierX, barrierY));
    }
    
    // Create mystery ship
    mysteryShip = std::make_shared<MysteryShip>();
    
    // Clear bullet lists
    playerBullets.clear();
    invaderBullets.clear();
    
    // Clear explosions
    explosions.clear();
}

void Game::run() {
    // Framerate control variables
    uint32_t lastTime = SDL_GetTicks();
    float deltaTime = 0.0f;
    running = true;
    
    // Main game loop
    while (running) {
        // Calculate delta time
        uint32_t currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Handle events, update, and render
        handleEvents();
        update(deltaTime);
        draw();
        
        // Simple frame limiting
        uint32_t frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < 1000 / FPS) {
            SDL_Delay(1000 / FPS - frameTime);
        }
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                    
                case SDLK_RETURN:
                    // Start game from attract mode
                    if (gameState == GameState::ATTRACT) {
                        startNewGame();
                    }
                    // Restart after game over
                    else if (gameState == GameState::GAME_OVER) {
                        if (SDL_GetTicks() - gameOverTimer > 2000) {
                            startNewGame();
                        }
                    }
                    // Start new game from hall of fame
                    else if (gameState == GameState::HALL_OF_FAME) {
                        startNewGame();
                    }
                    // Submit name in high score entry
                    else if (gameState == GameState::HIGH_SCORE_ENTRY) {
                        submitHighScore();
                    }
                    break;
                    
                case SDLK_SPACE:
                    // Shoot only when playing
                    if (gameState == GameState::PLAYING && player->isAlive()) {
                        tryPlayerShoot();
                    }
                    // Submit name in high score entry
                    else if (gameState == GameState::HIGH_SCORE_ENTRY) {
                        submitHighScore();
                    }
                    break;
                    
                case SDLK_q:
                    // Reset high scores in hall of fame screen
                    if (gameState == GameState::HALL_OF_FAME) {
                        highScoreManager.resetScores();
                    }
                    break;
                    
                // High score name entry navigation
                case SDLK_LEFT:
                    if (gameState == GameState::HIGH_SCORE_ENTRY) {
                        currentChar = (currentChar - 1 + HIGH_SCORE_NAME_LENGTH) % HIGH_SCORE_NAME_LENGTH;
                        nameEntryCooldown = SDL_GetTicks();
                    }
                    break;
                    
                case SDLK_RIGHT:
                    if (gameState == GameState::HIGH_SCORE_ENTRY) {
                        currentChar = (currentChar + 1) % HIGH_SCORE_NAME_LENGTH;
                        nameEntryCooldown = SDL_GetTicks();
                    }
                    break;
                    
                case SDLK_UP:
                    if (gameState == GameState::HIGH_SCORE_ENTRY) {
                        // Find the current character in the character list
                        auto it = HIGH_SCORE_CHARS.find(playerName[currentChar]);
                        if (it != std::string::npos) {
                            charIndex = static_cast<int>(it);
                        } else {
                            charIndex = 0;
                        }
                        
                        // Move to previous character
                        charIndex = (charIndex - 1 + HIGH_SCORE_CHARS.length()) % HIGH_SCORE_CHARS.length();
                        playerName[currentChar] = HIGH_SCORE_CHARS[charIndex];
                        nameEntryCooldown = SDL_GetTicks();
                    }
                    break;
                    
                case SDLK_DOWN:
                    if (gameState == GameState::HIGH_SCORE_ENTRY) {
                        // Find the current character in the character list
                        auto it = HIGH_SCORE_CHARS.find(playerName[currentChar]);
                        if (it != std::string::npos) {
                            charIndex = static_cast<int>(it);
                        } else {
                            charIndex = 0;
                        }
                        
                        // Move to next character
                        charIndex = (charIndex + 1) % HIGH_SCORE_CHARS.length();
                        playerName[currentChar] = HIGH_SCORE_CHARS[charIndex];
                        nameEntryCooldown = SDL_GetTicks();
                    }
                    break;
            }
        }
    }
    
    // Check continuous key presses for gameplay
    if (gameState == GameState::PLAYING && player->isAlive()) {
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);
        
        if (keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A]) {
            player->move(-1, gameArea);
        }
        
        if (keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D]) {
            player->move(1, gameArea);
        }
        
        if (keyState[SDL_SCANCODE_SPACE]) {
            tryPlayerShoot();
        }
    }
}

void Game::update(float deltaTime) {
    uint32_t currentTime = SDL_GetTicks();
    
    // Handle different game states
    if (gameState == GameState::LEVEL_TRANSITION) {
        // Handle level transition timing
        if (currentTime - transitionTimer > transitionDelay) {
            completeLevelTransition();
        }
    } else if (gameState == GameState::PLAYING) {
        // Update player
        player->update(deltaTime);
        
        // Update player bullets
        for (auto it = playerBullets.begin(); it != playerBullets.end();) {
            auto& bullet = *it;
            bullet->update(deltaTime);
            
            if (!bullet->isActive()) {
                it = playerBullets.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update invader bullets
        for (auto it = invaderBullets.begin(); it != invaderBullets.end();) {
            auto& bullet = *it;
            bullet->update(deltaTime);
            
            if (!bullet->isActive()) {
                it = invaderBullets.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update invader movement
        if (invaderGroup->move(currentTime, gameArea)) {
            // Play movement sound when invaders move down
            soundGenerator.playSound("invader_movement");
            lastInvaderMovementSound = currentTime;
            invaderMovementSoundIndex = (invaderMovementSoundIndex + 1) % 4;
        }
        
        // Check if invaders have reached the bottom
        if (invaderGroup->anyInvaderAtBottom(player->getCollisionRect().y)) {
            gameOver();
        }
        
        // Random invader shooting
        auto shooter = invaderGroup->getRandomShooter();
        if (shooter && shooter->isAlive()) {
            int bulletX = shooter->getCollisionRect().x + (shooter->getCollisionRect().width / 2) - (INVADER_BULLET_WIDTH / 2);
            int bulletY = shooter->getCollisionRect().y + shooter->getCollisionRect().height;
            invaderBullets.push_back(std::make_shared<InvaderBullet>(bulletX, bulletY));
            soundGenerator.playSound("invader_shoot");
        }
        
        // Mystery ship logic
        if (!mysteryShip->isActive()) {
            static uint32_t lastMysteryShipTime = 0;
            static int mysteryShipDelay = 15000 + rand() % 15000;  // 15-30 seconds
            
            if (currentTime - lastMysteryShipTime > mysteryShipDelay) {
                mysteryShip->activate(SCREEN_WIDTH);
                lastMysteryShipTime = currentTime;
                mysteryShipDelay = 15000 + rand() % 15000;
                soundGenerator.playSound("mystery_ship");
            }
        } else {
            mysteryShip->update(SCREEN_WIDTH);
            if (!mysteryShip->isActive()) {
                soundGenerator.stopSound("mystery_ship");
            }
        }
        
        // Check collisions
        checkCollisions();
        
        // Check if all invaders are dead
        if (invaderGroup->allDead()) {
            startLevelTransition();
        }
    } else if (gameState == GameState::HIGH_SCORE_ENTRY) {
        updateHighScoreEntry();
    } else if (gameState == GameState::HALL_OF_FAME) {
        updateHallOfFame();
    }
    
    // Update explosions (do this in all states)
    for (auto it = explosions.begin(); it != explosions.end();) {
        if (currentTime - it->startTime > 500) {  // Explosion lasts 500ms
            it = explosions.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::draw() {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(renderer);
    
    // Draw game border
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    SDL_Rect border = {gameArea.x, gameArea.y, gameArea.width, gameArea.height};
    SDL_RenderDrawRect(renderer, &border);
    
    // Draw score and high score
    drawScore();
    
    if (gameState == GameState::ATTRACT) {
        // Draw title screen
        auto titleText = GraphicsGenerator::createTextSurface("SPACE INVADERS", FONT_LARGE);
        
        int titleWidth, titleHeight;
        SDL_QueryTexture(titleText.get(), nullptr, nullptr, &titleWidth, &titleHeight);
        int titleX = SCREEN_WIDTH / 2 - titleWidth / 2;
        
        SDL_Rect titleRect = {titleX, SCREEN_HEIGHT / 3, titleWidth, titleHeight};
        SDL_RenderCopy(renderer, titleText.get(), nullptr, &titleRect);
        
        // Draw blinking "PRESS ENTER TO START"
        if ((SDL_GetTicks() / 500) % 2 == 0) {
            auto startText = GraphicsGenerator::createTextSurface("PRESS ENTER TO START", FONT_MEDIUM);
            
            int startWidth, startHeight;
            SDL_QueryTexture(startText.get(), nullptr, nullptr, &startWidth, &startHeight);
            int startX = SCREEN_WIDTH / 2 - startWidth / 2;
            
            SDL_Rect startRect = {startX, SCREEN_HEIGHT / 2, startWidth, startHeight};
            SDL_RenderCopy(renderer, startText.get(), nullptr, &startRect);
        }
    } else if (gameState == GameState::LEVEL_TRANSITION) {
        // Draw level transition screen
        auto levelCompleteText = GraphicsGenerator::createTextSurface("LEVEL " + std::to_string(level) + " COMPLETE!", FONT_LARGE);
        
        int textWidth, textHeight;
        SDL_QueryTexture(levelCompleteText.get(), nullptr, nullptr, &textWidth, &textHeight);
        int textX = SCREEN_WIDTH / 2 - textWidth / 2;
        int textY = SCREEN_HEIGHT / 2 - textHeight / 2;
        
        SDL_Rect textRect = {textX, textY, textWidth, textHeight};
        SDL_RenderCopy(renderer, levelCompleteText.get(), nullptr, &textRect);
        
        auto nextLevelText = GraphicsGenerator::createTextSurface("PREPARING LEVEL " + std::to_string(level + 1) + "...", FONT_MEDIUM);
        
        SDL_QueryTexture(nextLevelText.get(), nullptr, nullptr, &textWidth, &textHeight);
        textX = SCREEN_WIDTH / 2 - textWidth / 2;
        textY = SCREEN_HEIGHT / 2 + 50;
        
        SDL_Rect nextRect = {textX, textY, textWidth, textHeight};
        SDL_RenderCopy(renderer, nextLevelText.get(), nullptr, &nextRect);
        
        // Continue displaying game elements in the background
        player->draw(renderer);
        for (auto& barrier : barriers) {
            barrier->draw(renderer);
        }
        
        // Draw explosions
        for (const auto& explosion : explosions) {
            int width, height;
            SDL_QueryTexture(explosion.sprite.get(), nullptr, nullptr, &width, &height);
            SDL_Rect dstRect = {explosion.x, explosion.y, width, height};
            SDL_RenderCopy(renderer, explosion.sprite.get(), nullptr, &dstRect);
        }
    } else if (gameState == GameState::PLAYING || gameState == GameState::GAME_OVER) {
        // Draw player
        player->draw(renderer);
        
        // Draw invaders
        invaderGroup->draw(renderer);
        
        // Draw mystery ship
        mysteryShip->draw(renderer);
        
        // Draw barriers
        for (auto& barrier : barriers) {
            barrier->draw(renderer);
        }
        
        // Draw bullets
        for (auto& bullet : playerBullets) {
            bullet->draw(renderer);
        }
        
        for (auto& bullet : invaderBullets) {
            bullet->draw(renderer);
        }
        
        // Draw explosions
        for (const auto& explosion : explosions) {
            int width, height;
            SDL_QueryTexture(explosion.sprite.get(), nullptr, nullptr, &width, &height);
            SDL_Rect dstRect = {explosion.x, explosion.y, width, height};
            SDL_RenderCopy(renderer, explosion.sprite.get(), nullptr, &dstRect);
        }
        
        // Draw lives
        drawLives();
        
        // Draw level
        auto levelText = GraphicsGenerator::createTextSurface("LEVEL: " + std::to_string(level), FONT_SMALL);
        
        int levelWidth, levelHeight;
        SDL_QueryTexture(levelText.get(), nullptr, nullptr, &levelWidth, &levelHeight);
        int levelX = SCREEN_WIDTH - 50 - levelWidth;
        
        SDL_Rect levelRect = {levelX, SCREEN_HEIGHT - 40, levelWidth, levelHeight};
        SDL_RenderCopy(renderer, levelText.get(), nullptr, &levelRect);
        
        // Draw game over text
        if (gameState == GameState::GAME_OVER) {
            auto gameOverText = GraphicsGenerator::createTextSurface("GAME OVER", FONT_LARGE);
            
            int gameOverWidth, gameOverHeight;
            SDL_QueryTexture(gameOverText.get(), nullptr, nullptr, &gameOverWidth, &gameOverHeight);
            int gameOverX = SCREEN_WIDTH / 2 - gameOverWidth / 2;
            
            SDL_Rect gameOverRect = {gameOverX, SCREEN_HEIGHT / 2, gameOverWidth, gameOverHeight};
            SDL_RenderCopy(renderer, gameOverText.get(), nullptr, &gameOverRect);
            
            if ((SDL_GetTicks() / 500) % 2 == 0 && SDL_GetTicks() - gameOverTimer > 2000) {
                auto restartText = GraphicsGenerator::createTextSurface("PRESS ENTER TO RESTART", FONT_MEDIUM);
                
                int restartWidth, restartHeight;
                SDL_QueryTexture(restartText.get(), nullptr, nullptr, &restartWidth, &restartHeight);
                int restartX = SCREEN_WIDTH / 2 - restartWidth / 2;
                
                SDL_Rect restartRect = {restartX, SCREEN_HEIGHT / 2 + 50, restartWidth, restartHeight};
                SDL_RenderCopy(renderer, restartText.get(), nullptr, &restartRect);
            }
        }
    } else if (gameState == GameState::HIGH_SCORE_ENTRY) {
        // Draw the high score entry screen here (simplified for this draft)
        auto entryText = GraphicsGenerator::createTextSurface("NEW HIGH SCORE!", FONT_LARGE);
        
        int entryWidth, entryHeight;
        SDL_QueryTexture(entryText.get(), nullptr, nullptr, &entryWidth, &entryHeight);
        int entryX = SCREEN_WIDTH / 2 - entryWidth / 2;
        
        SDL_Rect entryRect = {entryX, SCREEN_HEIGHT / 4, entryWidth, entryHeight};
        SDL_RenderCopy(renderer, entryText.get(), nullptr, &entryRect);
        
        // Draw more UI components for the high score entry screen...
    } else if (gameState == GameState::HALL_OF_FAME) {
        // Draw the hall of fame screen here (simplified for this draft)
        auto titleText = GraphicsGenerator::createTextSurface("* HALL OF FAME *", FONT_LARGE);
        
        int titleWidth, titleHeight;
        SDL_QueryTexture(titleText.get(), nullptr, nullptr, &titleWidth, &titleHeight);
        int titleX = SCREEN_WIDTH / 2 - titleWidth / 2;
        
        SDL_Rect titleRect = {titleX, 50, titleWidth, titleHeight};
        SDL_RenderCopy(renderer, titleText.get(), nullptr, &titleRect);
        
        // Draw more UI components for the hall of fame screen...
    }
    
    // Update the screen
    SDL_RenderPresent(renderer);
}

void Game::startNewGame() {
    score = 0;
    level = 1;
    initGameObjects();
    gameState = GameState::PLAYING;
}

void Game::gameOver() {
    // Update high score
    if (score > highScore) {
        highScore = score;
    }
    
    // Check if score qualifies for high score list
    if (highScoreManager.isHighScore(score)) {
        // Show high score entry screen
        gameState = GameState::HIGH_SCORE_ENTRY;
        std::fill(playerName.begin(), playerName.end(), 'A');
        currentChar = 0;
        charIndex = 0;
        nameEntryCooldown = 0;
    } else {
        // Standard game over
        gameState = GameState::GAME_OVER;
        gameOverTimer = SDL_GetTicks();
        soundGenerator.playSound("game_over");
    }
}

void Game::startLevelTransition() {
    gameState = GameState::LEVEL_TRANSITION;
    transitionTimer = SDL_GetTicks();
}

void Game::completeLevelTransition() {
    int oldLevel = level;
    level++;
    
    // Clear bullets
    playerBullets.clear();
    invaderBullets.clear();
    
    // Reset invaders but make them faster
    invaderGroup = std::make_shared<InvaderGroup>();
    invaderGroup->createInvaders();
    
    // Return to playing state
    gameState = GameState::PLAYING;
}

void Game::tryPlayerShoot() {
    uint32_t currentTime = SDL_GetTicks();
    
    // Check if player can shoot based on cooldown
    if (player->canShoot(currentTime)) {
        // Create a new bullet
        playerBullets.push_back(player->shoot(currentTime));
        soundGenerator.playSound("player_shoot");
    }
}

void Game::checkCollisions() {
    // Player bullets vs invaders
    for (auto bulletIt = playerBullets.begin(); bulletIt != playerBullets.end();) {
        bool bulletHit = false;
        auto& bullet = *bulletIt;
        
        if (!bullet->isActive()) {
            bulletIt = playerBullets.erase(bulletIt);
            continue;
        }
        
        // Check mystery ship collision
        if (mysteryShip->isActive()) {
            auto mysteryRect = mysteryShip->getCollisionRect();
            auto bulletRect = bullet->getCollisionRect();
            
            if (bulletRect.collidesWith(mysteryRect)) {
                int points = mysteryShip->hit();
                score += points;
                std::cout << "Mystery ship hit! Score: " << score << std::endl;  // Debug output
                bullet->deactivate();
                bulletHit = true;
                addExplosion(mysteryShip->getCollisionRect().x, mysteryShip->getCollisionRect().y);
                soundGenerator.stopSound("mystery_ship");
                soundGenerator.playSound("mystery_ship_hit");
            }
        }
        
        // Check invader collisions
        if (!bulletHit) {
            for (const auto& invader : invaderGroup->getInvaders()) {
                if (!invader->isAlive()) {
                    continue;
                }
                
                auto invaderRect = invader->getCollisionRect();
                auto bulletRect = bullet->getCollisionRect();
                
                if (bulletRect.collidesWith(invaderRect)) {
                    invader->kill();
                    invaderGroup->invaderKilled();
                    int points = invader->getPoints();
                    score += points;
                    std::cout << "Invader hit! Type: " << invader->getType() 
                              << ", Points: " << points 
                              << ", Total Score: " << score << std::endl;  // Debug output
                    bullet->deactivate();
                    bulletHit = true;
                    addExplosion(invader->getCollisionRect().x, invader->getCollisionRect().y);
                    soundGenerator.playSound("invader_explosion");
                    break;
                }
            }
        }
        
        // Check barrier collisions
        if (!bulletHit) {
            for (const auto& barrier : barriers) {
                if (barrier->checkCollision(bullet->getCollisionRect())) {
                    barrier->damage(bullet->getCollisionRect());
                    bullet->deactivate();
                    bulletHit = true;
                    break;
                }
            }
        }
        
        if (bulletHit) {
            bulletIt = playerBullets.erase(bulletIt);
        } else {
            ++bulletIt;
        }
    }
    
    // Invader bullets vs player
    if (player->isAlive()) {
        auto playerRect = player->getCollisionRect();
        
        for (auto bulletIt = invaderBullets.begin(); bulletIt != invaderBullets.end();) {
            auto& bullet = *bulletIt;
            
            if (!bullet->isActive()) {
                bulletIt = invaderBullets.erase(bulletIt);
                continue;
            }
            
            auto bulletRect = bullet->getCollisionRect();
            
            if (bulletRect.collidesWith(playerRect)) {
                player->hit();
                bullet->deactivate();
                bulletIt = invaderBullets.erase(bulletIt);
                addExplosion(player->getCollisionRect().x, player->getCollisionRect().y);
                soundGenerator.playSound("player_explosion");
                
                // Check if game over
                if (player->getLives() <= 0) {
                    gameOver();
                } else {
                    // Respawn player after a delay
                    SDL_Delay(1000);
                    int playerX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
                    int playerY = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - PLAYER_HEIGHT - 20;
                    player->resetPosition(playerX, playerY);
                }
                
                break;
            } else {
                ++bulletIt;
            }
        }
    }
    
    // Invader bullets vs barriers
    for (auto bulletIt = invaderBullets.begin(); bulletIt != invaderBullets.end();) {
        bool bulletHit = false;
        auto& bullet = *bulletIt;
        
        if (!bullet->isActive()) {
            bulletIt = invaderBullets.erase(bulletIt);
            continue;
        }
        
        for (const auto& barrier : barriers) {
            if (barrier->checkCollision(bullet->getCollisionRect())) {
                barrier->damage(bullet->getCollisionRect());
                bullet->deactivate();
                bulletHit = true;
                break;
            }
        }
        
        if (bulletHit) {
            bulletIt = invaderBullets.erase(bulletIt);
        } else {
            ++bulletIt;
        }
    }
}

void Game::addExplosion(int x, int y) {
    int size = 40;
    Explosion explosion;
    explosion.sprite = GraphicsGenerator::createExplosion(size);
    explosion.x = x;
    explosion.y = y;
    explosion.startTime = SDL_GetTicks();
    explosions.push_back(explosion);
}

void Game::drawScore() {
    // Draw score and high score at the top of the screen
    auto scoreText = GraphicsGenerator::createTextSurface("SCORE", FONT_SMALL);
    
    int textWidth, textHeight;
    SDL_QueryTexture(scoreText.get(), nullptr, nullptr, &textWidth, &textHeight);
    
    SDL_Rect scoreTextRect = {50, 2, textWidth, textHeight};
    SDL_RenderCopy(renderer, scoreText.get(), nullptr, &scoreTextRect);
    
    // Draw "HIGH SCORE" text
    auto highScoreText = GraphicsGenerator::createTextSurface("HIGH SCORE", FONT_SMALL);
    
    SDL_QueryTexture(highScoreText.get(), nullptr, nullptr, &textWidth, &textHeight);
    int highScoreTextX = SCREEN_WIDTH - 50 - textWidth;
    
    SDL_Rect highScoreTextRect = {highScoreTextX, 2, textWidth, textHeight};
    SDL_RenderCopy(renderer, highScoreText.get(), nullptr, &highScoreTextRect);
    
    // Draw score numbers
    drawDigits(std::to_string(score), 50, 20);
    
    // Draw high score numbers
    drawDigits(std::to_string(highScore), SCREEN_WIDTH - 50 - 4 * 20, 20);
}

void Game::drawDigits(const std::string& numberStr, int x, int y) {
    int digitWidth = 22;  // Width of each digit including spacing
    
    for (size_t i = 0; i < numberStr.length(); i++) {
        if (numberStr[i] >= '0' && numberStr[i] <= '9') {
            int digitIndex = numberStr[i] - '0';
            
            if (digitIndex >= 0 && digitIndex < digitSprites.size()) {
                int digitX = x + i * digitWidth;
                
                int width, height;
                SDL_QueryTexture(digitSprites[digitIndex].get(), nullptr, nullptr, &width, &height);
                
                SDL_Rect digitRect = {digitX, y, width, height};
                SDL_RenderCopy(renderer, digitSprites[digitIndex].get(), nullptr, &digitRect);
            }
        }
    }
}

void Game::drawLives() {
    // Draw the player's remaining lives
    auto livesText = GraphicsGenerator::createTextSurface("LIVES:", FONT_SMALL);
    
    int textWidth, textHeight;
    SDL_QueryTexture(livesText.get(), nullptr, nullptr, &textWidth, &textHeight);
    
    SDL_Rect livesTextRect = {50, SCREEN_HEIGHT - 40, textWidth, textHeight};
    SDL_RenderCopy(renderer, livesText.get(), nullptr, &livesTextRect);
    
    // Draw small player ships for each life
    int shipWidth = PLAYER_WIDTH / 2;
    int shipHeight = PLAYER_HEIGHT / 2;
    
    for (int i = 0; i < player->getLives(); i++) {
        int shipX = 120 + (i * (shipWidth + 10));
        int shipY = SCREEN_HEIGHT - 40;
        
        auto shipSprite = GraphicsGenerator::createPlayerShip();
        SDL_Rect shipRect = {shipX, shipY, shipWidth, shipHeight};
        SDL_RenderCopy(renderer, shipSprite.get(), nullptr, &shipRect);
    }
}

void Game::updateHighScoreEntry() {
    // Implementation simplified for this draft
}

void Game::submitHighScore() {
    std::string name(playerName.begin(), playerName.end());
    highScoreManager.addScore(name, score, level);
    gameState = GameState::HALL_OF_FAME;
    scrollPosition = 0;
    scrollTimer = SDL_GetTicks();
}

void Game::updateHallOfFame() {
    // Implementation simplified for this draft
}

} // namespace SpaceInvaders
