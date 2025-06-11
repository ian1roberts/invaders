#pragma once

#include "Entity.hpp"
#include <memory>
#include <vector>

namespace SpaceInvaders {

class Invader : public Entity {
public:
    Invader(int x, int y, int invaderType, int row, int col);
    ~Invader() override = default;
    
    void update(float deltaTime) override;
    void draw(SDL_Renderer* renderer) override;
    
    void move(int dx, int dy);
    void animate(int frame);
    int getPoints() const;
    
    bool isAlive() const { return alive; }
    void kill() { alive = false; }
    int getType() const { return type; }
    int getRow() const { return row; }
    int getCol() const { return col; }
    
private:
    int type;
    int row;
    int col;
    bool alive;
    std::shared_ptr<SDL_Texture> sprite;
    std::shared_ptr<SDL_Texture> spriteAlt;
    std::shared_ptr<SDL_Texture> currentSprite;
};

class InvaderGroup {
public:
    InvaderGroup();
    ~InvaderGroup() = default;
    
    void createInvaders();
    bool move(uint32_t currentTime, const Rectangle& gameArea);
    void invaderKilled();
    bool anyInvaderAtBottom(int bottomY) const;
    void draw(SDL_Renderer* renderer);
    std::shared_ptr<Invader> getRandomShooter() const;
    bool allDead() const;
    
    // Add getter for invaders collection
    const std::vector<std::shared_ptr<Invader>>& getInvaders() const { return invaders; }
    
private:
    std::vector<std::shared_ptr<Invader>> invaders;
    int speed;
    int direction;
    bool moveDown;
    int frame;
    uint32_t lastMoveTime;
    int moveDelay;
    int invadersKilled;
    int totalInvaders;
};

} // namespace SpaceInvaders
