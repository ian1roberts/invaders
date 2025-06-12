#include "space_invaders/SoundGenerator.hpp"
#include <cmath>
#include <random>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <array>

namespace SpaceInvaders {

SoundGenerator::SoundGenerator() : numChannels(2) {
}

SoundGenerator::~SoundGenerator() {
    // Free all sound chunks
    for (auto& [name, chunk] : sounds) {
        if (chunk) {
            Mix_FreeChunk(chunk);
        }
    }
    
    // Quit SDL_mixer
    Mix_CloseAudio();
    Mix_Quit();
}

void SoundGenerator::initialize() {
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0) {
        throw std::runtime_error("SDL_mixer could not initialize! Error: " + std::string(Mix_GetError()));
    }
    
    // Set number of channels (simultaneous sounds)
    Mix_AllocateChannels(8);
}

void SoundGenerator::generateAllSounds() {
    sounds["player_shoot"] = generatePlayerShoot();
    sounds["invader_shoot"] = generateInvaderShoot();
    sounds["player_explosion"] = generatePlayerExplosion();
    sounds["invader_explosion"] = generateInvaderExplosion();
    sounds["mystery_ship"] = generateMysteryShip();
    sounds["mystery_ship_hit"] = generateMysteryShipHit();
    sounds["game_over"] = generateGameOver();
    
    // Generate the 4 movement sounds for the iconic descending bass sequence
    sounds["invader_movement0"] = generateInvaderMovementSound(0);
    sounds["invader_movement1"] = generateInvaderMovementSound(1);
    sounds["invader_movement2"] = generateInvaderMovementSound(2);
    sounds["invader_movement3"] = generateInvaderMovementSound(3);
}

void SoundGenerator::playSound(const std::string& soundName) {
    if (sounds.find(soundName) != sounds.end() && sounds[soundName]) {
        Mix_PlayChannel(-1, sounds[soundName], 0);
    }
}

void SoundGenerator::stopSound(const std::string& soundName) {
    if (sounds.find(soundName) != sounds.end() && sounds[soundName]) {
        // Find all channels playing this sound and halt them
        for (int i = 0; i < Mix_AllocateChannels(-1); i++) {
            if (Mix_Playing(i) && Mix_GetChunk(i) == sounds[soundName]) {
                Mix_HaltChannel(i);
            }
        }
    }
}

Mix_Chunk* SoundGenerator::generatePlayerShoot() {
    // Create a short high-pitched zap
    int sampleRate = 44100;
    float duration = 0.2f;  // seconds
    int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<int16_t> waveform(samples);
    
    // Start with a higher frequency and decrease
    float freqStart = 1000.0f;
    float freqEnd = 300.0f;
    
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float freq = freqStart - (freqStart - freqEnd) * (t / duration);
        
        // Generate the sine wave
        float sample = std::sin(2.0f * M_PI * freq * t) * 0.5f;
        
        // Apply a quick decay envelope
        float envelope = std::exp(-5.0f * t);
        sample = sample * envelope;
        
        // Convert to 16-bit
        waveform[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    return createChunkFromSamples(waveform);
}

Mix_Chunk* SoundGenerator::generateInvaderShoot() {
    // Create an alien-like descending tone
    int sampleRate = 44100;
    float duration = 0.3f;  // seconds
    int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<int16_t> waveform(samples);
    std::vector<float> noise = std::vector<float>(samples);
    
    // Generate random noise
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.2f, 0.2f);
    
    for (int i = 0; i < samples; i++) {
        noise[i] = dis(gen);
    }
    
    // Complex frequency modulation for alien feel
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        
        float freqBase = 400.0f;
        float freqMod = 100.0f;
        float freq = freqBase - freqMod * std::sin(2.0f * M_PI * 3.0f * t);
        
        // Generate the waveform with some noise
        float sample = std::sin(2.0f * M_PI * freq * t) * 0.5f;
        sample = sample + noise[i];
        
        // Apply envelope
        float envelope = std::exp(-3.0f * t);
        sample = sample * envelope;
        
        // Convert to 16-bit
        waveform[i] = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, sample)) * 32767.0f);
    }
    
    return createChunkFromSamples(waveform);
}

Mix_Chunk* SoundGenerator::generatePlayerExplosion() {
    // Create an explosion sound
    int sampleRate = 44100;
    float duration = 0.5f;  // seconds
    int samples = static_cast<int>(sampleRate * duration);
    
    // White noise as base
    std::vector<int16_t> noise = std::vector<int16_t>(samples);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.8f, 0.8f);
    
    for (int i = 0; i < samples; i++) {
        // Generate noise
        noise[i] = static_cast<int16_t>(dis(gen) * 32767.0f);
    }
    
    // Add some low frequency rumble
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float rumbleFreq = 30.0f;
        float rumble = std::sin(2.0f * M_PI * rumbleFreq * t) * 0.3f * 32767.0f;
        
        // Mix with noise
        noise[i] = static_cast<int16_t>(std::max(-32767.0f, std::min(32767.0f, static_cast<float>(noise[i]) + rumble)));
    }
    
    // Apply envelope - quick attack, slower decay
    float attack = 0.05f;
    float decay = duration - attack;
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float envelope;
        
        if (t < attack) {
            envelope = t / attack;
        } else {
            envelope = std::exp(-(t - attack) / (decay * 0.5f));
        }
        
        noise[i] = static_cast<int16_t>(noise[i] * envelope);
    }
    
    return createChunkFromSamples(noise);
}

Mix_Chunk* SoundGenerator::generateInvaderExplosion() {
    // Create an alien death sound
    int sampleRate = 44100;
    float duration = 0.4f;  // seconds
    int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<int16_t> waveform(samples);
    
    // High pitched descending tone with noise
    float freqStart = 800.0f;
    float freqEnd = 200.0f;
    
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float freq = freqStart - (freqStart - freqEnd) * (t / duration);
        
        // Generate tone
        float tone = std::sin(2.0f * M_PI * freq * t) * 0.5f;
        
        // Add noise
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-0.5f, 0.5f);
        float noise = dis(gen) * 0.3f;
        
        // Mix tone and noise
        float sample = tone + noise;
        
        // Apply envelope
        float envelope = std::exp(-5.0f * t);
        sample = sample * envelope;
        
        // Convert to 16-bit
        waveform[i] = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, sample)) * 32767.0f);
    }
    
    return createChunkFromSamples(waveform);
}

Mix_Chunk* SoundGenerator::generateMysteryShip() {
    // Create an eerie UFO sound
    int sampleRate = 44100;
    float duration = 3.0f;  // seconds (loopable)
    int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<int16_t> waveform(samples);
    
    // Oscillating frequency between two values
    float freq1 = 600.0f;
    float freq2 = 800.0f;
    float oscRate = 4.0f;  // Hz
    
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float freq = freq1 + (freq2 - freq1) * 0.5f * (1.0f + std::sin(2.0f * M_PI * oscRate * t));
        
        // Generate waveform - mix of sine waves
        float wave1 = std::sin(2.0f * M_PI * freq * t) * 0.3f;
        float wave2 = std::sin(2.0f * M_PI * (freq * 1.5f) * t) * 0.15f;
        float sample = wave1 + wave2;
        
        // Convert to 16-bit
        waveform[i] = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, sample)) * 32767.0f);
    }
    
    return createChunkFromSamples(waveform);
}

Mix_Chunk* SoundGenerator::generateMysteryShipHit() {
    // Create a special explosion for the mystery ship
    int sampleRate = 44100;
    float duration = 0.8f;  // seconds
    int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<int16_t> waveform(samples);
    
    // Complex frequency pattern
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        
        // Complex frequency pattern
        float baseFreq = 500.0f;
        float freqPattern = baseFreq * (1.0f + 0.5f * std::sin(2.0f * M_PI * 10.0f * t));
        
        // Create a sweep
        float sweep = std::sin(2.0f * M_PI * freqPattern * t);
        
        // Add noise burst
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-0.8f, 0.8f);
        float noise = dis(gen);
        float decay = std::exp(-5.0f * t);
        noise = noise * decay;
        
        // Combine and add beeping for "points"
        float beepFreq = 1200.0f;
        float beep = std::sin(2.0f * M_PI * beepFreq * t);
        float beepEnv = 0.5f * (1.0f + std::sin(2.0f * M_PI * 20.0f * t));
        beep = beep * beepEnv;
        
        float sample = sweep * 0.3f + noise * 0.3f + beep * 0.4f;
        
        // Apply overall envelope
        float envelope = std::exp(-3.0f * t);
        sample = sample * envelope;
        
        // Convert to 16-bit
        waveform[i] = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, sample)) * 32767.0f);
    }
    
    return createChunkFromSamples(waveform);
}

Mix_Chunk* SoundGenerator::generateGameOver() {
    // Create a descending "sad" game over sound
    int sampleRate = 44100;
    float duration = 1.5f;  // seconds
    int samples = static_cast<int>(sampleRate * duration);
    
    std::vector<int16_t> waveform(samples);
    
    // Descending notes
    std::vector<int> notes = {400, 350, 300, 250, 200, 150};
    float noteDuration = duration / notes.size();
    
    for (int noteIdx = 0; noteIdx < notes.size(); noteIdx++) {
        int startSample = static_cast<int>(noteIdx * noteDuration * sampleRate);
        int endSample = static_cast<int>((noteIdx + 1) * noteDuration * sampleRate);
        
        if (endSample > samples) {
            endSample = samples;
        }
        
        for (int i = startSample; i < endSample; i++) {
            float t = static_cast<float>(i - startSample) / sampleRate;
            float noteWave = std::sin(2.0f * M_PI * notes[noteIdx] * t);
            
            // Apply envelope to each note
            float env = std::exp(-3.0f * t / noteDuration);
            noteWave = noteWave * env;
            
            waveform[i] = static_cast<int16_t>(noteWave * 0.7f * 32767.0f);
        }
    }
    
    return createChunkFromSamples(waveform);
}

Mix_Chunk* SoundGenerator::generateInvaderMovementSound(int noteIndex) {
    // Create one of the four distinct notes in the iconic Space Invaders movement pattern
    int sampleRate = 44100;
    // We keep the sound duration shorter for better playback quality
    // but this won't affect the movement timing which is handled separately in Game.cpp
    float duration = 0.8f;  // shorter duration for better sound quality
    int samples = static_cast<int>(sampleRate * duration);
    
    // The four frequencies used in the original game (approximated)
    // These notes create the iconic descending bass line
    std::array<float, 4> frequencies = { 550.0f, 440.0f, 330.0f, 220.0f };
    float baseFreq = frequencies[noteIndex]; // Use the specified note
    
    std::vector<int16_t> waveform(samples);
    
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        
        // Simple square wave like the original arcade sounds
        // Reduced volume by factor of 2 (from 0.8 to 0.4)
        float signal = (std::sin(2.0f * M_PI * baseFreq * t) > 0) ? 0.2f : -0.2f;
        
        // Apply quick decay to match arcade sound
        float envelope = std::exp(-4.0f * t); // Slightly gentler decay for the shorter sound
        float wave = signal * envelope;
        
        // Convert to 16-bit
        waveform[i] = static_cast<int16_t>(wave * 32767.0f);
    }
    
    return createChunkFromSamples(waveform);
}

std::vector<int16_t> SoundGenerator::generateSineWave(float frequency, float duration, float amplitude) {
    int sampleRate = 44100;
    int samples = static_cast<int>(sampleRate * duration);
    std::vector<int16_t> waveform(samples);
    
    for (int i = 0; i < samples; i++) {
        float t = static_cast<float>(i) / sampleRate;
        float sample = std::sin(2.0f * M_PI * frequency * t) * amplitude;
        waveform[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    return waveform;
}

std::vector<int16_t> SoundGenerator::generateNoise(int samples, float amplitude) {
    std::vector<int16_t> noise(samples);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-amplitude, amplitude);
    
    for (int i = 0; i < samples; i++) {
        noise[i] = static_cast<int16_t>(dis(gen) * 32767.0f);
    }
    
    return noise;
}

std::vector<int16_t> SoundGenerator::applyEnvelope(const std::vector<int16_t>& samples, float attack, float decay) {
    std::vector<int16_t> result = samples;
    int sampleRate = 44100;
    float duration = static_cast<float>(samples.size()) / sampleRate;
    
    for (size_t i = 0; i < samples.size(); i++) {
        float t = static_cast<float>(i) / sampleRate;
        float envelope;
        
        if (t < attack) {
            envelope = t / attack;
        } else {
            envelope = std::exp(-(t - attack) / decay);
        }
        
        result[i] = static_cast<int16_t>(samples[i] * envelope);
    }
    
    return result;
}

Mix_Chunk* SoundGenerator::createChunkFromSamples(const std::vector<int16_t>& samples) {
    // Create audio buffer
    Uint32 audioLen = static_cast<Uint32>(samples.size()) * sizeof(int16_t) * numChannels;
    Uint8* audioBuffer = new Uint8[audioLen];
    
    // Fill buffer with stereo audio (duplicate mono to both channels)
    int16_t* audioBufferInt16 = reinterpret_cast<int16_t*>(audioBuffer);
    for (size_t i = 0; i < samples.size(); i++) {
        for (int channel = 0; channel < numChannels; channel++) {
            audioBufferInt16[i * numChannels + channel] = samples[i];
        }
    }
    
    // Create Mix_Chunk with the audio data
    Mix_Chunk* chunk = Mix_QuickLoad_RAW(audioBuffer, audioLen);
    if (!chunk) {
        delete[] audioBuffer;
        throw std::runtime_error("Failed to create Mix_Chunk: " + std::string(Mix_GetError()));
    }
    
    // Set the chunk's allocated flag to 1 so SDL_mixer will free the buffer when the chunk is freed
    chunk->allocated = 1;
    
    return chunk;
}

} // namespace SpaceInvaders
