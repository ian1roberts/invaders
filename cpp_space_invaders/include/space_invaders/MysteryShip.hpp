#pragma once

#include "Entity.hpp"
#include <array>
#include <random>

namespace SpaceInvaders {

class MysteryShip : public Entity {
public:
    MysteryShip();
    ~MysteryShip() override = default;
    
    void update(float deltaTime) override;
    void draw(SDL_Renderer* renderer) override;
    
    void activate(int screenWidth);
    void update(int screenWidth);
    int hit();
    
    bool isActive() const { return active; }
    bool isVisibleOnScreen(int screenWidth) const; // New method to check if ship is visible
    
private:
    int speed;
    std::array<int, 4> points;
    bool active;
    int direction;
    
    std::mt19937 rng; // Random number generator
};

} // namespace SpaceInvaders
