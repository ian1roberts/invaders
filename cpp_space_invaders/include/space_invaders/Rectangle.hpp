#pragma once

namespace SpaceInvaders {

class Rectangle {
public:
    Rectangle() = default;
    Rectangle(int x, int y, int width, int height) 
        : x(x), y(y), width(width), height(height) {}

    bool collidesWith(const Rectangle& other) const {
        return (x < other.x + other.width &&
                x + width > other.x &&
                y < other.y + other.height &&
                y + height > other.y);
    }

    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

} // namespace SpaceInvaders
