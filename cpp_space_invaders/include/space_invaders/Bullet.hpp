#pragma once

#include "Entity.hpp"
#include <memory>

namespace SpaceInvaders {

class Bullet : public Entity {
public:
    Bullet(int x, int y, int width, int height, int speed);
    ~Bullet() override = default;
    
    virtual void update(float deltaTime) override;
    virtual void draw(SDL_Renderer* renderer) override;
    
    bool isActive() const { return active; }
    void deactivate() { active = false; }
    
protected:
    int speed;
    bool active;
};

class PlayerBullet : public Bullet {
public:
    PlayerBullet(int x, int y);
    ~PlayerBullet() override = default;
    
    void update(float deltaTime) override;
    void draw(SDL_Renderer* renderer) override;
};

class InvaderBullet : public Bullet {
public:
    InvaderBullet(int x, int y);
    ~InvaderBullet() override = default;
    
    void update(float deltaTime) override;
    void draw(SDL_Renderer* renderer) override;
};

} // namespace SpaceInvaders

