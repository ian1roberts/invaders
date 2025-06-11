#include "space_invaders/Barrier.hpp"
#include "space_invaders/GraphicsGenerator.hpp"
#include <SDL2/SDL.h>

namespace SpaceInvaders {

Barrier::Barrier(int x, int y)
    : Entity(x, y, BARRIER_WIDTH, BARRIER_HEIGHT) {
    sprite = GraphicsGenerator::createBarrier();
}

void Barrier::draw(SDL_Renderer* renderer) {
    Entity::draw(renderer);
}

bool Barrier::checkCollision(const Rectangle& rect) {
    // Convert rect to barrier-local coordinates
    int localX = rect.x - x;
    int localY = rect.y - y;
    
    // Don't check outside barrier bounds
    if (localX < 0 || localX >= width || 
        localY < 0 || localY >= height) {
        return false;
    }
    
    // Check if this section is already fully damaged
    int pieceSize = BARRIER_PIECE_SIZE;
    for (int y = localY; y < std::min(localY + rect.height, height); y += pieceSize) {
        for (int x = localX; x < std::min(localX + rect.width, width); x += pieceSize) {
            // Get the pixel at this position
            std::pair<int, int> pieceKey = {x / pieceSize, y / pieceSize};
            if (damageMap.find(pieceKey) == damageMap.end()) {
                // Check if this piece exists in the sprite (not transparent)
                // We need access to the texture's pixels which is tricky with SDL2
                // For simplicity, we'll assume collision if not in damage map
                return true;
            }
        }
    }
    
    return false;
}

void Barrier::damage(const Rectangle& rect) {
    // Convert rect to barrier-local coordinates
    int localX = rect.x - x;
    int localY = rect.y - y;
    
    // Don't damage outside barrier bounds
    if (localX < 0 || localX >= width || 
        localY < 0 || localY >= height) {
        return;
    }
    
    // Damage pieces in the collision area
    int pieceSize = BARRIER_PIECE_SIZE;
    int damageRadius = 2;  // How many pieces around the hit to damage
    
    int centerX = localX + rect.width / 2;
    int centerY = localY + rect.height / 2;
    int centerPieceX = centerX / pieceSize;
    int centerPieceY = centerY / pieceSize;
    
    for (int dy = -damageRadius; dy <= damageRadius; dy++) {
        for (int dx = -damageRadius; dx <= damageRadius; dx++) {
            int pieceX = centerPieceX + dx;
            int pieceY = centerPieceY + dy;
            
            // Skip if outside barrier
            if (pieceX < 0 || pieceX >= width / pieceSize ||
                pieceY < 0 || pieceY >= height / pieceSize) {
                continue;
            }
            
            std::pair<int, int> pieceKey = {pieceX, pieceY};
            
            // Damage the piece if it's not already fully damaged
            int damageLevel = damageMap[pieceKey];
            if (damageLevel < BARRIER_DAMAGE_LEVELS) {
                damageMap[pieceKey] = damageLevel + 1;
            }
            
            // Update the sprite texture if fully damaged
            if (damageMap[pieceKey] >= BARRIER_DAMAGE_LEVELS) {
                // In a full implementation, we would modify the sprite texture
                // to "erase" the damaged piece, but this requires more complex
                // SDL2 texture manipulation that would clutter this example
            }
        }
    }
    
    // In a full implementation, we would update the sprite texture here
    // For now, we'll rely on the damage map for collision detection
}

} // namespace SpaceInvaders
