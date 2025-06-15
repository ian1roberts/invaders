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
            // This creates the iconic sound pattern effect
            soundGenerator.playSound("invader_movement" + std::to_string(invaderMovementSoundIndex));
            invaderMovementSoundIndex = (invaderMovementSoundIndex + 1) % 4;
            lastInvaderMovementSound = currentTime;
        } else {
            // Play regular movement sound - changing the rhythm based on how many are left
            soundGenerator.playSound("invader_movement" + std::to_string(invaderMovementSoundIndex));
            invaderMovementSoundIndex = (invaderMovementSoundIndex + 1) % 4;
            lastInvaderMovementSound = currentTime;
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
        // Draw high score entry screen
        auto entryText = GraphicsGenerator::createTextSurface("NEW HIGH SCORE!", FONT_LARGE);
        
        int entryWidth, entryHeight;
        SDL_QueryTexture(entryText.get(), nullptr, nullptr, &entryWidth, &entryHeight);
        int entryX = SCREEN_WIDTH / 2 - entryWidth / 2;
        
        SDL_Rect entryRect = {entryX, SCREEN_HEIGHT / 4, entryWidth, entryHeight};
        SDL_RenderCopy(renderer, entryText.get(), nullptr, &entryRect);
        
        // Draw score
        auto scoreText = GraphicsGenerator::createTextSurface("YOUR SCORE: " + std::to_string(score), FONT_MEDIUM);
        int scoreWidth, scoreHeight;
        SDL_QueryTexture(scoreText.get(), nullptr, nullptr, &scoreWidth, &scoreHeight);
        int scoreX = SCREEN_WIDTH / 2 - scoreWidth / 2;
        
        SDL_Rect scoreRect = {scoreX, SCREEN_HEIGHT / 4 + 60, scoreWidth, scoreHeight};
        SDL_RenderCopy(renderer, scoreText.get(), nullptr, &scoreRect);
        
        // Draw name entry instructions
        auto instructText = GraphicsGenerator::createTextSurface("ENTER YOUR NAME:", FONT_MEDIUM);
        int instWidth, instHeight;
        SDL_QueryTexture(instructText.get(), nullptr, nullptr, &instWidth, &instHeight);
        int instX = SCREEN_WIDTH / 2 - instWidth / 2;
        
        SDL_Rect instRect = {instX, SCREEN_HEIGHT / 2, instWidth, instHeight};
        SDL_RenderCopy(renderer, instructText.get(), nullptr, &instRect);
        
        // Draw name entry field
        int nameY = SCREEN_HEIGHT / 2 + 60;
        int charWidth = 40;
        int totalWidth = HIGH_SCORE_NAME_LENGTH * charWidth;
        int nameX = SCREEN_WIDTH / 2 - totalWidth / 2;
        
        for (int i = 0; i < HIGH_SCORE_NAME_LENGTH; i++) {
            // Create character texture
            std::string charStr(1, playerName[i]);
            auto charText = GraphicsGenerator::createTextSurface(charStr, FONT_LARGE);
            
            int charTextWidth, charTextHeight;
            SDL_QueryTexture(charText.get(), nullptr, nullptr, &charTextWidth, &charTextHeight);
            
            int charX = nameX + i * charWidth + (charWidth / 2 - charTextWidth / 2);
            SDL_Rect charRect = {charX, nameY, charTextWidth, charTextHeight};
            SDL_RenderCopy(renderer, charText.get(), nullptr, &charRect);
            
            // Draw highlight for selected character
            if (i == currentChar) {
                SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
                SDL_Rect highlightRect = {nameX + i * charWidth, nameY + charTextHeight + 4, charWidth, 2};
                SDL_RenderFillRect(renderer, &highlightRect);
            }
        }
        
        // Draw control instructions
        auto controlText = GraphicsGenerator::createTextSurface("USE ARROWS TO SELECT LETTERS, ENTER/SPACE TO CONFIRM", FONT_SMALL);
        int controlWidth, controlHeight;
        SDL_QueryTexture(controlText.get(), nullptr, nullptr, &controlWidth, &controlHeight);
        int controlX = SCREEN_WIDTH / 2 - controlWidth / 2;
        
        SDL_Rect controlRect = {controlX, SCREEN_HEIGHT - 80, controlWidth, controlHeight};
        SDL_RenderCopy(renderer, controlText.get(), nullptr, &controlRect);
    } else if (gameState == GameState::HALL_OF_FAME) {
        // Draw random stars in background for visual effect
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        for (int i = 0; i < 40; i++) {
            int x = rand() % SCREEN_WIDTH;
            int y = rand() % SCREEN_HEIGHT;
            int size = rand() % 2 + 1;
            SDL_Rect starRect = {x, y, size, size};
            SDL_RenderFillRect(renderer, &starRect);
        }
        
        // Draw title
        auto titleText = GraphicsGenerator::createTextSurface("* HALL OF FAME *", FONT_LARGE);
        
        int titleWidth, titleHeight;
        SDL_QueryTexture(titleText.get(), nullptr, nullptr, &titleWidth, &titleHeight);
        int titleX = SCREEN_WIDTH / 2 - titleWidth / 2;
        
        SDL_Rect titleRect = {titleX, 50, titleWidth, titleHeight};
        SDL_RenderCopy(renderer, titleText.get(), nullptr, &titleRect);
        
        // Draw underline
        SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
        SDL_Rect underlineRect = {titleX, 50 + titleHeight + 4, titleWidth, 2};
        SDL_RenderFillRect(renderer, &underlineRect);
        
        // Draw scrolling banner
        auto bannerText = GraphicsGenerator::createTextSurface("CONGRATULATIONS ON YOUR HIGH SCORE!", FONT_MEDIUM);
        
        int bannerWidth, bannerHeight;
        SDL_QueryTexture(bannerText.get(), nullptr, nullptr, &bannerWidth, &bannerHeight);
        
        SDL_Rect bannerRect = {scrollPosition, 100, bannerWidth, bannerHeight};
        SDL_RenderCopy(renderer, bannerText.get(), nullptr, &bannerRect);
        
        // Draw column headers
        auto rankHeader = GraphicsGenerator::createTextSurface("RANK", FONT_SMALL);
        auto nameHeader = GraphicsGenerator::createTextSurface("NAME", FONT_SMALL);
        auto scoreHeader = GraphicsGenerator::createTextSurface("SCORE", FONT_SMALL);
        auto levelHeader = GraphicsGenerator::createTextSurface("LEVEL", FONT_SMALL);
        
        int headerY = 150;
        int rankX = 100;
        int nameX = 200;
        int scoreX = 400;
        int levelX = 550;
        
        SDL_Rect rankHeaderRect = {rankX, headerY, 0, 0};
        SDL_QueryTexture(rankHeader.get(), nullptr, nullptr, &rankHeaderRect.w, &rankHeaderRect.h);
        SDL_RenderCopy(renderer, rankHeader.get(), nullptr, &rankHeaderRect);
        
        SDL_Rect nameHeaderRect = {nameX, headerY, 0, 0};
        SDL_QueryTexture(nameHeader.get(), nullptr, nullptr, &nameHeaderRect.w, &nameHeaderRect.h);
        SDL_RenderCopy(renderer, nameHeader.get(), nullptr, &nameHeaderRect);
        
        SDL_Rect scoreHeaderRect = {scoreX, headerY, 0, 0};
        SDL_QueryTexture(scoreHeader.get(), nullptr, nullptr, &scoreHeaderRect.w, &scoreHeaderRect.h);
        SDL_RenderCopy(renderer, scoreHeader.get(), nullptr, &scoreHeaderRect);
        
        SDL_Rect levelHeaderRect = {levelX, headerY, 0, 0};
        SDL_QueryTexture(levelHeader.get(), nullptr, nullptr, &levelHeaderRect.w, &levelHeaderRect.h);
        SDL_RenderCopy(renderer, levelHeader.get(), nullptr, &levelHeaderRect);
        
        // Draw header underline
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
        SDL_Rect headerUnderline = {rankX, headerY + rankHeaderRect.h + 2, levelX + levelHeaderRect.w - rankX, 1};
        SDL_RenderFillRect(renderer, &headerUnderline);
        
        // Draw high score entries
        const auto& highScores = highScoreManager.getHighScores();
        int entryY = headerY + rankHeaderRect.h + 15;
        int entrySpacing = 30;
        
        for (size_t i = 0; i < highScores.size(); i++) {
            // Highlight user's new score
            if (score > 0 && 
                highScores[i].name == std::string(playerName.begin(), playerName.end()) && 
                highScores[i].score == score && 
                highScores[i].level == level) {
                SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
            } else {
                SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
            }
            
            // Draw rank
            auto rankText = GraphicsGenerator::createTextSurface(std::to_string(i + 1), FONT_SMALL);
            SDL_Rect rankRect = {rankX, entryY, 0, 0};
            SDL_QueryTexture(rankText.get(), nullptr, nullptr, &rankRect.w, &rankRect.h);
            SDL_RenderCopy(renderer, rankText.get(), nullptr, &rankRect);
            
            // Draw name
            auto nameText = GraphicsGenerator::createTextSurface(highScores[i].name, FONT_SMALL);
            SDL_Rect nameRect = {nameX, entryY, 0, 0};
            SDL_QueryTexture(nameText.get(), nullptr, nullptr, &nameRect.w, &nameRect.h);
            SDL_RenderCopy(renderer, nameText.get(), nullptr, &nameRect);
            
            // Draw score
            auto scoreText = GraphicsGenerator::createTextSurface(std::to_string(highScores[i].score), FONT_SMALL);
            SDL_Rect scoreRect = {scoreX, entryY, 0, 0};
            SDL_QueryTexture(scoreText.get(), nullptr, nullptr, &scoreRect.w, &scoreRect.h);
            SDL_RenderCopy(renderer, scoreText.get(), nullptr, &scoreRect);
            
            // Draw level
            auto levelText = GraphicsGenerator::createTextSurface(std::to_string(highScores[i].level), FONT_SMALL);
            SDL_Rect levelRect = {levelX, entryY, 0, 0};
            SDL_QueryTexture(levelText.get(), nullptr, nullptr, &levelRect.w, &levelRect.h);
            SDL_RenderCopy(renderer, levelText.get(), nullptr, &levelRect);
            
            entryY += entrySpacing;
        }
        
        // Draw instruction text
        auto instructText = GraphicsGenerator::createTextSurface("PRESS ENTER TO PLAY AGAIN", FONT_MEDIUM);
        int instructWidth, instructHeight;
        SDL_QueryTexture(instructText.get(), nullptr, nullptr, &instructWidth, &instructHeight);
        int instructX = SCREEN_WIDTH / 2 - instructWidth / 2;
        SDL_Rect instructRect;
        
        // Only show the instruction text blinking
        if ((SDL_GetTicks() / 800) % 2 == 0) {
            instructRect = {instructX, SCREEN_HEIGHT - 100, instructWidth, instructHeight};
            SDL_RenderCopy(renderer, instructText.get(), nullptr, &instructRect);
        } else {
            // Still need height for positioning the reset text
            instructRect.h = instructHeight;
        }
        
        // Draw reset instruction below the "PRESS ENTER TO PLAY AGAIN" text
        auto resetText = GraphicsGenerator::createTextSurface("PRESS Q TO RESET HIGH SCORES", FONT_SMALL);
        int resetWidth, resetHeight;
        SDL_QueryTexture(resetText.get(), nullptr, nullptr, &resetWidth, &resetHeight);
        int resetX = SCREEN_WIDTH / 2 - resetWidth / 2;
        int resetY = SCREEN_HEIGHT - 125 + instructRect.h + 15; // Position below the play again text with spacing
        
        SDL_Rect resetRect = {resetX, resetY, resetWidth, resetHeight};
        SDL_RenderCopy(renderer, resetText.get(), nullptr, &resetRect);
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
    
    // Store current player lives and position
    int currentLives = player->getLives();
    
    // Reset invaders but make them faster
    invaderGroup = std::make_shared<InvaderGroup>();
    invaderGroup->createInvaders();
    
    // Create player with preserved lives
    int playerX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
    int playerY = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - PLAYER_HEIGHT - 20;
    player = std::make_shared<Player>(playerX, playerY);
    player->setLives(currentLives); // Set the stored lives
    
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
    int digitWidth = 20;  // Slightly smaller digit width
    
    // First, draw a black background for better readability
    int totalWidth = numberStr.length() * digitWidth;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Pure black background
    SDL_Rect bgRect = {x - 5, y - 2, totalWidth + 10, 30};  // Slightly smaller height
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw a subtle border around the score display
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255); // Dark gray border
    SDL_RenderDrawRect(renderer, &bgRect);
    
    // Draw each digit with improved spacing
    for (size_t i = 0; i < numberStr.length(); i++) {
        if (numberStr[i] >= '0' && numberStr[i] <= '9') {
            int digitIndex = numberStr[i] - '0';
            
            if (digitIndex >= 0 && digitIndex < digitSprites.size()) {
                int digitX = x + i * digitWidth;
                
                int width, height;
                SDL_QueryTexture(digitSprites[digitIndex].get(), nullptr, nullptr, &width, &height);
                
                // Calculate slightly smaller size for rendering
                int renderWidth = static_cast<int>(width * 0.9);  // 90% of original size
                int renderHeight = static_cast<int>(height * 0.9);
                
                // Draw the digit itself with green tint
                SDL_SetTextureColorMod(digitSprites[digitIndex].get(), 0, 255, 0);  // Set to green
                SDL_Rect digitRect = {digitX, y, renderWidth, renderHeight};
                SDL_RenderCopy(renderer, digitSprites[digitIndex].get(), nullptr, &digitRect);
                SDL_SetTextureColorMod(digitSprites[digitIndex].get(), 255, 255, 255);  // Reset to white for other uses
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
    uint32_t currentTime = SDL_GetTicks();
    
    // Auto-cycle characters when key is held down
    if (currentTime - nameEntryCooldown > nameEntryDelay) {
        const uint8_t* keyState = SDL_GetKeyboardState(nullptr);
        bool keyPressed = false;
        
        if (keyState[SDL_SCANCODE_UP]) {
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
            keyPressed = true;
        }
        else if (keyState[SDL_SCANCODE_DOWN]) {
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
            keyPressed = true;
        }
        
        if (keyPressed) {
            nameEntryCooldown = currentTime;
        }
    }
}

void Game::submitHighScore() {
    std::string name(playerName.begin(), playerName.end());
    highScoreManager.addScore(name, score, level);
    gameState = GameState::HALL_OF_FAME;
    scrollPosition = 0;
    scrollTimer = SDL_GetTicks();
}

void Game::updateHallOfFame() {
    uint32_t currentTime = SDL_GetTicks();
    
    // Update scrolling banner position
    if (currentTime - scrollTimer > 16) {  // ~60 FPS
        scrollPosition -= scrollSpeed;
        
        // Reset when text scrolls off screen (assuming banner is 800px wide)
        if (scrollPosition < -800) {
            scrollPosition = SCREEN_WIDTH;
        }
        
        scrollTimer = currentTime;
    }
}

} // namespace SpaceInvaders
