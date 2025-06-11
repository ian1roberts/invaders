#pragma once

#include "Rectangle.hpp"
#include <SDL2/SDL.h>
#include <memory>

namespace SpaceInvaders {

class Entity {
public:
    Entity() = default;
    Entity(int x, int y, int width, int height);
    virtual ~Entity() = default;
    
    virtual void update(float deltaTime);
    virtual void draw(SDL_Renderer* renderer);
    
    Rectangle getCollisionRect() const;
    
protected:
    int x = 0;
    int y = 0; 
    int width = 0;
    int height = 0;
    std::shared_ptr<SDL_Texture> sprite;
    Rectangle rect;
};

} // namespace SpaceInvaders
