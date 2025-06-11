#pragma once

#include "Entity.hpp"
#include "Rectangle.hpp"
#include <map>
#include <utility>

namespace SpaceInvaders {

class Barrier : public Entity {
public:
    Barrier(int x, int y);
    ~Barrier() override = default;
    
    void draw(SDL_Renderer* renderer) override;
    
    bool checkCollision(const Rectangle& rect);
    void damage(const Rectangle& rect);
    
private:
    std::map<std::pair<int, int>, int> damageMap; // Maps (x, y) piece coordinates to damage level
};

} // namespace SpaceInvaders

