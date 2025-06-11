#!/usr/bin/env python3
import pygame
import numpy as np
import random

class SoundGenerator:
    """Generate retro-style sound effects programmatically"""
    
    def __init__(self):
        # Initialize mixer with specific settings
        pygame.mixer.init(frequency=44100, size=-16, channels=2)  # Changed to 2 channels
        self.sounds = {}
        self.num_channels = 2  # Store channel count to use in sound generation
        
    def generate_all_sounds(self):
        """Generate all game sounds"""
        self.sounds["player_shoot"] = self.generate_player_shoot()
        self.sounds["invader_shoot"] = self.generate_invader_shoot()
        self.sounds["player_explosion"] = self.generate_player_explosion()
        self.sounds["invader_explosion"] = self.generate_invader_explosion()
        self.sounds["mystery_ship"] = self.generate_mystery_ship()
        self.sounds["mystery_ship_hit"] = self.generate_mystery_ship_hit()
        self.sounds["game_over"] = self.generate_game_over()
        self.sounds["invader_movement"] = self.generate_invader_movement_sounds()
        
    def play_sound(self, sound_name):
        """Play a sound by name"""
        if sound_name in self.sounds:
            self.sounds[sound_name].play()
    
    def stop_sound(self, sound_name):
        """Stop a sound by name"""
        if sound_name in self.sounds:
            self.sounds[sound_name].stop()
    
    def generate_player_shoot(self):
        """Generate player shooting sound"""
        # Create a short high-pitched zap
        sample_rate = 44100
        duration = 0.2  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Start with a higher frequency and decrease
        freq_start = 1000
        freq_end = 300
        freq = np.linspace(freq_start, freq_end, int(sample_rate * duration))
        
        # Generate the waveform
        waveform = np.sin(2 * np.pi * freq * t) * 0.5
        
        # Apply a quick decay envelope
        envelope = np.exp(-5 * t)
        waveform = waveform * envelope
        
        # Convert to 16-bit PCM
        waveform = (waveform * 32767).astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_invader_shoot(self):
        """Generate invader shooting sound"""
        # Create an alien-like descending tone
        sample_rate = 44100
        duration = 0.3  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Complex frequency modulation for alien feel
        freq_base = 400
        freq_mod = 100
        freq = freq_base - freq_mod * np.sin(2 * np.pi * 3 * t)
        
        # Generate the waveform with some noise
        waveform = np.sin(2 * np.pi * freq * t) * 0.5
        noise = np.random.uniform(-0.2, 0.2, size=len(t))
        waveform = waveform + noise
        
        # Apply envelope
        envelope = np.exp(-3 * t)
        waveform = waveform * envelope
        
        # Convert to 16-bit PCM
        waveform = np.clip(waveform, -1, 1) * 32767
        waveform = waveform.astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_player_explosion(self):
        """Generate player explosion sound"""
        # Create an explosion sound
        sample_rate = 44100
        duration = 0.5  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # White noise as base
        waveform = np.random.uniform(-0.8, 0.8, size=len(t))
        
        # Add some low frequency rumble
        rumble_freq = 30
        rumble = np.sin(2 * np.pi * rumble_freq * t) * 0.3
        waveform = waveform + rumble
        
        # Apply envelope - quick attack, slower decay
        attack = 0.05
        decay = duration - attack
        envelope = np.ones_like(t)
        attack_mask = t < attack
        decay_mask = t >= attack
        
        envelope[attack_mask] = t[attack_mask] / attack
        envelope[decay_mask] = np.exp(-(t[decay_mask] - attack) / (decay * 0.5))
        
        waveform = waveform * envelope
        
        # Convert to 16-bit PCM
        waveform = np.clip(waveform, -1, 1) * 32767
        waveform = waveform.astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_invader_explosion(self):
        """Generate invader explosion sound"""
        # Create an alien death sound
        sample_rate = 44100
        duration = 0.4  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # High pitched descending tone with noise
        freq_start = 800
        freq_end = 200
        freq = np.linspace(freq_start, freq_end, int(sample_rate * duration))
        
        tone = np.sin(2 * np.pi * freq * t) * 0.5
        noise = np.random.uniform(-0.5, 0.5, size=len(t))
        
        # Mix tone and noise
        waveform = tone + noise * 0.3
        
        # Apply envelope
        envelope = np.exp(-5 * t)
        waveform = waveform * envelope
        
        # Convert to 16-bit PCM
        waveform = np.clip(waveform, -1, 1) * 32767
        waveform = waveform.astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_mystery_ship(self):
        """Generate mystery ship sound"""
        # Create an eerie UFO sound
        sample_rate = 44100
        duration = 3.0  # seconds (loopable)
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Oscillating frequency between two values
        freq1 = 600
        freq2 = 800
        osc_rate = 4  # Hz
        freq = freq1 + (freq2 - freq1) * 0.5 * (1 + np.sin(2 * np.pi * osc_rate * t))
        
        # Generate waveform - mix of sine waves
        wave1 = np.sin(2 * np.pi * freq * t) * 0.3
        wave2 = np.sin(2 * np.pi * (freq * 1.5) * t) * 0.15
        waveform = wave1 + wave2
        
        # Convert to 16-bit PCM
        waveform = np.clip(waveform, -1, 1) * 32767
        waveform = waveform.astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_mystery_ship_hit(self):
        """Generate mystery ship explosion sound"""
        # Create a special explosion for the mystery ship
        sample_rate = 44100
        duration = 0.8  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Complex frequency pattern
        base_freq = 500
        freq_pattern = base_freq * (1 + 0.5 * np.sin(2 * np.pi * 10 * t))
        
        # Create a sweep
        sweep = np.sin(2 * np.pi * freq_pattern * t)
        
        # Add noise burst
        noise = np.random.uniform(-0.8, 0.8, size=len(t))
        decay = np.exp(-5 * t)
        noise = noise * decay
        
        # Combine and add beeping for "points"
        beep_freq = 1200
        beep = np.sin(2 * np.pi * beep_freq * t)
        beep_env = 0.5 * (1 + np.sin(2 * np.pi * 20 * t))
        beep = beep * beep_env
        
        waveform = sweep * 0.3 + noise * 0.3 + beep * 0.4
        
        # Apply overall envelope
        envelope = np.exp(-3 * t)
        waveform = waveform * envelope
        
        # Convert to 16-bit PCM
        waveform = np.clip(waveform, -1, 1) * 32767
        waveform = waveform.astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_game_over(self):
        """Generate game over sound"""
        # Create a descending "sad" game over sound
        sample_rate = 44100
        duration = 1.5  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Descending notes
        notes = [400, 350, 300, 250, 200, 150]
        note_duration = duration / len(notes)
        
        waveform = np.zeros_like(t)
        for i, note in enumerate(notes):
            start_idx = int(i * note_duration * sample_rate)
            end_idx = int((i + 1) * note_duration * sample_rate)
            if end_idx > len(t):
                end_idx = len(t)
                
            t_note = np.linspace(0, note_duration, end_idx - start_idx, False)
            note_wave = np.sin(2 * np.pi * note * t_note)
            
            # Apply envelope to each note
            env = np.exp(-3 * t_note / note_duration)
            note_wave = note_wave * env
            
            waveform[start_idx:end_idx] = note_wave * 0.7
        
        # Convert to 16-bit PCM
        waveform = np.clip(waveform, -1, 1) * 32767
        waveform = waveform.astype(np.int16)
        
        # Duplicate mono to match channel count (stereo)
        if self.num_channels > 1:
            waveform = np.column_stack([waveform] * self.num_channels)
        else:
            waveform = waveform.reshape(-1, 1)
        
        # Create and return PyGame sound
        sound = pygame.sndarray.make_sound(waveform)
        return sound
    
    def generate_invader_movement_sounds(self):
        """Generate the four different invader movement sounds"""
        # Create the classic "step" sounds that cycle during gameplay
        sample_rate = 44100
        duration = 0.1  # seconds
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # We'll create 4 variations of the sound
        base_freqs = [160, 180, 140, 120]
        sound_array = []
        
        for base_freq in base_freqs:
            # Simple sine wave with slight pitch shift
            waveform = np.sin(2 * np.pi * base_freq * t)
            
            # Apply quick decay
            envelope = np.exp(-20 * t)
            waveform = waveform * envelope * 0.8
            
            # Convert to 16-bit PCM
            waveform = np.clip(waveform, -1, 1) * 32767
            waveform = waveform.astype(np.int16)
            
            # Duplicate mono to match channel count (stereo)
            if self.num_channels > 1:
                waveform = np.column_stack([waveform] * self.num_channels)
            else:
                waveform = waveform.reshape(-1, 1)
            
            # Create PyGame sound
            sound = pygame.sndarray.make_sound(waveform)
            sound_array.append(sound)
        
        return sound_array[0]  # We'll handle cycling in the game loop