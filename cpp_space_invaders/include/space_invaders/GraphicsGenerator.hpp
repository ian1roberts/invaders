#pragma once

#include "Constants.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <memory>
#include <vector>
#include <string>

namespace SpaceInvaders {

class GraphicsGenerator {
public:
    static void initialize(SDL_Renderer* renderer);
    static void cleanup();
    
    // Sprite generation methods
    static std::shared_ptr<SDL_Texture> createPlayerShip();
    static std::shared_ptr<SDL_Texture> createInvader(int invaderType);
    static std::shared_ptr<SDL_Texture> createAlternateFrame(std::shared_ptr<SDL_Texture> sprite);
    static std::shared_ptr<SDL_Texture> createMysteryShip();
    static std::shared_ptr<SDL_Texture> createBarrier();
    static std::shared_ptr<SDL_Texture> createBarrierPiece();
    static std::shared_ptr<SDL_Texture> createExplosion(int size);
    static std::shared_ptr<SDL_Texture> createBullet(int bulletType);
    static std::shared_ptr<SDL_Texture> createTextSurface(const std::string& text, int size, const SDL_Color& color = WHITE);
    static std::vector<std::shared_ptr<SDL_Texture>> createDigitSprites();

private:
    static SDL_Renderer* renderer;
    
    // Helper methods
    static std::shared_ptr<SDL_Texture> createTextureFromSurface(SDL_Surface* surface);
    static SDL_Surface* createRGBASurface(int width, int height);
    static void setPixel(SDL_Surface* surface, int x, int y, const SDL_Color& color);
    static SDL_Color getPixel(SDL_Surface* surface, int x, int y);
};

} // namespace SpaceInvaders
