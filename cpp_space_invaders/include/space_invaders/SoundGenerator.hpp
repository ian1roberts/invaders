#pragma once

#include <SDL2/SDL_mixer.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace SpaceInvaders {

class SoundGenerator {
public:
    SoundGenerator();
    ~SoundGenerator();
    
    void initialize();
    void generateAllSounds();
    void playSound(const std::string& soundName);
    void stopSound(const std::string& soundName);
    
private:
    // Map of sound names to Mix_Chunk pointers
    std::map<std::string, Mix_Chunk*> sounds;
    int numChannels;
    
    // Sound generation methods
    Mix_Chunk* generatePlayerShoot();
    Mix_Chunk* generateInvaderShoot();
    Mix_Chunk* generatePlayerExplosion();
    Mix_Chunk* generateInvaderExplosion();
    Mix_Chunk* generateMysteryShip();
    Mix_Chunk* generateMysteryShipHit();
    Mix_Chunk* generateGameOver();
    Mix_Chunk* generateInvaderMovementSounds();
    
    // Helper methods for sound wave generation
    std::vector<int16_t> generateSineWave(float frequency, float duration, float amplitude = 0.5f);
    std::vector<int16_t> generateNoise(int samples, float amplitude = 0.5f);
    std::vector<int16_t> applyEnvelope(const std::vector<int16_t>& samples, float attack, float decay);
    Mix_Chunk* createChunkFromSamples(const std::vector<int16_t>& samples);
};

} // namespace SpaceInvaders
