#include "space_invaders/GraphicsGenerator.hpp"
#include <SDL2/SDL_ttf.h>
#include <stdexcept>
#include <cmath>
#include <random>

namespace SpaceInvaders {

SDL_Renderer* GraphicsGenerator::renderer = nullptr;

void GraphicsGenerator::initialize(SDL_Renderer* rendererPtr) {
    renderer = rendererPtr;
    
    // Initialize SDL_ttf for text rendering
    if (TTF_Init() < 0) {
        throw std::runtime_error("SDL_ttf could not initialize! Error: " + std::string(TTF_GetError()));
    }
}

void GraphicsGenerator::cleanup() {
    TTF_Quit();
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createPlayerShip() {
    SDL_Surface* surface = createRGBASurface(PLAYER_WIDTH, PLAYER_HEIGHT);
    
    // Draw the cannon base (green rectangle)
    SDL_Rect baseRect = {0, PLAYER_HEIGHT - 10, PLAYER_WIDTH, 10};
    SDL_FillRect(surface, &baseRect, SDL_MapRGBA(surface->format, GREEN.r, GREEN.g, GREEN.b, GREEN.a));
    
    // Draw the cannon (rectangle with a triangle on top)
    SDL_Rect cannonRect = {PLAYER_WIDTH / 2 - 5, 0, 10, PLAYER_HEIGHT - 10};
    SDL_FillRect(surface, &cannonRect, SDL_MapRGBA(surface->format, GREEN.r, GREEN.g, GREEN.b, GREEN.a));
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createInvader(int invaderType) {
    SDL_Surface* surface = createRGBASurface(INVADER_WIDTH, INVADER_HEIGHT);
    
    // Set the drawing color to green
    Uint32 color = SDL_MapRGBA(surface->format, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
    Uint32 blackColor = SDL_MapRGBA(surface->format, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    
    if (invaderType == 0) {  // Top row - squid-like
        // Main body
        SDL_Rect body = {INVADER_WIDTH / 4, INVADER_HEIGHT / 3, 
                         INVADER_WIDTH / 2, INVADER_HEIGHT / 2};
        SDL_FillRect(surface, &body, color);
        
        // Tentacles
        for (int i = 0; i < 3; i++) {
            int xPos = INVADER_WIDTH / 4 + (i * INVADER_WIDTH / 6);
            SDL_Rect tent = {xPos, INVADER_HEIGHT * 5 / 6,
                             INVADER_WIDTH / 12, INVADER_HEIGHT / 6};
            SDL_FillRect(surface, &tent, color);
        }
        
        // Eyes
        int eyeSize = INVADER_WIDTH / 10;
        SDL_Rect leftEye = {INVADER_WIDTH / 3, INVADER_HEIGHT / 3 + eyeSize,
                           eyeSize, eyeSize};
        SDL_Rect rightEye = {INVADER_WIDTH * 2 / 3 - eyeSize, INVADER_HEIGHT / 3 + eyeSize,
                            eyeSize, eyeSize};
        SDL_FillRect(surface, &leftEye, blackColor);
        SDL_FillRect(surface, &rightEye, blackColor);
        
    } else if (invaderType == 1) {  // Middle rows - crab-like
        // Main body
        SDL_Rect body = {INVADER_WIDTH / 6, INVADER_HEIGHT / 4, 
                        INVADER_WIDTH * 2 / 3, INVADER_HEIGHT / 2};
        SDL_FillRect(surface, &body, color);
        
        // Claws
        SDL_Rect leftClaw = {0, INVADER_HEIGHT / 2, 
                            INVADER_WIDTH / 6, INVADER_HEIGHT / 4};
        SDL_Rect rightClaw = {INVADER_WIDTH * 5 / 6, INVADER_HEIGHT / 2, 
                             INVADER_WIDTH / 6, INVADER_HEIGHT / 4};
        SDL_FillRect(surface, &leftClaw, color);
        SDL_FillRect(surface, &rightClaw, color);
        
        // Eyes
        int eyeSize = INVADER_WIDTH / 10;
        SDL_Rect leftEye = {INVADER_WIDTH / 3, INVADER_HEIGHT / 3,
                           eyeSize, eyeSize};
        SDL_Rect rightEye = {INVADER_WIDTH * 2 / 3 - eyeSize, INVADER_HEIGHT / 3,
                            eyeSize, eyeSize};
        SDL_FillRect(surface, &leftEye, blackColor);
        SDL_FillRect(surface, &rightEye, blackColor);
        
    } else {  // Bottom rows - octopus-like
        // Main circular body
        // Since SDL doesn't have circle drawing, we approximate with squares
        int centerX = INVADER_WIDTH / 2;
        int centerY = INVADER_HEIGHT / 2;
        int radius = std::min(INVADER_WIDTH, INVADER_HEIGHT) / 3;
        
        for (int y = centerY - radius; y <= centerY + radius; y++) {
            for (int x = centerX - radius; x <= centerX + radius; x++) {
                if (std::pow(x - centerX, 2) + std::pow(y - centerY, 2) <= std::pow(radius, 2)) {
                    setPixel(surface, x, y, GREEN);
                }
            }
        }
        
        // Tentacles
        for (int i = 0; i < 4; i++) {
            int xOffset = INVADER_WIDTH / 8 + (i * INVADER_WIDTH / 4);
            SDL_Rect tent = {xOffset, INVADER_HEIGHT * 3 / 4,
                            INVADER_WIDTH / 12, INVADER_HEIGHT / 4};
            SDL_FillRect(surface, &tent, color);
        }
        
        // Eyes
        int eyeSize = INVADER_WIDTH / 12;
        int eyeY = centerY - eyeSize / 2;
        SDL_Rect leftEye = {centerX - radius / 2, eyeY, eyeSize, eyeSize};
        SDL_Rect rightEye = {centerX + radius / 2 - eyeSize, eyeY, eyeSize, eyeSize};
        SDL_FillRect(surface, &leftEye, blackColor);
        SDL_FillRect(surface, &rightEye, blackColor);
    }
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createAlternateFrame(std::shared_ptr<SDL_Texture> sprite) {
    // Getting pixel data from texture is complex in SDL2
    // For simplicity, we'll create a slightly modified version
    
    // Get texture width and height
    int width, height;
    SDL_QueryTexture(sprite.get(), nullptr, nullptr, &width, &height);
    
    // Create a temporary render target
    SDL_Texture* tempTarget = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_TARGET, 
        width, 
        height
    );
    
    // Set blend mode to enable alpha
    SDL_SetTextureBlendMode(tempTarget, SDL_BLENDMODE_BLEND);
    
    // Set render target to our temp texture
    SDL_SetRenderTarget(renderer, tempTarget);
    
    // Clear the texture
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    // Draw the original sprite slightly shifted
    SDL_Rect srcRect = {0, 0, width, height};
    SDL_Rect dstRect = {2, 0, width - 2, height};
    SDL_RenderCopy(renderer, sprite.get(), &srcRect, &dstRect);
    
    // Reset render target
    SDL_SetRenderTarget(renderer, nullptr);
    
    return std::shared_ptr<SDL_Texture>(tempTarget, SDL_DestroyTexture);
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createMysteryShip() {
    SDL_Surface* surface = createRGBASurface(MYSTERY_SHIP_WIDTH, MYSTERY_SHIP_HEIGHT);
    
    // Use RED for mystery ship like in original game
    Uint32 color = SDL_MapRGBA(surface->format, RED.r, RED.g, RED.b, RED.a);
    
    // Create UFO shape with simple geometry (oval shape)
    int centerX = MYSTERY_SHIP_WIDTH / 2;
    int centerY = MYSTERY_SHIP_HEIGHT / 2;
    int radiusX = MYSTERY_SHIP_WIDTH / 2;
    int radiusY = MYSTERY_SHIP_HEIGHT / 3;
    
    // Draw oval body
    for (int y = 0; y < MYSTERY_SHIP_HEIGHT; y++) {
        for (int x = 0; x < MYSTERY_SHIP_WIDTH; x++) {
            float normalizedX = static_cast<float>(x - centerX) / radiusX;
            float normalizedY = static_cast<float>(y - centerY) / radiusY;
            
            if (normalizedX * normalizedX + normalizedY * normalizedY <= 1.0f) {
                setPixel(surface, x, y, RED);
            }
        }
    }
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createBarrierPiece() {
    int pieceSize = BARRIER_PIECE_SIZE;
    SDL_Surface* surface = createRGBASurface(pieceSize, pieceSize);
    
    // Fill with green
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, GREEN.r, GREEN.g, GREEN.b, GREEN.a));
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createBarrier() {
    SDL_Surface* surface = createRGBASurface(BARRIER_WIDTH, BARRIER_HEIGHT);
    
    int pieceSize = BARRIER_PIECE_SIZE;
    
    // Draw the main barrier shape (fortress-like)
    for (int x = 0; x < BARRIER_WIDTH / pieceSize; x++) {
        for (int y = 0; y < BARRIER_HEIGHT / pieceSize; y++) {
            // Skip the bottom corners to create an arch
            if (y > BARRIER_HEIGHT / pieceSize * 2 / 3 && 
                (x < BARRIER_WIDTH / pieceSize / 4 || 
                x > BARRIER_WIDTH / pieceSize * 3 / 4)) {
                continue;
            }
            
            // Create the middle arch opening
            if (y > BARRIER_HEIGHT / pieceSize / 2 && 
                x > BARRIER_WIDTH / pieceSize / 3 && 
                x < BARRIER_WIDTH / pieceSize * 2 / 3) {
                continue;
            }
            
            SDL_Rect pieceRect = {x * pieceSize, y * pieceSize, pieceSize, pieceSize};
            SDL_FillRect(surface, &pieceRect, 
                         SDL_MapRGBA(surface->format, GREEN.r, GREEN.g, GREEN.b, GREEN.a));
        }
    }
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createExplosion(int size) {
    SDL_Surface* surface = createRGBASurface(size, size);
    
    // Draw random explosion particles
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> posDist(0, size - 1);
    std::uniform_int_distribution<> radiusDist(1, size / 5);
    std::uniform_int_distribution<> colorDist(0, 2);
    
    const SDL_Color colors[] = {YELLOW, RED, WHITE};
    
    int numParticles = 20;
    for (int i = 0; i < numParticles; i++) {
        int x = posDist(gen);
        int y = posDist(gen);
        int radius = radiusDist(gen);
        SDL_Color color = colors[colorDist(gen)];
        
        // Draw a filled circle
        for (int cy = -radius; cy <= radius; cy++) {
            for (int cx = -radius; cx <= radius; cx++) {
                if (cx*cx + cy*cy <= radius*radius) {
                    int drawX = x + cx;
                    int drawY = y + cy;
                    if (drawX >= 0 && drawX < size && drawY >= 0 && drawY < size) {
                        setPixel(surface, drawX, drawY, color);
                    }
                }
            }
        }
    }
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    return texture;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createBullet(int bulletType) {
    if (bulletType == 0) {  // Player bullet
        SDL_Surface* surface = createRGBASurface(PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT);
        
        // Fill with white
        SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        
        auto texture = createTextureFromSurface(surface);
        SDL_FreeSurface(surface);
        return texture;
        
    } else {  // Invader bullet (zigzag shape)
        SDL_Surface* surface = createRGBASurface(INVADER_BULLET_WIDTH, INVADER_BULLET_HEIGHT);
        
        // Create zigzag pattern
        int xLeft = 0;
        int xRight = INVADER_BULLET_WIDTH;
        int xMid = INVADER_BULLET_WIDTH / 2;
        int segmentHeight = INVADER_BULLET_HEIGHT / 3;
        
        for (int i = 0; i < 3; i++) {
            int y1 = i * segmentHeight;
            int y2 = (i + 1) * segmentHeight;
            
            if (i % 2 == 0) {
                // Draw line from left to middle
                for (int y = y1; y < y2; y++) {
                    double t = (y - y1) / static_cast<double>(y2 - y1);
                    int x = xLeft + static_cast<int>(t * (xMid - xLeft));
                    setPixel(surface, x, y, WHITE);
                }
            } else {
                // Draw line from right to middle
                for (int y = y1; y < y2; y++) {
                    double t = (y - y1) / static_cast<double>(y2 - y1);
                    int x = xRight - static_cast<int>(t * (xRight - xMid));
                    setPixel(surface, x, y, WHITE);
                }
            }
        }
        
        auto texture = createTextureFromSurface(surface);
        SDL_FreeSurface(surface);
        return texture;
    }
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createTextSurface(const std::string& text, int size, const SDL_Color& color) {
    TTF_Font* font = TTF_OpenFont("assets/fonts/courier.ttf", size);
    if (!font) {
        // Fall back to a system font if the file is not found
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", size);
        if (!font) {
            throw std::runtime_error("Failed to load font: " + std::string(TTF_GetError()));
        }
    }
    
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), sdlColor);
    
    if (!surface) {
        TTF_CloseFont(font);
        throw std::runtime_error("Failed to render text: " + std::string(TTF_GetError()));
    }
    
    auto texture = createTextureFromSurface(surface);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    return texture;
}

std::vector<std::shared_ptr<SDL_Texture>> GraphicsGenerator::createDigitSprites() {
    std::vector<std::shared_ptr<SDL_Texture>> digitSprites;
    int digitWidth = 20;
    int digitHeight = 30;
    
    for (int i = 0; i < 10; i++) {
        SDL_Surface* surface = createRGBASurface(digitWidth, digitHeight);
        
        // Draw digit outline
        SDL_Rect outlineRect = {0, 0, digitWidth, digitHeight};
        SDL_Color outlineColor = {WHITE.r, WHITE.g, WHITE.b, WHITE.a};
        
        // Since SDL doesn't have a built-in rectangle outline function,
        // we'll draw four rectangles for the edges
        
        // Top edge
        SDL_Rect topEdge = {0, 0, digitWidth, 2};
        // Left edge
        SDL_Rect leftEdge = {0, 0, 2, digitHeight};
        // Right edge
        SDL_Rect rightEdge = {digitWidth - 2, 0, 2, digitHeight};
        // Bottom edge
        SDL_Rect bottomEdge = {0, digitHeight - 2, digitWidth, 2};
        
        SDL_FillRect(surface, &topEdge, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        SDL_FillRect(surface, &leftEdge, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        SDL_FillRect(surface, &rightEdge, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        SDL_FillRect(surface, &bottomEdge, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        
        // Draw the segments for each digit
        if (i == 0 || i == 2 || i == 3 || i == 5 || i == 6 || i == 7 || i == 8 || i == 9) {
            // Top horizontal
            SDL_Rect topSegment = {2, 2, digitWidth - 4, 2};
            SDL_FillRect(surface, &topSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        if (i == 0 || i == 4 || i == 5 || i == 6 || i == 8 || i == 9) {
            // Top-left vertical
            SDL_Rect topLeftSegment = {2, 3, 2, digitHeight / 2 - 3};
            SDL_FillRect(surface, &topLeftSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        if (i == 0 || i == 1 || i == 2 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9) {
            // Top-right vertical
            SDL_Rect topRightSegment = {digitWidth - 4, 3, 2, digitHeight / 2 - 3};
            SDL_FillRect(surface, &topRightSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        if (i == 2 || i == 3 || i == 4 || i == 5 || i == 6 || i == 8 || i == 9) {
            // Middle horizontal
            SDL_Rect midSegment = {2, digitHeight / 2, digitWidth - 4, 2};
            SDL_FillRect(surface, &midSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        if (i == 0 || i == 2 || i == 6 || i == 8) {
            // Bottom-left vertical
            SDL_Rect botLeftSegment = {2, digitHeight / 2 + 2, 2, digitHeight / 2 - 4};
            SDL_FillRect(surface, &botLeftSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        if (i == 0 || i == 1 || i == 3 || i == 4 || i == 5 || i == 6 || i == 7 || i == 8 || i == 9) {
            // Bottom-right vertical
            SDL_Rect botRightSegment = {digitWidth - 4, digitHeight / 2 + 2, 2, digitHeight / 2 - 4};
            SDL_FillRect(surface, &botRightSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        if (i == 0 || i == 2 || i == 3 || i == 5 || i == 6 || i == 8 || i == 9) {
            // Bottom horizontal
            SDL_Rect botSegment = {2, digitHeight - 4, digitWidth - 4, 2};
            SDL_FillRect(surface, &botSegment, SDL_MapRGBA(surface->format, WHITE.r, WHITE.g, WHITE.b, WHITE.a));
        }
        
        auto texture = createTextureFromSurface(surface);
        SDL_FreeSurface(surface);
        
        digitSprites.push_back(texture);
    }
    
    return digitSprites;
}

std::shared_ptr<SDL_Texture> GraphicsGenerator::createTextureFromSurface(SDL_Surface* surface) {
    if (!renderer) {
        throw std::runtime_error("Renderer not initialized");
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        throw std::runtime_error("Failed to create texture from surface: " + std::string(SDL_GetError()));
    }
    
    return std::shared_ptr<SDL_Texture>(texture, SDL_DestroyTexture);
}

SDL_Surface* GraphicsGenerator::createRGBASurface(int width, int height) {
    // Create a surface with per-pixel alpha
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    #else
        SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    #endif
            
    if (!surface) {
        throw std::runtime_error("Failed to create surface: " + std::string(SDL_GetError()));
    }
    
    // Enable alpha blending on the surface
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
    
    // Clear surface to transparent
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
    
    return surface;
}

void GraphicsGenerator::setPixel(SDL_Surface* surface, int x, int y, const SDL_Color& color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        return;
    }
    
    Uint32 pixel = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    pixels[y * surface->w + x] = pixel;
}

SDL_Color GraphicsGenerator::getPixel(SDL_Surface* surface, int x, int y) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        return {0, 0, 0, 0};
    }
    
    Uint32* pixels = static_cast<Uint32*>(surface->pixels);
    Uint32 pixel = pixels[y * surface->w + x];
    
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
    
    return {r, g, b, a};
}

} // namespace SpaceInvaders
