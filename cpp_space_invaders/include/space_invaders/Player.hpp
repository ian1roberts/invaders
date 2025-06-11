#pragma once

#include "Entity.hpp"
#include "Constants.hpp"
#include <memory>

namespace SpaceInvaders {

class Bullet;

class Player : public Entity {
public:
    Player(int x, int y);
    ~Player() override = default;
    
    void update(float deltaTime) override;
    void draw(SDL_Renderer* renderer) override;
    
    void move(int direction, const Rectangle& gameArea);
    bool canShoot(uint32_t currentTime) const;
    std::shared_ptr<Bullet> shoot(uint32_t currentTime);
    void hit();
    void resetPosition(int x, int y);
    
    bool isAlive() const { return alive; }
    int getLives() const { return lives; }
    
private:
    int speed;
    int lives;
    bool alive;
    uint32_t lastShotTime;
};

} // namespace SpaceInvaders
