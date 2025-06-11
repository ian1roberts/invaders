#include "space_invaders/Player.hpp"
#include "space_invaders/Bullet.hpp"
#include "space_invaders/GraphicsGenerator.hpp"

namespace SpaceInvaders {

Player::Player(int x, int y)
    : Entity(x, y, PLAYER_WIDTH, PLAYER_HEIGHT),
      speed(PLAYER_SPEED),
      lives(PLAYER_LIVES),
      alive(true),
      lastShotTime(0) {
    sprite = GraphicsGenerator::createPlayerShip();
}

void Player::update(float deltaTime) {
    // Player movement is handled by input processing
}

void Player::draw(SDL_Renderer* renderer) {
    if (alive) {
        Entity::draw(renderer);
    }
}

void Player::move(int direction, const Rectangle& gameArea) {
    if (!alive) {
        return;
    }
    
    int newX = x + (direction * speed);
    
    // Check boundaries
    if (newX < gameArea.x) {
        newX = gameArea.x;
    } else if (newX + width > gameArea.x + gameArea.width) {
        newX = gameArea.x + gameArea.width - width;
    }
    
    x = newX;
    rect.x = newX;
}

bool Player::canShoot(uint32_t currentTime) const {
    return (currentTime - lastShotTime > PLAYER_BULLET_COOLDOWN);
}

std::shared_ptr<Bullet> Player::shoot(uint32_t currentTime) {
    lastShotTime = currentTime;
    int bulletX = x + (width / 2) - (PLAYER_BULLET_WIDTH / 2);
    int bulletY = y - PLAYER_BULLET_HEIGHT;
    return std::make_shared<PlayerBullet>(bulletX, bulletY);
}

void Player::hit() {
    alive = false;
    lives--;
}

void Player::resetPosition(int x, int y) {
    this->x = x;
    this->y = y;
    rect.x = x;
    rect.y = y;
    alive = true;
}

} // namespace SpaceInvaders
