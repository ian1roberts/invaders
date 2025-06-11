#!/usr/bin/env python3
import pygame
import random
from space_invaders.constants import *
from space_invaders.graphics import GraphicsGenerator

class Player:
    """The player-controlled ship at the bottom of the screen"""
    
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = PLAYER_WIDTH
        self.height = PLAYER_HEIGHT
        self.speed = PLAYER_SPEED
        self.lives = PLAYER_LIVES
        self.sprite = GraphicsGenerator.create_player_ship()
        self.rect = pygame.Rect(x, y, self.width, self.height)
        self.is_alive = True
        self.last_shot_time = 0
        
    def move(self, direction, game_area_rect):
        """Move the player left or right within game boundaries"""
        if not self.is_alive:
            return
            
        new_x = self.x + (direction * self.speed)
        
        # Check boundaries
        if new_x < game_area_rect.left:
            new_x = game_area_rect.left
        elif new_x + self.width > game_area_rect.right:
            new_x = game_area_rect.right - self.width
        
        self.x = new_x
        self.rect.x = new_x
        
    def can_shoot(self, current_time):
        """Check if player can shoot based on cooldown"""
        return current_time - self.last_shot_time > PLAYER_BULLET_COOLDOWN
    
    def shoot(self, current_time):
        """Create a bullet at the player's position"""
        self.last_shot_time = current_time
        bullet_x = self.x + (self.width // 2) - (PLAYER_BULLET_WIDTH // 2)
        bullet_y = self.y - PLAYER_BULLET_HEIGHT
        return PlayerBullet(bullet_x, bullet_y)
    
    def hit(self):
        """Handle player being hit"""
        self.is_alive = False
        self.lives -= 1
    
    def reset_position(self, x, y):
        """Reset player to the initial position"""
        self.x = x
        self.y = y
        self.rect.x = x
        self.rect.y = y
        self.is_alive = True
    
    def draw(self, surface):
        """Draw the player on the screen"""
        if self.is_alive:
            surface.blit(self.sprite, (self.x, self.y))
            
    def get_collision_rect(self):
        """Return rectangle for collision detection"""
        return pygame.Rect(self.x, self.y, self.width, self.height)


class Invader:
    """An alien invader"""
    
    def __init__(self, x, y, invader_type, row, col):
        self.x = x
        self.y = y
        self.width = INVADER_WIDTH
        self.height = INVADER_HEIGHT
        self.type = invader_type  # 0, 1, or 2 for different types/rows
        self.row = row
        self.col = col
        self.sprite = GraphicsGenerator.create_invader(invader_type)
        self.sprite_alt = GraphicsGenerator.create_alternate_frame(self.sprite)
        self.current_sprite = self.sprite
        self.rect = pygame.Rect(x, y, self.width, self.height)
        self.is_alive = True
        
    def move(self, dx, dy):
        """Move the invader by dx and dy"""
        if not self.is_alive:
            return
            
        self.x += dx
        self.y += dy
        self.rect.x = self.x
        self.rect.y = self.y
    
    def animate(self, frame):
        """Toggle between animation frames"""
        if not self.is_alive:
            return
            
        if frame % 2 == 0:
            self.current_sprite = self.sprite
        else:
            self.current_sprite = self.sprite_alt
    
    def draw(self, surface):
        """Draw the invader on the screen"""
        if self.is_alive:
            surface.blit(self.current_sprite, (self.x, self.y))
    
    def get_collision_rect(self):
        """Return rectangle for collision detection"""
        if self.is_alive:
            return pygame.Rect(self.x, self.y, self.width, self.height)
        return None
    
    def get_points(self):
        """Return points value based on invader type"""
        if self.type == 0:  # Top row
            return SCORE_INVADER_TOP_ROW
        elif self.type == 1:  # Middle rows
            return SCORE_INVADER_MIDDLE_ROW
        else:  # Bottom rows
            return SCORE_INVADER_BOTTOM_ROW


class InvaderGroup:
    """Manages a group of invaders"""
    
    def __init__(self):
        self.invaders = []
        self.speed = INVADER_MOVE_SPEED_H
        self.direction = 1  # 1 for right, -1 for left
        self.move_down = False
        self.frame = 0
        self.last_move_time = 0
        self.move_delay = 1000  # Start with a delay of 1000ms
        self.invaders_killed = 0
        self.total_invaders = INVADER_ROWS * INVADERS_PER_ROW
        
    def create_invaders(self):
        """Create the initial formation of invaders"""
        self.invaders = []
        
        start_x = GAME_AREA_MARGIN_X + INVADER_H_PADDING
        start_y = GAME_AREA_MARGIN_Y + INVADER_V_PADDING
        
        for row in range(INVADER_ROWS):
            invader_type = 0 if row == 0 else (1 if row < 3 else 2)
            y = start_y + row * (INVADER_HEIGHT + INVADER_V_SPACING)
            
            for col in range(INVADERS_PER_ROW):
                x = start_x + col * (INVADER_WIDTH + INVADER_H_SPACING)
                self.invaders.append(Invader(x, y, invader_type, row, col))
    
    def move(self, current_time, game_area_rect):
        """Move all invaders in the current direction"""
        # Don't move if it's not time yet
        if current_time - self.last_move_time < self.move_delay:
            return False
            
        self.last_move_time = current_time
        self.frame += 1
        
        moved_down = False
        dx = self.direction * self.speed
        dy = INVADER_MOVE_DOWN if self.move_down else 0
        
        if self.move_down:
            moved_down = True
            self.move_down = False
        
        # Check if any invader would hit the edge after moving
        need_direction_change = False
        for invader in self.invaders:
            if not invader.is_alive:
                continue
                
            new_x = invader.x + dx
            if (new_x < game_area_rect.left or 
                new_x + INVADER_WIDTH > game_area_rect.right):
                need_direction_change = True
                break
        
        # If hitting edge, prepare to move down on next update
        if need_direction_change:
            self.direction *= -1  # Reverse direction
            self.move_down = True
            return False
        
        # Move all invaders
        for invader in self.invaders:
            if invader.is_alive:
                invader.move(dx, dy)
                invader.animate(self.frame)
        
        return moved_down
        
    def invader_killed(self):
        """Track invader deaths and increase speed"""
        self.invaders_killed += 1
        
        # Increase speed as invaders are killed
        remaining = self.total_invaders - self.invaders_killed
        speed_factor = 1.0 - (remaining / self.total_invaders)
        self.move_delay = max(100, 1000 - (900 * speed_factor))  # Min 100ms delay
    
    def any_invader_at_bottom(self, bottom_y):
        """Check if any invader has reached the bottom"""
        for invader in self.invaders:
            if invader.is_alive and invader.y + invader.height >= bottom_y:
                return True
        return False
    
    def draw(self, surface):
        """Draw all invaders"""
        for invader in self.invaders:
            invader.draw(surface)
    
    def get_random_shooter(self):
        """Select a random invader to shoot"""
        if random.random() > INVADER_FIRING_CHANCE:
            return None
            
        # Get the bottom-most invader in each column
        bottom_invaders = {}
        for invader in self.invaders:
            if not invader.is_alive:
                continue
                
            col = invader.col
            if col not in bottom_invaders or invader.y > bottom_invaders[col].y:
                bottom_invaders[col] = invader
        
        if bottom_invaders:
            shooter = random.choice(list(bottom_invaders.values()))
            return shooter
        
        return None
    
    def all_dead(self):
        """Check if all invaders are dead"""
        return self.invaders_killed >= self.total_invaders


class Barrier:
    """A defensive barrier that can be damaged"""
    
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.width = BARRIER_WIDTH
        self.height = BARRIER_HEIGHT
        self.sprite = GraphicsGenerator.create_barrier()
        self.damage_map = {}  # Tracks damaged pieces
        
    def check_collision(self, rect):
        """Check if a bullet collided with an undamaged part of the barrier"""
        # Convert rect to barrier-local coordinates
        local_x = rect.x - self.x
        local_y = rect.y - self.y
        
        # Don't check outside barrier bounds
        if (local_x < 0 or local_x >= self.width or 
            local_y < 0 or local_y >= self.height):
            return False
        
        # Check if this section is already fully damaged
        piece_size = 5
        for y in range(local_y, min(local_y + rect.height, self.height), piece_size):
            for x in range(local_x, min(local_x + rect.width, self.width), piece_size):
                # Get the pixel at this position
                piece_key = (x // piece_size, y // piece_size)
                if piece_key not in self.damage_map:
                    pixel_pos = (self.x + x, self.y + y)
                    # Check if there's a non-transparent pixel at this position
                    if self.sprite.get_at((x, y))[3] > 0:  # Alpha > 0 means not transparent
                        return True
        
        return False
    
    def damage(self, rect):
        """Damage the barrier where the bullet hit"""
        # Convert rect to barrier-local coordinates
        local_x = rect.x - self.x
        local_y = rect.y - self.y
        
        # Don't damage outside barrier bounds
        if (local_x < 0 or local_x >= self.width or 
            local_y < 0 or local_y >= self.height):
            return
        
        # Damage pieces in the collision area
        piece_size = 5
        damage_radius = 2  # How many pieces around the hit to damage
        
        center_x = local_x + rect.width // 2
        center_y = local_y + rect.height // 2
        center_piece_x = center_x // piece_size
        center_piece_y = center_y // piece_size
        
        for dy in range(-damage_radius, damage_radius + 1):
            for dx in range(-damage_radius, damage_radius + 1):
                piece_x = center_piece_x + dx
                piece_y = center_piece_y + dy
                
                # Skip if outside barrier
                if (piece_x < 0 or piece_x >= self.width // piece_size or
                    piece_y < 0 or piece_y >= self.height // piece_size):
                    continue
                
                piece_key = (piece_x, piece_y)
                
                # Damage the piece if it's not already fully damaged
                damage_level = self.damage_map.get(piece_key, 0)
                if damage_level < BARRIER_DAMAGE_LEVELS:
                    self.damage_map[piece_key] = damage_level + 1
                
                # Clear the piece from the sprite if fully damaged
                if self.damage_map[piece_key] >= BARRIER_DAMAGE_LEVELS:
                    # Create a small transparent rect to "erase" the damaged piece
                    erase_rect = pygame.Rect(
                        piece_x * piece_size, piece_y * piece_size, 
                        piece_size, piece_size
                    )
                    self.sprite.fill((0, 0, 0, 0), erase_rect)
    
    def draw(self, surface):
        """Draw the barrier with damage"""
        surface.blit(self.sprite, (self.x, self.y))


class MysteryShip:
    """The special mystery ship that appears at the top periodically"""
    
    def __init__(self):
        self.width = MYSTERY_SHIP_WIDTH
        self.height = MYSTERY_SHIP_HEIGHT
        self.speed = MYSTERY_SHIP_SPEED
        self.sprite = GraphicsGenerator.create_mystery_ship()
        self.points = MYSTERY_SHIP_POINTS
        self.is_active = False
        self.direction = 1  # 1 for right to left, -1 for left to right
        self.x = 0
        self.y = GAME_AREA_MARGIN_Y + 20  # Near the top
        self.rect = pygame.Rect(self.x, self.y, self.width, self.height)
        
    def activate(self, screen_width):
        """Start the mystery ship moving across the screen"""
        if self.is_active:
            return
            
        self.is_active = True
        self.direction = random.choice([-1, 1])
        
        if self.direction > 0:
            # Moving right, start at left edge
            self.x = -self.width
        else:
            # Moving left, start at right edge
            self.x = screen_width
            
        self.rect.x = self.x
        self.rect.y = self.y
    
    def update(self, screen_width):
        """Update the mystery ship position"""
        if not self.is_active:
            return
            
        self.x += self.direction * self.speed
        self.rect.x = self.x
        
        # Deactivate if we've gone off screen
        if ((self.direction > 0 and self.x > screen_width) or
            (self.direction < 0 and self.x < -self.width)):
            self.is_active = False
    
    def draw(self, surface):
        """Draw the mystery ship"""
        if self.is_active:
            surface.blit(self.sprite, (self.x, self.y))
    
    def hit(self):
        """Handle being hit by player bullet"""
        self.is_active = False
        # Return random point value
        return random.choice(self.points)
    
    def get_collision_rect(self):
        """Return rectangle for collision detection"""
        if self.is_active:
            return self.rect
        return None


class Bullet:
    """Base class for bullets"""
    
    def __init__(self, x, y, width, height, speed):
        self.x = x
        self.y = y
        self.width = width
        self.height = height
        self.speed = speed
        self.active = True
        self.rect = pygame.Rect(x, y, width, height)
    
    def update(self):
        """Update bullet position"""
        pass
    
    def draw(self, surface):
        """Draw the bullet"""
        pass
    
    def get_collision_rect(self):
        """Return rectangle for collision detection"""
        if self.active:
            return self.rect
        return None


class PlayerBullet(Bullet):
    """A bullet fired by the player"""
    
    def __init__(self, x, y):
        super().__init__(x, y, PLAYER_BULLET_WIDTH, PLAYER_BULLET_HEIGHT, PLAYER_BULLET_SPEED)
        self.sprite = GraphicsGenerator.create_bullet(0)  # 0 for player bullet
    
    def update(self):
        """Move bullet upwards"""
        if not self.active:
            return
            
        self.y -= self.speed
        self.rect.y = self.y
        
        # Deactivate if it goes off the top of the screen
        if self.y < 0:
            self.active = False
    
    def draw(self, surface):
        """Draw player bullet"""
        if self.active:
            surface.blit(self.sprite, (self.x, self.y))


class InvaderBullet(Bullet):
    """A bullet fired by an invader"""
    
    def __init__(self, x, y):
        super().__init__(x, y, INVADER_BULLET_WIDTH, INVADER_BULLET_HEIGHT, INVADER_BULLET_SPEED)
        self.sprite = GraphicsGenerator.create_bullet(1)  # 1 for invader bullet
    
    def update(self):
        """Move bullet downwards"""
        if not self.active:
            return
            
        self.y += self.speed
        self.rect.y = self.y
        
        # Deactivate if it goes off the bottom of the screen
        if self.y > SCREEN_HEIGHT:
            self.active = False
    
    def draw(self, surface):
        """Draw invader bullet"""
        if self.active:
            surface.blit(self.sprite, (self.x, self.y))