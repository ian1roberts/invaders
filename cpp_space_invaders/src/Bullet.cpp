#include "space_invaders/Bullet.hpp"
#include "space_invaders/GraphicsGenerator.hpp"
#include "space_invaders/Constants.hpp"

namespace SpaceInvaders {

Bullet::Bullet(int x, int y, int width, int height, int speed)
    : Entity(x, y, width, height),
      speed(speed),
      active(true) {
}

void Bullet::update(float deltaTime) {
    // Base update does nothing, specific bullets will override
}

void Bullet::draw(SDL_Renderer* renderer) {
    if (active) {
        Entity::draw(renderer);
    }
}

// PlayerBullet implementation
PlayerBullet::PlayerBullet(int x, int y)
    : Bullet(x, y, PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT, PLAYER_BULLET_SPEED) {
    sprite = GraphicsGenerator::createBullet(0);  // 0 for player bullet
}

void PlayerBullet::update(float deltaTime) {
    if (!active) {
        return;
    }
    
    y -= speed;
    rect.y = y;
    
    // Deactivate if it goes off the top of the screen
    if (y < 0) {
        active = false;
    }
}

void PlayerBullet::draw(SDL_Renderer* renderer) {
    if (active) {
        Entity::draw(renderer);
    }
}

// InvaderBullet implementation
InvaderBullet::InvaderBullet(int x, int y)
    : Bullet(x, y, INVADER_BULLET_WIDTH, INVADER_BULLET_HEIGHT, INVADER_BULLET_SPEED) {
    sprite = GraphicsGenerator::createBullet(1);  // 1 for invader bullet
}

void InvaderBullet::update(float deltaTime) {
    if (!active) {
        return;
    }
    
    y += speed;
    rect.y = y;
    
    // Deactivate if it goes off the bottom of the screen
    if (y > SCREEN_HEIGHT) {
        active = false;
    }
}

void InvaderBullet::draw(SDL_Renderer* renderer) {
    if (active) {
        Entity::draw(renderer);
    }
}

} // namespace SpaceInvaders
