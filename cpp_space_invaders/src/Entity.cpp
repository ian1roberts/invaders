#include "space_invaders/Entity.hpp"

namespace SpaceInvaders {

Entity::Entity(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height), rect(x, y, width, height) {
}

void Entity::update(float deltaTime) {
    // Base implementation does nothing
}

void Entity::draw(SDL_Renderer* renderer) {
    if (sprite) {
        SDL_Rect destRect = {x, y, width, height};
        SDL_RenderCopy(renderer, sprite.get(), nullptr, &destRect);
    }
}

Rectangle Entity::getCollisionRect() const {
    return Rectangle(x, y, width, height);
}

} // namespace SpaceInvaders
