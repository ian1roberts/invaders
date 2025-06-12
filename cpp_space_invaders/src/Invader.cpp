#include "space_invaders/Invader.hpp"
#include "space_invaders/GraphicsGenerator.hpp"
#include <random>
#include <algorithm>
#include <map>

namespace SpaceInvaders {

// Invader implementation
Invader::Invader(int x, int y, int invaderType, int row, int col)
    : Entity(x, y, INVADER_WIDTH, INVADER_HEIGHT),
      type(invaderType),
      row(row),
      col(col),
      alive(true) {
    sprite = GraphicsGenerator::createInvader(invaderType);
    spriteAlt = GraphicsGenerator::createAlternateFrame(sprite);
    currentSprite = sprite;
}

void Invader::update(float deltaTime) {
    // Invader movement is handled by InvaderGroup
}

void Invader::draw(SDL_Renderer* renderer) {
    if (alive) {
        SDL_Rect destRect = {x, y, width, height};
        SDL_RenderCopy(renderer, currentSprite.get(), nullptr, &destRect);
    }
}

void Invader::move(int dx, int dy) {
    if (!alive) {
        return;
    }
    
    x += dx;
    y += dy;
    rect.x = x;
    rect.y = y;
}

void Invader::animate(int frame) {
    if (!alive) {
        return;
    }
    
    if (frame % 2 == 0) {
        currentSprite = sprite;
    } else {
        currentSprite = spriteAlt;
    }
}

int Invader::getPoints() const {
    switch (type) {
    case 0:  // Top row
        return SCORE_INVADER_TOP_ROW;
    case 1:  // Middle rows
        return SCORE_INVADER_MIDDLE_ROW;
    default:  // Bottom rows
        return SCORE_INVADER_BOTTOM_ROW;
    }
}

// InvaderGroup implementation
InvaderGroup::InvaderGroup()
    : speed(INVADER_MOVE_SPEED_H),
      direction(1),  // 1 for right, -1 for left
      moveDown(false),
      frame(0),
      lastMoveTime(0),
      moveDelay(1000),  // Start with 1000ms delay
      invadersKilled(0),
      totalInvaders(INVADER_ROWS * INVADERS_PER_ROW) {
}

void InvaderGroup::createInvaders() {
    invaders.clear();
    
    int startX = GAME_AREA_MARGIN_X + INVADER_H_PADDING;
    int startY = GAME_AREA_MARGIN_Y + INVADER_V_PADDING;
    
    for (int row = 0; row < INVADER_ROWS; row++) {
        int invaderType = (row == 0) ? 0 : ((row < 3) ? 1 : 2);
        int y = startY + row * (INVADER_HEIGHT + INVADER_V_SPACING);
        
        for (int col = 0; col < INVADERS_PER_ROW; col++) {
            int x = startX + col * (INVADER_WIDTH + INVADER_H_SPACING);
            invaders.push_back(std::make_shared<Invader>(x, y, invaderType, row, col));
        }
    }
}

bool InvaderGroup::move(uint32_t currentTime, const Rectangle& gameArea) {
    // Don't move if it's not time yet
    if (currentTime - lastMoveTime < moveDelay) {
        return false;
    }
    
    lastMoveTime = currentTime;
    frame++;
    
    bool movedDown = false;
    int dx = direction * speed;
    int dy = moveDown ? INVADER_MOVE_DOWN : 0;
    
    if (moveDown) {
        movedDown = true;
        moveDown = false;
    }
    
    // Check if any invader would hit the edge after moving
    bool needDirectionChange = false;
    for (const auto& invader : invaders) {
        if (!invader->isAlive()) {
            continue;
        }
        
        int newX = invader->getCollisionRect().x + dx;
        if (newX < gameArea.x || 
            newX + INVADER_WIDTH > gameArea.x + gameArea.width) {
            needDirectionChange = true;
            break;
        }
    }
    
    // If hitting edge, prepare to move down on next update
    if (needDirectionChange) {
        direction *= -1;  // Reverse direction
        moveDown = true;
        return false;
    }
    
    // Move all invaders
    for (const auto& invader : invaders) {
        if (invader->isAlive()) {
            invader->move(dx, dy);
            invader->animate(frame);
        }
    }
    
    return movedDown;
}

void InvaderGroup::invaderKilled() {
    invadersKilled++;
    
    // Improve speed increase calculations using a more exponential curve
    // This will create the dramatic acceleration effect of the original
    float remaining = totalInvaders - invadersKilled;
    float percentKilled = 1.0f - (remaining / totalInvaders);
    
    // Exponential speed increase - starts slow, then dramatically speeds up
    // as last invaders remain (matching arcade behavior)
    float speedFactor = std::exp(percentKilled * 2.5f) - 1.0f;
    speedFactor = std::min(speedFactor, 9.0f); // Cap at 9x speed increase
    
    // Calculate delay based on speedFactor - minimum 50ms for extreme difficulty at the end
    moveDelay = std::max(50, static_cast<int>(1000 / (1.0f + speedFactor)));
}

bool InvaderGroup::anyInvaderAtBottom(int bottomY) const {
    for (const auto& invader : invaders) {
        if (invader->isAlive() && invader->getCollisionRect().y + invader->getCollisionRect().height >= bottomY) {
            return true;
        }
    }
    return false;
}

void InvaderGroup::draw(SDL_Renderer* renderer) {
    for (const auto& invader : invaders) {
        invader->draw(renderer);
    }
}

std::shared_ptr<Invader> InvaderGroup::getRandomShooter() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    if (dis(gen) > INVADER_FIRING_CHANCE) {
        return nullptr;
    }
    
    // Get the bottom-most invader in each column
    std::map<int, std::shared_ptr<Invader>> bottomInvaders;
    for (const auto& invader : invaders) {
        if (!invader->isAlive()) {
            continue;
        }
        
        int col = invader->getCol();
        if (bottomInvaders.find(col) == bottomInvaders.end() || 
            invader->getCollisionRect().y > bottomInvaders[col]->getCollisionRect().y) {
            bottomInvaders[col] = invader;
        }
    }
    
    if (bottomInvaders.empty()) {
        return nullptr;
    }
    
    // Select a random bottom invader
    std::uniform_int_distribution<> column_dist(0, bottomInvaders.size() - 1);
    auto it = bottomInvaders.begin();
    std::advance(it, column_dist(gen));
    return it->second;
}

bool InvaderGroup::allDead() const {
    return invadersKilled >= totalInvaders;
}

} // namespace SpaceInvaders
