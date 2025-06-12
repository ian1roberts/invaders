#include "space_invaders/MysteryShip.hpp"
#include "space_invaders/GraphicsGenerator.hpp"
#include <random>

namespace SpaceInvaders {

MysteryShip::MysteryShip()
    : Entity(0, GAME_AREA_MARGIN_Y + 20, MYSTERY_SHIP_WIDTH, MYSTERY_SHIP_HEIGHT),
      speed(MYSTERY_SHIP_SPEED),
      points{MYSTERY_SHIP_POINTS},
      active(false),
      direction(1),
      rng(std::random_device{}()) {
    sprite = GraphicsGenerator::createMysteryShip();
}

void MysteryShip::update(float deltaTime) {
    if (!active) {
        return;
    }
    
    x += direction * speed;
    rect.x = x;
}

void MysteryShip::draw(SDL_Renderer* renderer) {
    if (active) {
        Entity::draw(renderer);
    }
}

void MysteryShip::activate(int screenWidth) {
    if (active) {
        return;
    }
    
    active = true;
    
    std::uniform_int_distribution<> dirDist(0, 1);
    direction = dirDist(rng) == 0 ? -1 : 1;
    
    if (direction > 0) {
        // Moving right, start at left edge
        x = -width;
    } else {
        // Moving left, start at right edge
        x = screenWidth;
    }
    
    rect.x = x;
    rect.y = y;
}

void MysteryShip::update(int screenWidth) {
    if (!active) {
        return;
    }
    
    x += direction * speed;
    rect.x = x;
    
    // Deactivate if gone off screen
    if ((direction > 0 && x > screenWidth) || 
        (direction < 0 && x < -width)) {
        active = false;
    }
}

int MysteryShip::hit() {
    active = false;
    
    // Return random points value
    std::uniform_int_distribution<> pointsDist(0, points.size() - 1);
    return points[pointsDist(rng)];
}

bool MysteryShip::isVisibleOnScreen(int screenWidth) const {
    // Ship is visible if it's active and at least partially on screen
    return active && 
           ((x + width > 0) && (x < screenWidth));
}

} // namespace SpaceInvaders
