#!/usr/bin/env python3
import pygame
import random
from space_invaders.constants import *

class GraphicsGenerator:
    """Generate retro-style pixel art for Space Invaders"""
    
    @staticmethod
    def create_player_ship():
        """Create the player's cannon/ship sprite"""
        surface = pygame.Surface((PLAYER_WIDTH, PLAYER_HEIGHT), pygame.SRCALPHA)
        
        # Draw the cannon base (green rectangle)
        base_rect = pygame.Rect(0, PLAYER_HEIGHT - 10, PLAYER_WIDTH, 10)
        pygame.draw.rect(surface, GREEN, base_rect)
        
        # Draw the cannon (rectangle with a triangle on top)
        cannon_rect = pygame.Rect(PLAYER_WIDTH // 2 - 5, 0, 10, PLAYER_HEIGHT - 10)
        pygame.draw.rect(surface, GREEN, cannon_rect)
        
        return surface
        
    @staticmethod
    def create_invader(invader_type):
        """Create an invader sprite based on its type (0, 1, or 2)"""
        surface = pygame.Surface((INVADER_WIDTH, INVADER_HEIGHT), pygame.SRCALPHA)
        color = GREEN
        
        if invader_type == 0:  # Top row - squid-like
            # Main body
            body = pygame.Rect(INVADER_WIDTH // 4, INVADER_HEIGHT // 3, 
                              INVADER_WIDTH // 2, INVADER_HEIGHT // 2)
            pygame.draw.rect(surface, color, body)
            
            # Tentacles
            for i in range(3):
                x_pos = INVADER_WIDTH // 4 + (i * INVADER_WIDTH // 6)
                tent = pygame.Rect(x_pos, INVADER_HEIGHT * 5 // 6,
                                 INVADER_WIDTH // 12, INVADER_HEIGHT // 6)
                pygame.draw.rect(surface, color, tent)
            
            # Eyes
            eye_size = INVADER_WIDTH // 10
            left_eye = pygame.Rect(INVADER_WIDTH // 3, INVADER_HEIGHT // 3 + eye_size,
                                 eye_size, eye_size)
            right_eye = pygame.Rect(INVADER_WIDTH * 2 // 3 - eye_size, INVADER_HEIGHT // 3 + eye_size,
                                  eye_size, eye_size)
            pygame.draw.rect(surface, BLACK, left_eye)
            pygame.draw.rect(surface, BLACK, right_eye)
            
        elif invader_type == 1:  # Middle rows - crab-like
            # Main body
            body = pygame.Rect(INVADER_WIDTH // 6, INVADER_HEIGHT // 4, 
                              INVADER_WIDTH * 2 // 3, INVADER_HEIGHT // 2)
            pygame.draw.rect(surface, color, body)
            
            # Claws
            left_claw = pygame.Rect(0, INVADER_HEIGHT // 2, 
                                  INVADER_WIDTH // 6, INVADER_HEIGHT // 4)
            right_claw = pygame.Rect(INVADER_WIDTH * 5 // 6, INVADER_HEIGHT // 2, 
                                   INVADER_WIDTH // 6, INVADER_HEIGHT // 4)
            pygame.draw.rect(surface, color, left_claw)
            pygame.draw.rect(surface, color, right_claw)
            
            # Eyes
            eye_size = INVADER_WIDTH // 10
            left_eye = pygame.Rect(INVADER_WIDTH // 3, INVADER_HEIGHT // 3,
                                 eye_size, eye_size)
            right_eye = pygame.Rect(INVADER_WIDTH * 2 // 3 - eye_size, INVADER_HEIGHT // 3,
                                  eye_size, eye_size)
            pygame.draw.rect(surface, BLACK, left_eye)
            pygame.draw.rect(surface, BLACK, right_eye)
            
        else:  # Bottom rows - octopus-like
            # Main circular body
            center_x = INVADER_WIDTH // 2
            center_y = INVADER_HEIGHT // 2
            radius = min(INVADER_WIDTH, INVADER_HEIGHT) // 3
            pygame.draw.circle(surface, color, (center_x, center_y), radius)
            
            # Tentacles
            for i in range(4):
                x_offset = INVADER_WIDTH // 8 + (i * INVADER_WIDTH // 4)
                tent = pygame.Rect(x_offset, INVADER_HEIGHT * 3 // 4,
                                 INVADER_WIDTH // 12, INVADER_HEIGHT // 4)
                pygame.draw.rect(surface, color, tent)
            
            # Eyes
            eye_size = INVADER_WIDTH // 12
            eye_y = center_y - eye_size // 2
            left_eye = pygame.Rect(center_x - radius // 2, eye_y, eye_size, eye_size)
            right_eye = pygame.Rect(center_x + radius // 2 - eye_size, eye_y, eye_size, eye_size)
            pygame.draw.rect(surface, BLACK, left_eye)
            pygame.draw.rect(surface, BLACK, right_eye)
            
        return surface
        
    @staticmethod
    def create_alternate_frame(sprite):
        """Create an alternate animation frame for sprites"""
        alt_sprite = sprite.copy()
        width = sprite.get_width()
        height = sprite.get_height()
        
        # Modify some pixels to create animation effect
        for x in range(width):
            for y in range(height):
                if x < width // 4 or x > width * 3 // 4:
                    if sprite.get_at((x, y))[3] > 0:  # If pixel is not transparent
                        # Shift certain edge pixels to create movement illusion
                        if y > height // 2:
                            alt_sprite.set_at((x, y), (0, 0, 0, 0))  # Make transparent
                            if y < height - 1:
                                color = sprite.get_at((x, y))
                                alt_sprite.set_at((x, y + 1), color)
        
        return alt_sprite
    
    @staticmethod
    def create_mystery_ship():
        """Create the mystery UFO that appears at the top"""
        surface = pygame.Surface((MYSTERY_SHIP_WIDTH, MYSTERY_SHIP_HEIGHT), pygame.SRCALPHA)
        
        # UFO body (ellipse)
        ellipse_rect = pygame.Rect(0, MYSTERY_SHIP_HEIGHT // 3, 
                                  MYSTERY_SHIP_WIDTH, MYSTERY_SHIP_HEIGHT // 2)
        pygame.draw.ellipse(surface, RED, ellipse_rect)
        
        # UFO top dome
        dome_rect = pygame.Rect(MYSTERY_SHIP_WIDTH // 4, 0, 
                               MYSTERY_SHIP_WIDTH // 2, MYSTERY_SHIP_HEIGHT // 2)
        pygame.draw.ellipse(surface, RED, dome_rect)
        
        # Windows
        window_size = MYSTERY_SHIP_WIDTH // 10
        for i in range(3):
            window_x = MYSTERY_SHIP_WIDTH // 4 + i * (MYSTERY_SHIP_WIDTH // 4) 
            window_y = MYSTERY_SHIP_HEIGHT // 2
            window_rect = pygame.Rect(window_x, window_y, window_size, window_size)
            pygame.draw.rect(surface, YELLOW, window_rect)
        
        return surface
        
    @staticmethod
    def create_barrier_piece():
        """Create a single piece of the defensive barriers"""
        piece_size = 5
        surface = pygame.Surface((piece_size, piece_size))
        surface.fill(GREEN)
        return surface
    
    @staticmethod
    def create_barrier():
        """Create a defensive barrier made up of small blocks"""
        surface = pygame.Surface((BARRIER_WIDTH, BARRIER_HEIGHT), pygame.SRCALPHA)
        piece_size = 5
        
        # Draw the main barrier shape (fortress-like)
        for x in range(BARRIER_WIDTH // piece_size):
            for y in range(BARRIER_HEIGHT // piece_size):
                # Skip the bottom corners to create an arch
                if (y > BARRIER_HEIGHT // piece_size * 2 // 3 and 
                    (x < BARRIER_WIDTH // piece_size // 4 or 
                     x > BARRIER_WIDTH // piece_size * 3 // 4)):
                    continue
                
                # Create the middle arch opening
                if (y > BARRIER_HEIGHT // piece_size // 2 and 
                    x > BARRIER_WIDTH // piece_size // 3 and 
                    x < BARRIER_WIDTH // piece_size * 2 // 3):
                    continue
                
                piece = GraphicsGenerator.create_barrier_piece()
                surface.blit(piece, (x * piece_size, y * piece_size))
                
        return surface
    
    @staticmethod
    def create_explosion(size):
        """Create an explosion sprite"""
        surface = pygame.Surface((size, size), pygame.SRCALPHA)
        
        # Draw random explosion particles
        num_particles = 20
        for _ in range(num_particles):
            x = random.randint(0, size - 1)
            y = random.randint(0, size - 1)
            radius = random.randint(1, size // 5)
            color = random.choice([YELLOW, RED, WHITE])
            pygame.draw.circle(surface, color, (x, y), radius)
            
        return surface
    
    @staticmethod
    def create_bullet(bullet_type):
        """Create bullet sprites
        bullet_type: 0 for player bullet, 1 for invader bullet
        """
        if bullet_type == 0:  # Player bullet
            surface = pygame.Surface((PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT), pygame.SRCALPHA)
            pygame.draw.rect(surface, WHITE, (0, 0, PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT))
            
        else:  # Invader bullet (zigzag shape)
            surface = pygame.Surface((INVADER_BULLET_WIDTH, INVADER_BULLET_HEIGHT), pygame.SRCALPHA)
            
            # Create zigzag pattern
            points = []
            x_left = 0
            x_right = INVADER_BULLET_WIDTH
            x_mid = INVADER_BULLET_WIDTH // 2
            segment_height = INVADER_BULLET_HEIGHT // 3
            
            for i in range(4):
                y = i * segment_height
                if i % 2 == 0:
                    points.append((x_left, y))
                    points.append((x_mid, y + segment_height // 2))
                else:
                    points.append((x_right, y))
                    points.append((x_mid, y + segment_height // 2))
            
            pygame.draw.lines(surface, WHITE, False, points, 2)
            
        return surface
    
    @staticmethod
    def create_text_surface(text, size, color=WHITE):
        """Create a text surface using a pixel font effect"""
        # Create our own pixelated font
        font = pygame.font.SysFont('Courier', size, bold=True)
        text_surface = font.render(text, False, color)
        return text_surface
    
    @staticmethod
    def create_digit_sprites():
        """Create sprites for score digits (0-9)"""
        digit_sprites = []
        digit_width = 20
        digit_height = 30
        
        for i in range(10):
            surface = pygame.Surface((digit_width, digit_height), pygame.SRCALPHA)
            
            # Draw digit outline
            pygame.draw.rect(surface, WHITE, (0, 0, digit_width, digit_height), 2)
            
            # Draw the segments for each digit
            if i in [0, 2, 3, 5, 6, 7, 8, 9]:  # Top horizontal
                pygame.draw.line(surface, WHITE, (2, 2), (digit_width - 3, 2), 2)
                
            if i in [0, 4, 5, 6, 8, 9]:  # Top-left vertical
                pygame.draw.line(surface, WHITE, (2, 3), (2, digit_height // 2 - 1), 2)
                
            if i in [0, 1, 2, 3, 4, 7, 8, 9]:  # Top-right vertical
                pygame.draw.line(surface, WHITE, (digit_width - 3, 3), (digit_width - 3, digit_height // 2 - 1), 2)
                
            if i in [2, 3, 4, 5, 6, 8, 9]:  # Middle horizontal
                pygame.draw.line(surface, WHITE, (2, digit_height // 2), (digit_width - 3, digit_height // 2), 2)
                
            if i in [0, 2, 6, 8]:  # Bottom-left vertical
                pygame.draw.line(surface, WHITE, (2, digit_height // 2 + 1), (2, digit_height - 3), 2)
                
            if i in [0, 1, 3, 4, 5, 6, 7, 8, 9]:  # Bottom-right vertical
                pygame.draw.line(surface, WHITE, (digit_width - 3, digit_height // 2 + 1), 
                               (digit_width - 3, digit_height - 3), 2)
                
            if i in [0, 2, 3, 5, 6, 8, 9]:  # Bottom horizontal
                pygame.draw.line(surface, WHITE, (2, digit_height - 3), (digit_width - 3, digit_height - 3), 2)
                
            digit_sprites.append(surface)
            
        return digit_sprites