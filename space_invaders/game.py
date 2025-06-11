#!/usr/bin/env python3
import os
import json
import math
import pygame
import random
import time
from space_invaders.constants import *
from space_invaders.entities import Player, InvaderGroup, Barrier, MysteryShip, PlayerBullet, InvaderBullet
from space_invaders.graphics import GraphicsGenerator
from space_invaders.sound import SoundGenerator

class HighScoreManager:
    """Manages the high score list and file operations"""
    
    def __init__(self):
        """Initialize the high score manager"""
        self.high_scores = []
        self.load_scores()
        
    def load_scores(self):
        """Load high scores from file"""
        try:
            if os.path.exists(HIGH_SCORE_FILE):
                with open(HIGH_SCORE_FILE, 'r') as file:
                    self.high_scores = json.load(file)
            else:
                # Create default high scores if file doesn't exist
                self.high_scores = [
                    {"name": "CLAUDE", "score": 1000, "level": 3},
                    {"name": "IAN", "score": 800, "level": 2},
                    {"name": "CPU", "score": 600, "level": 2},
                    {"name": "AI", "score": 400, "level": 1},
                    {"name": "ML", "score": 200, "level": 1},
                    {"name": "GPT", "score": 150, "level": 1},
                    {"name": "HAL", "score": 100, "level": 1},
                    {"name": "R2D2", "score": 75, "level": 1},
                    {"name": "C3PO", "score": 50, "level": 1},
                    {"name": "WALL-E", "score": 25, "level": 1}
                ]
                self.save_scores()
        except Exception as e:
            print(f"Error loading high scores: {e}")
            self.high_scores = []
    
    def save_scores(self):
        """Save high scores to file"""
        try:
            with open(HIGH_SCORE_FILE, 'w') as file:
                json.dump(self.high_scores, file)
        except Exception as e:
            print(f"Error saving high scores: {e}")
    
    def is_high_score(self, score):
        """Check if a score qualifies for the high score list"""
        if len(self.high_scores) < HIGH_SCORE_COUNT:
            return True
        return score > min(entry["score"] for entry in self.high_scores)
    
    def add_score(self, name, score, level):
        """Add a new high score entry"""
        new_entry = {"name": name, "score": score, "level": level}
        self.high_scores.append(new_entry)
        # Sort by score (highest first)
        self.high_scores.sort(key=lambda x: x["score"], reverse=True)
        # Keep only top scores
        self.high_scores = self.high_scores[:HIGH_SCORE_COUNT]
        self.save_scores()
    
    def get_high_scores(self):
        """Return the list of high scores"""
        return self.high_scores
    
    def reset_scores(self):
        """Reset high scores to default"""
        if os.path.exists(HIGH_SCORE_FILE):
            os.remove(HIGH_SCORE_FILE)
        self.load_scores()


class SpaceInvadersGame:
    """Main game class that manages the game logic and rendering"""
    
    def __init__(self):
        """Initialize the game"""
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption(SCREEN_TITLE)
        self.clock = pygame.time.Clock()
        self.game_area = pygame.Rect(
            GAME_AREA_MARGIN_X, GAME_AREA_MARGIN_Y, 
            GAME_AREA_WIDTH, GAME_AREA_HEIGHT
        )
        self.running = True
        self.game_state = STATE_ATTRACT
        
        # Create sound generator and generate sounds
        self.sound_generator = SoundGenerator()
        self.sound_generator.generate_all_sounds()
        
        # Initialize high score manager
        self.high_score_manager = HighScoreManager()
        
        # Initialize game objects
        self.init_game_objects()
        
        # Game state variables
        self.score = 0
        # Initialize high score from the hall of fame's highest score
        self.high_score = 0
        high_scores = self.high_score_manager.get_high_scores()
        if high_scores:
            self.high_score = high_scores[0]["score"]
        
        self.level = 1
        self.last_invader_movement_sound = 0
        self.invader_movement_sound_index = 0
        self.explosion_sprites = []
        self.explosion_timers = []
        self.game_over_timer = 0
        self.digit_sprites = GraphicsGenerator.create_digit_sprites()
        
        # Level transition variables
        self.transition_timer = 0
        self.transition_delay = 2000  # 2 seconds
        
        # High score entry variables
        self.player_name = ["A"] * HIGH_SCORE_NAME_LENGTH  # Default name
        self.current_char = 0  # Current character position
        self.char_index = 0    # Index in the HIGH_SCORE_CHARS
        self.name_entry_cooldown = 0
        self.name_entry_delay = 150  # ms between character changes
        
        # Hall of Fame variables
        self.scroll_position = 0
        self.scroll_timer = 0
        self.scroll_speed = 1  # pixels per frame
        
    def init_game_objects(self):
        """Initialize or reset all game objects"""
        # Create player
        player_x = SCREEN_WIDTH // 2 - PLAYER_WIDTH // 2
        player_y = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - PLAYER_HEIGHT - 20
        self.player = Player(player_x, player_y)
        
        # Create invader group
        self.invader_group = InvaderGroup()
        self.invader_group.create_invaders()
        
        # Create barriers
        self.barriers = []
        barrier_spacing = GAME_AREA_WIDTH // (BARRIER_COUNT + 1)
        for i in range(BARRIER_COUNT):
            barrier_x = GAME_AREA_MARGIN_X + barrier_spacing * (i + 1) - (BARRIER_WIDTH // 2)
            barrier_y = BARRIER_Y_POS
            self.barriers.append(Barrier(barrier_x, barrier_y))
        
        # Create mystery ship
        self.mystery_ship = MysteryShip()
        
        # Bullet lists
        self.player_bullets = []
        self.invader_bullets = []
        
        # Game timing
        self.last_mystery_ship_time = pygame.time.get_ticks()
        self.mystery_ship_delay = random.randint(15000, 30000)  # 15-30 seconds
        
    def handle_events(self):
        """Process user input"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
                
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.running = False
                    
                elif event.key == pygame.K_RETURN:
                    # Start game from attract mode
                    if self.game_state == STATE_ATTRACT:
                        self.start_new_game()
                    # Restart after game over
                    elif self.game_state == STATE_GAME_OVER:
                        if pygame.time.get_ticks() - self.game_over_timer > 2000:
                            self.start_new_game()
                    # Start new game from hall of fame
                    elif self.game_state == STATE_HALL_OF_FAME:
                        self.start_new_game()
                    # Submit name in high score entry
                    elif self.game_state == STATE_HIGH_SCORE_ENTRY:
                        self.submit_high_score()
                
                elif event.key == pygame.K_SPACE:
                    # Shoot only when playing
                    if self.game_state == STATE_PLAYING and self.player.is_alive:
                        self.try_player_shoot()
                    # Submit name in high score entry
                    elif self.game_state == STATE_HIGH_SCORE_ENTRY:
                        self.submit_high_score()
                
                elif event.key == pygame.K_q:
                    # Reset high scores in hall of fame screen
                    if self.game_state == STATE_HALL_OF_FAME:
                        self.high_score_manager.reset_scores()
                
                # High score name entry navigation
                elif self.game_state == STATE_HIGH_SCORE_ENTRY:
                    if event.key == pygame.K_LEFT:
                        self.current_char = (self.current_char - 1) % HIGH_SCORE_NAME_LENGTH
                        self.name_entry_cooldown = pygame.time.get_ticks()
                    
                    elif event.key == pygame.K_RIGHT:
                        self.current_char = (self.current_char + 1) % HIGH_SCORE_NAME_LENGTH
                        self.name_entry_cooldown = pygame.time.get_ticks()
                    
                    elif event.key == pygame.K_UP:
                        # Find the current character in the character list
                        try:
                            self.char_index = HIGH_SCORE_CHARS.index(self.player_name[self.current_char])
                        except ValueError:
                            self.char_index = 0
                            
                        # Move to previous character
                        self.char_index = (self.char_index - 1) % len(HIGH_SCORE_CHARS)
                        self.player_name[self.current_char] = HIGH_SCORE_CHARS[self.char_index]
                        self.name_entry_cooldown = pygame.time.get_ticks()
                    
                    elif event.key == pygame.K_DOWN:
                        # Find the current character in the character list
                        try:
                            self.char_index = HIGH_SCORE_CHARS.index(self.player_name[self.current_char])
                        except ValueError:
                            self.char_index = 0
                            
                        # Move to next character
                        self.char_index = (self.char_index + 1) % len(HIGH_SCORE_CHARS)
                        self.player_name[self.current_char] = HIGH_SCORE_CHARS[self.char_index]
                        self.name_entry_cooldown = pygame.time.get_ticks()
        
        # Check continuous key presses for gameplay
        if self.game_state == STATE_PLAYING and self.player.is_alive:
            keys = pygame.key.get_pressed()
            
            if keys[pygame.K_LEFT] or keys[pygame.K_a]:
                self.player.move(-1, self.game_area)
                
            if keys[pygame.K_RIGHT] or keys[pygame.K_d]:
                self.player.move(1, self.game_area)
                
            if keys[pygame.K_SPACE]:
                self.try_player_shoot()
                
    def try_player_shoot(self):
        """Try to fire a player bullet if cooldown allows"""
        current_time = pygame.time.get_ticks()
        
        # Check if player can shoot based on cooldown
        if self.player.can_shoot(current_time):
            # Create a new bullet
            bullet = self.player.shoot(current_time)
            self.player_bullets.append(bullet)
            
            # Play sound
            self.sound_generator.play_sound("player_shoot")
            
    def update(self):
        """Update game state"""
        current_time = pygame.time.get_ticks()
        
        # Handle different game states
        if self.game_state == STATE_LEVEL_TRANSITION:
            # Handle level transition timing
            if current_time - self.transition_timer > self.transition_delay:
                self.complete_level_transition()
                
        elif self.game_state == STATE_PLAYING:
            # Update player bullets
            for bullet in self.player_bullets[:]:
                bullet.update()
                if not bullet.active:
                    self.player_bullets.remove(bullet)
            
            # Update invader bullets
            for bullet in self.invader_bullets[:]:
                bullet.update()
                if not bullet.active:
                    self.invader_bullets.remove(bullet)
            
            # Update invader movement
            if self.invader_group.move(current_time, self.game_area):
                # Play movement sound when invaders move down
                self.sound_generator.play_sound("invader_movement")
                self.last_invader_movement_sound = current_time
                self.invader_movement_sound_index = (self.invader_movement_sound_index + 1) % 4
            
            # Check if invaders have reached the bottom
            if self.invader_group.any_invader_at_bottom(self.player.y):
                self.game_over()
                
            # Random invader shooting
            shooter = self.invader_group.get_random_shooter()
            if shooter and shooter.is_alive:
                bullet_x = shooter.x + (shooter.width // 2) - (INVADER_BULLET_WIDTH // 2)
                bullet_y = shooter.y + shooter.height
                self.invader_bullets.append(InvaderBullet(bullet_x, bullet_y))
                self.sound_generator.play_sound("invader_shoot")
            
            # Mystery ship logic
            if not self.mystery_ship.is_active:
                if current_time - self.last_mystery_ship_time > self.mystery_ship_delay:
                    self.mystery_ship.activate(SCREEN_WIDTH)
                    self.last_mystery_ship_time = current_time
                    self.mystery_ship_delay = random.randint(15000, 30000)  # 15-30 seconds
                    self.sound_generator.play_sound("mystery_ship")
            else:
                self.mystery_ship.update(SCREEN_WIDTH)
                if not self.mystery_ship.is_active:
                    self.sound_generator.stop_sound("mystery_ship")
            
            # Check collisions
            self.check_collisions()
            
            # Check if all invaders are dead
            if self.invader_group.all_dead():
                self.start_level_transition()
        
        elif self.game_state == STATE_HIGH_SCORE_ENTRY:
            self.update_high_score_entry()
        
        elif self.game_state == STATE_HALL_OF_FAME:
            self.update_hall_of_fame()
        
        # Update explosions (do this in all states)
        expired_explosions = []
        for i, timer in enumerate(self.explosion_timers):
            if current_time - timer > 500:  # Explosion lasts 500ms
                expired_explosions.append(i)
                
        # Remove expired explosions from the end to avoid index shifting issues
        for i in reversed(expired_explosions):
            self.explosion_timers.pop(i)
            self.explosion_sprites.pop(i)
            
    def start_level_transition(self):
        """Start the level transition sequence"""
        print(f"Starting level transition from level {self.level}")
        self.game_state = STATE_LEVEL_TRANSITION
        self.transition_timer = pygame.time.get_ticks()
            
    def complete_level_transition(self):
        """Complete the level transition and start the next level"""
        old_level = self.level
        self.level += 1
        print(f"Completing level transition from {old_level} to {self.level}")
        
        # Clear bullets
        self.player_bullets = []
        self.invader_bullets = []
        
        # Reset invaders but make them faster
        self.invader_group = InvaderGroup()  # Create a completely new invader group
        self.invader_group.create_invaders()
        self.invader_group.move_delay = max(100, 1000 - (self.level * 100))  # Progressively faster
        
        # Return to playing state
        self.game_state = STATE_PLAYING
                
    def check_collisions(self):
        """Check for collisions between game objects"""
        # Player bullets vs invaders
        for bullet in self.player_bullets[:]:
            if not bullet.active:
                continue
                
            # Check mystery ship collision
            mystery_rect = self.mystery_ship.get_collision_rect()
            if mystery_rect and bullet.get_collision_rect().colliderect(mystery_rect):
                points = self.mystery_ship.hit()
                self.score += points
                print(f"Mystery ship hit! Score: {self.score}")  # Debug output
                bullet.active = False
                self.player_bullets.remove(bullet)
                self.add_explosion(self.mystery_ship.x, self.mystery_ship.y)
                self.sound_generator.stop_sound("mystery_ship")
                self.sound_generator.play_sound("mystery_ship_hit")
                continue
            
            # Check invader collisions
            for invader in self.invader_group.invaders:
                invader_rect = invader.get_collision_rect()
                if invader_rect and bullet.get_collision_rect().colliderect(invader_rect):
                    invader.is_alive = False
                    self.invader_group.invader_killed()
                    points = invader.get_points()
                    self.score += points
                    print(f"Invader hit! Type: {invader.type}, Points: {points}, Total Score: {self.score}")  # Debug output
                    bullet.active = False
                    self.player_bullets.remove(bullet)
                    self.add_explosion(invader.x, invader.y)
                    self.sound_generator.play_sound("invader_explosion")
                    break
        
        # Player bullets vs barriers
        for bullet in self.player_bullets[:]:
            if not bullet.active:
                continue
                
            for barrier in self.barriers:
                if barrier.check_collision(bullet.get_collision_rect()):
                    barrier.damage(bullet.get_collision_rect())
                    bullet.active = False
                    self.player_bullets.remove(bullet)
                    break
        
        # Invader bullets vs player
        if self.player.is_alive:
            player_rect = self.player.get_collision_rect()
            for bullet in self.invader_bullets[:]:
                if bullet.active and bullet.get_collision_rect().colliderect(player_rect):
                    self.player.hit()
                    bullet.active = False
                    self.invader_bullets.remove(bullet)
                    self.add_explosion(self.player.x, self.player.y)
                    self.sound_generator.play_sound("player_explosion")
                    
                    # Check if game over
                    if self.player.lives <= 0:
                        self.game_over()
                    else:
                        # Respawn player after a delay
                        pygame.time.delay(1000)
                        player_x = SCREEN_WIDTH // 2 - PLAYER_WIDTH // 2
                        player_y = SCREEN_HEIGHT - GAME_AREA_MARGIN_Y - PLAYER_HEIGHT - 20
                        self.player.reset_position(player_x, player_y)
                    
                    break
        
        # Invader bullets vs barriers
        for bullet in self.invader_bullets[:]:
            if not bullet.active:
                continue
                
            for barrier in self.barriers:
                if barrier.check_collision(bullet.get_collision_rect()):
                    barrier.damage(bullet.get_collision_rect())
                    bullet.active = False
                    self.invader_bullets.remove(bullet)
                    break
                    
    def add_explosion(self, x, y):
        """Add an explosion effect at the given position"""
        size = 40  # Size of explosion
        explosion_sprite = GraphicsGenerator.create_explosion(size)
        self.explosion_sprites.append((explosion_sprite, (x, y)))
        self.explosion_timers.append(pygame.time.get_ticks())
                    
    def start_new_game(self):
        """Start a new game"""
        self.score = 0
        self.level = 1
        self.init_game_objects()
        self.game_state = STATE_PLAYING
        
    def game_over(self):
        """Handle game over state"""
        # Update high score
        if self.score > self.high_score:
            self.high_score = self.score
            
        # Check if score qualifies for high score list
        if self.high_score_manager.is_high_score(self.score):
            # Show high score entry screen
            self.game_state = STATE_HIGH_SCORE_ENTRY
            self.player_name = ["A"] * HIGH_SCORE_NAME_LENGTH
            self.current_char = 0
            self.char_index = 0
            self.name_entry_cooldown = 0
        else:
            # Standard game over
            self.game_state = STATE_GAME_OVER
            self.game_over_timer = pygame.time.get_ticks()
            self.sound_generator.play_sound("game_over")
        
    def next_level(self):
        """Set up the next level"""
        self.level += 1
        
        # Clear bullets
        self.player_bullets = []
        self.invader_bullets = []
        
        # Reset invaders but make them faster
        self.invader_group.create_invaders()
        self.invader_group.move_delay = max(100, self.invader_group.move_delay - 100)
        
    def draw_score(self):
        """Draw the score and high score at the top of the screen"""
        # Draw score area background (solid black rectangle)
        score_bg_height = 45
        score_bg_rect = pygame.Rect(0, 0, SCREEN_WIDTH, score_bg_height)
        pygame.draw.rect(self.screen, BLACK, score_bg_rect)
        
        # Draw "SCORE" text - positioned at the very top
        score_text = GraphicsGenerator.create_text_surface("SCORE", FONT_SMALL)
        self.screen.blit(score_text, (50, 2))
        
        # Draw "HIGH SCORE" text - positioned at the very top
        high_score_text = GraphicsGenerator.create_text_surface("HIGH SCORE", FONT_SMALL)
        high_score_x = SCREEN_WIDTH - 50 - high_score_text.get_width()
        self.screen.blit(high_score_text, (high_score_x, 2))
        
        # Draw actual score - just below the header
        score_str = str(self.score).zfill(4)
        score_x = 50
        score_y = 20
        self.draw_digits(score_str, score_x, score_y)
        
        # Draw high score - just below the header
        high_score_str = str(self.high_score).zfill(4)
        high_score_x = SCREEN_WIDTH - 50 - (len(high_score_str) * 20)  # Slightly reduced spacing
        self.draw_digits(high_score_str, high_score_x, score_y)
        
    def draw_digits(self, number_str, x, y):
        """Draw a series of digits using the custom digit sprites"""
        digit_width = 22  # Width of each digit including spacing
        
        for i, digit in enumerate(number_str):
            digit_index = int(digit)
            digit_sprite = self.digit_sprites[digit_index]
            self.screen.blit(digit_sprite, (x + i * digit_width, y))
            
    def draw_lives(self):
        """Draw the player's remaining lives"""
        lives_text = GraphicsGenerator.create_text_surface(f"LIVES:", FONT_SMALL)
        self.screen.blit(lives_text, (50, SCREEN_HEIGHT - 40))
        
        # Draw small player ships for each life
        ship_width = PLAYER_WIDTH // 2
        ship_height = PLAYER_HEIGHT // 2
        ship_sprite = pygame.transform.scale(self.player.sprite, (ship_width, ship_height))
        
        for i in range(self.player.lives):
            ship_x = 120 + (i * (ship_width + 10))
            ship_y = SCREEN_HEIGHT - 40
            self.screen.blit(ship_sprite, (ship_x, ship_y))
            
    def draw(self):
        """Draw the game screen"""
        # Clear the screen
        self.screen.fill(BLACK)
        
        # Draw game border
        pygame.draw.rect(self.screen, WHITE, self.game_area, 1)
        
        # Draw score and high score
        self.draw_score()
        
        if self.game_state == STATE_ATTRACT:
            # Draw title screen
            title_text = GraphicsGenerator.create_text_surface("SPACE INVADERS", FONT_LARGE)
            title_x = SCREEN_WIDTH // 2 - title_text.get_width() // 2
            self.screen.blit(title_text, (title_x, SCREEN_HEIGHT // 3))
            
            # Draw blinking "PRESS ENTER TO START"
            if (pygame.time.get_ticks() // 500) % 2 == 0:
                start_text = GraphicsGenerator.create_text_surface("PRESS ENTER TO START", FONT_MEDIUM)
                start_x = SCREEN_WIDTH // 2 - start_text.get_width() // 2
                self.screen.blit(start_text, (start_x, SCREEN_HEIGHT // 2))
                
        elif self.game_state == STATE_LEVEL_TRANSITION:
            # Draw level transition screen
            level_complete_text = GraphicsGenerator.create_text_surface(f"LEVEL {self.level} COMPLETE!", FONT_LARGE)
            x = SCREEN_WIDTH // 2 - level_complete_text.get_width() // 2
            y = SCREEN_HEIGHT // 2 - level_complete_text.get_height() // 2
            self.screen.blit(level_complete_text, (x, y))
            
            next_level_text = GraphicsGenerator.create_text_surface(f"PREPARING LEVEL {self.level + 1}...", FONT_MEDIUM)
            x = SCREEN_WIDTH // 2 - next_level_text.get_width() // 2
            y = SCREEN_HEIGHT // 2 + 50
            self.screen.blit(next_level_text, (x, y))
            
            # Continue displaying game elements in the background
            self.player.draw(self.screen)
            for barrier in self.barriers:
                barrier.draw(self.screen)
                
            # Draw explosions
            for explosion_sprite, pos in self.explosion_sprites:
                self.screen.blit(explosion_sprite, pos)
                
        elif self.game_state == STATE_PLAYING or self.game_state == STATE_GAME_OVER:
            # Draw player
            self.player.draw(self.screen)
            
            # Draw invaders
            self.invader_group.draw(self.screen)
            
            # Draw mystery ship
            self.mystery_ship.draw(self.screen)
            
            # Draw barriers
            for barrier in self.barriers:
                barrier.draw(self.screen)
                
            # Draw bullets
            for bullet in self.player_bullets:
                bullet.draw(self.screen)
                
            for bullet in self.invader_bullets:
                bullet.draw(self.screen)
                
            # Draw explosions
            for explosion_sprite, pos in self.explosion_sprites:
                self.screen.blit(explosion_sprite, pos)
                
            # Draw lives
            self.draw_lives()
            
            # Draw level
            level_text = GraphicsGenerator.create_text_surface(f"LEVEL: {self.level}", FONT_SMALL)
            level_x = SCREEN_WIDTH - 50 - level_text.get_width()
            self.screen.blit(level_text, (level_x, SCREEN_HEIGHT - 40))
            
            # Draw game over text
            if self.game_state == STATE_GAME_OVER:
                game_over_text = GraphicsGenerator.create_text_surface("GAME OVER", FONT_LARGE)
                game_over_x = SCREEN_WIDTH // 2 - game_over_text.get_width() // 2
                self.screen.blit(game_over_text, (game_over_x, SCREEN_HEIGHT // 2))
                
                if (pygame.time.get_ticks() // 500) % 2 == 0 and pygame.time.get_ticks() - self.game_over_timer > 2000:
                    restart_text = GraphicsGenerator.create_text_surface("PRESS ENTER TO RESTART", FONT_MEDIUM)
                    restart_x = SCREEN_WIDTH // 2 - restart_text.get_width() // 2
                    self.screen.blit(restart_text, (restart_x, SCREEN_HEIGHT // 2 + 50))
        
        elif self.game_state == STATE_HIGH_SCORE_ENTRY:
            # Draw retro-styled high score entry screen with decorative elements
            # Draw a starfield background effect - reduced frequency for photosensitivity
            for _ in range(20):  # Reduced from 50 to 20 stars
                x = random.randint(0, SCREEN_WIDTH)
                y = random.randint(0, SCREEN_HEIGHT)
                size = random.randint(1, 2)  # Reduced max size from 3 to 2
                pygame.draw.circle(self.screen, WHITE, (x, y), size)
            
            # Draw header with pulsing effect
            pulse_size = int(abs(math.sin(pygame.time.get_ticks() / 500)) * 8)
            entry_text = GraphicsGenerator.create_text_surface("NEW HIGH SCORE!", FONT_LARGE + pulse_size)
            entry_x = SCREEN_WIDTH // 2 - entry_text.get_width() // 2
            self.screen.blit(entry_text, (entry_x, SCREEN_HEIGHT // 4))
            
            # Draw score info
            score_text = GraphicsGenerator.create_text_surface(f"SCORE: {self.score}", FONT_MEDIUM)
            score_x = SCREEN_WIDTH // 2 - score_text.get_width() // 2
            self.screen.blit(score_text, (score_x, SCREEN_HEIGHT // 3))
            
            # Draw entry instructions
            name_text = GraphicsGenerator.create_text_surface("ENTER YOUR NAME:", FONT_MEDIUM)
            name_x = SCREEN_WIDTH // 2 - name_text.get_width() // 2
            self.screen.blit(name_text, (name_x, SCREEN_HEIGHT // 2 - 20))
            
            # Draw controls hint
            controls_text = GraphicsGenerator.create_text_surface("ARROWS TO SELECT, SPACE TO CONFIRM", FONT_SMALL)
            controls_x = SCREEN_WIDTH // 2 - controls_text.get_width() // 2
            self.screen.blit(controls_text, (controls_x, SCREEN_HEIGHT // 2 + 80))
            
            # Draw the player's name with the current character highlighted
            for i, char in enumerate(self.player_name):
                # Determine color - highlight the current character position
                color = YELLOW if i == self.current_char else WHITE
                char_surface = GraphicsGenerator.create_text_surface(char, FONT_LARGE, color)
                
                # Calculate position (centered, with spacing)
                total_width = HIGH_SCORE_NAME_LENGTH * 40  # 40px per character with spacing
                start_x = SCREEN_WIDTH // 2 - total_width // 2
                char_x = start_x + i * 40
                char_y = SCREEN_HEIGHT // 2 + 20
                
                # Draw character
                self.screen.blit(char_surface, (char_x, char_y))
                
                # Draw selection box around current character
                if i == self.current_char:
                    box_rect = pygame.Rect(char_x - 5, char_y - 5, 40, 45)
                    pygame.draw.rect(self.screen, GREEN, box_rect, 2)
            
            # Draw blinking indicator for current position
            if (pygame.time.get_ticks() // 400) % 2 == 0:
                char_index = HIGH_SCORE_CHARS.index(self.player_name[self.current_char])
                
                # Draw up arrow above current character
                up_char = HIGH_SCORE_CHARS[(char_index - 1) % len(HIGH_SCORE_CHARS)]
                up_text = GraphicsGenerator.create_text_surface(up_char, FONT_SMALL, (100, 100, 100))
                
                # Calculate position
                total_width = HIGH_SCORE_NAME_LENGTH * 40
                start_x = SCREEN_WIDTH // 2 - total_width // 2
                up_x = start_x + self.current_char * 40
                up_y = SCREEN_HEIGHT // 2 - 5
                
                # Draw indicator
                self.screen.blit(up_text, (up_x, up_y))
                
                # Draw down arrow below current character
                down_char = HIGH_SCORE_CHARS[(char_index + 1) % len(HIGH_SCORE_CHARS)]
                down_text = GraphicsGenerator.create_text_surface(down_char, FONT_SMALL, (100, 100, 100))
                down_y = SCREEN_HEIGHT // 2 + 70
                
                self.screen.blit(down_text, (up_x, down_y))
        
        elif self.game_state == STATE_HALL_OF_FAME:
            # Draw retro-styled hall of fame display
            
            # Draw a starfield background effect - reduced frequency for photosensitivity
            for _ in range(40):  # Reduced from 100 to 40 stars
                x = random.randint(0, SCREEN_WIDTH)
                y = random.randint(0, SCREEN_HEIGHT)
                size = random.randint(1, 2)  # Reduced max size from 3 to 2
                pygame.draw.circle(self.screen, WHITE, (x, y), size)
            
            # Draw title - removed blue box background
            title_y = 50
            title_text = GraphicsGenerator.create_text_surface("* HALL OF FAME *", FONT_LARGE)
            title_x = SCREEN_WIDTH // 2 - title_text.get_width() // 2
            
            # Simple underline instead of box
            underline_y = title_y + title_text.get_height() + 5
            pygame.draw.line(self.screen, WHITE, 
                            (title_x - 10, underline_y), 
                            (title_x + title_text.get_width() + 10, underline_y), 2)
            
            self.screen.blit(title_text, (title_x, title_y))
            
            # Draw table headers
            header_y = 120
            pygame.draw.line(self.screen, WHITE, (100, header_y), (SCREEN_WIDTH - 100, header_y), 2)
            
            rank_text = GraphicsGenerator.create_text_surface("RANK", FONT_SMALL)
            name_text = GraphicsGenerator.create_text_surface("NAME", FONT_SMALL)
            score_text = GraphicsGenerator.create_text_surface("SCORE", FONT_SMALL)
            level_text = GraphicsGenerator.create_text_surface("LEVEL", FONT_SMALL)
            
            self.screen.blit(rank_text, (120, header_y - 25))
            self.screen.blit(name_text, (220, header_y - 25))
            self.screen.blit(score_text, (400, header_y - 25))
            self.screen.blit(level_text, (550, header_y - 25))
            
            # Draw high scores with alternating row colors
            high_scores = self.high_score_manager.get_high_scores()
            start_y = 150
            
            for i, entry in enumerate(high_scores):
                row_y = start_y + i * 35
                
                # Draw alternating row backgrounds
                if i % 2 == 0:
                    pygame.draw.rect(self.screen, (20, 20, 50), 
                                    (100, row_y - 5, SCREEN_WIDTH - 200, 30))
                
                # Get entry data
                rank = f"{i + 1}."
                name = entry["name"]
                score = str(entry["score"]).zfill(4)
                level = f"{entry['level']}"
                
                # Highlight top 3 rankings with special colors
                rank_color = WHITE
                if i == 0:
                    rank_color = YELLOW  # Gold for 1st
                elif i == 1:
                    rank_color = (200, 200, 200)  # Silver for 2nd
                elif i == 2:
                    rank_color = (184, 115, 51)  # Bronze for 3rd
                
                # Draw entry data
                rank_surf = GraphicsGenerator.create_text_surface(rank, FONT_MEDIUM, rank_color)
                name_surf = GraphicsGenerator.create_text_surface(name, FONT_MEDIUM, WHITE)
                score_surf = GraphicsGenerator.create_text_surface(score, FONT_MEDIUM, GREEN)
                level_surf = GraphicsGenerator.create_text_surface(level, FONT_MEDIUM, RED)
                
                self.screen.blit(rank_surf, (120, row_y))
                self.screen.blit(name_surf, (220, row_y))
                self.screen.blit(score_surf, (400, row_y))
                self.screen.blit(level_surf, (550, row_y))
            
            # Draw instructions
            pygame.draw.rect(self.screen, BLACK, (0, SCREEN_HEIGHT - 70, SCREEN_WIDTH, 70))
            
            keys_y = SCREEN_HEIGHT - 60
            enter_text = GraphicsGenerator.create_text_surface("ENTER: NEW GAME", FONT_SMALL)
            q_text = GraphicsGenerator.create_text_surface("Q: RESET SCORES", FONT_SMALL)
            esc_text = GraphicsGenerator.create_text_surface("ESC: QUIT", FONT_SMALL)
            
            self.screen.blit(enter_text, (120, keys_y))
            self.screen.blit(q_text, (350, keys_y))
            self.screen.blit(esc_text, (580, keys_y))
            
            # Draw scrolling banner at bottom with credits
            banner_y = SCREEN_HEIGHT - 30
            banner_text = "CREATED BY CLAUDE 3.7 AND IAN      *      SPACE INVADERS      *      " + time.strftime("%d %b %Y")
            # Create a doubly long text to handle scrolling
            banner_surface = GraphicsGenerator.create_text_surface(banner_text + "      " + banner_text, FONT_SMALL, YELLOW)
            
            # Create scrolling effect
            scroll_width = banner_surface.get_width()
            if self.scroll_position < -scroll_width // 2:
                self.scroll_position = 0
                
            self.screen.blit(banner_surface, (self.scroll_position, banner_y))
            
            # Draw a decorative border around the screen
            pygame.draw.rect(self.screen, GREEN, (10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20), 1)
        
        # Update the display
        pygame.display.flip()
        
    def run(self):
        """Main game loop"""
        while self.running:
            self.handle_events()
            self.update()
            self.draw()
            self.clock.tick(FPS)
            
        pygame.quit()
        
    def update_high_score_entry(self):
        """Update the high score name entry screen"""
        current_time = pygame.time.get_ticks()
        
        # Handle cooldown for character selection
        if current_time - self.name_entry_cooldown < self.name_entry_delay:
            return
            
        keys = pygame.key.get_pressed()
        
        # Navigate left/right to select character position
        if keys[pygame.K_LEFT]:
            self.current_char = (self.current_char - 1) % HIGH_SCORE_NAME_LENGTH
            self.name_entry_cooldown = current_time
        elif keys[pygame.K_RIGHT]:
            self.current_char = (self.current_char + 1) % HIGH_SCORE_NAME_LENGTH
            self.name_entry_cooldown = current_time
        
        # Change the character at current position
        if keys[pygame.K_UP]:
            # Move to previous character in the list
            self.char_index = (self.char_index - 1) % len(HIGH_SCORE_CHARS)
            self.player_name[self.current_char] = HIGH_SCORE_CHARS[self.char_index]
            self.name_entry_cooldown = current_time
        elif keys[pygame.K_DOWN]:
            # Move to next character in the list
            self.char_index = (self.char_index + 1) % len(HIGH_SCORE_CHARS)
            self.player_name[self.current_char] = HIGH_SCORE_CHARS[self.char_index]
            self.name_entry_cooldown = current_time
            
        # Space or Enter to confirm name
        if keys[pygame.K_SPACE] or keys[pygame.K_RETURN]:
            self.submit_high_score()
            
    def submit_high_score(self):
        """Submit the high score to the hall of fame"""
        name = "".join(self.player_name)
        self.high_score_manager.add_score(name, self.score, self.level)
        self.game_state = STATE_HALL_OF_FAME
        self.scroll_position = 0
        self.scroll_timer = pygame.time.get_ticks()
        
    def update_hall_of_fame(self):
        """Update the hall of fame display"""
        current_time = pygame.time.get_ticks()
        
        # Update scrolling banner position
        self.scroll_position -= self.scroll_speed
        # Reset when text scrolls off screen (assuming banner is 800px wide)
        if self.scroll_position < -800:
            self.scroll_position = SCREEN_WIDTH
            
        # Check for key presses
        keys = pygame.key.get_pressed()
        
        if keys[pygame.K_q]:
            # Reset high scores
            self.high_score_manager.reset_scores()
            
        elif keys[pygame.K_RETURN]:
            # Start a new game
            self.start_new_game()
            
        elif keys[pygame.K_ESCAPE]:
            # Exit game
            self.running = False