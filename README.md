# Space Invaders

A retro Space Invaders homage created with Pygame.

## Description

This is a Python implementation of the classic arcade game Space Invaders, created entirely from scratch using Pygame. All graphics and sound effects are generated procedurally without using any external assets.

## Features

- Classic Space Invaders gameplay with all major elements from the original
- Procedurally generated graphics
- Synthesized sound effects using NumPy
- Increasingly difficult levels
- Score tracking with high score

## Installation

```bash
# Install with pip
pip install space-invaders

# Or install from source
git clone https://github.com/yourusername/space-invaders.git
cd space-invaders
pip install -e .
```

## How to Play

Once installed, you can run the game using:

```bash
# If installed via pip:
invaders

# Or run the module directly:
python -m space_invaders.main
```

### Controls

- Left/Right Arrow Keys or A/D: Move your ship left and right
- Space: Fire your weapon
- Enter: Start game / Restart after game over
- Escape: Quit the game

## Dependencies

- Python 3.8+
- Pygame 2.5.0+
- NumPy 1.24.0+

## Development

This project uses Poetry for dependency management:

```bash
# Install dependencies
poetry install

# Run the game
poetry run invaders
```

## License

MIT License