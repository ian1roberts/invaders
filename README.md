# Space Invaders

A retro Space Invaders homage created with Pygame.

## Description

This is a Python and C++ implementation of the classic arcade game Space Invaders, created entirely from scratch using Pygame. All graphics and sound effects are generated procedurally without using any external assets. What's more ... Claude Sonnet 3.7 wrote it all with a few wacky prompts ...

## Features

- Classic Space Invaders gameplay with all major elements from the original
- Procedurally generated graphics
- Synthesized sound effects using NumPy
- Increasingly difficult levels
- Score tracking with high score

## Installation

### Python Quickstart 

```bash
# Clone the repository
git clone https://github.com/yourusername/space-invaders.git
cd space-invaders

# Install using Poetry
poetry install

# Activate the Poetry environment
poetry shell

# Run the game
invaders
```

## How to Play

Once installed, you can run the game using:

```bash
# Using Poetry
poetry run invaders

# Or from within a Poetry shell
invaders
```

### Controls

- Left/Right Arrow Keys or A/D: Move your ship left and right
- Space: Fire your weapon
- Enter: Start game / Restart after game over
- Escape: Quit the game

## Dependencies

- Python 3.9+
- Pygame 2.5.0+
- NumPy 1.26.0+

## Development

This project uses Poetry for dependency management:

```bash
# Install dependencies
poetry install

# Run the game
poetry run invaders

# Add new dependencies
poetry add package-name

# Update dependencies
poetry update
```

### C++

Create a build directory, and make

```bash
mkdir build && cd build
cmake ..
make -j4
```
Launch the gave with `./SpaceInvaders`

## License

MIT License