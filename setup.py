#!/usr/bin/env python3
from setuptools import setup, find_packages

setup(
    name="space-invaders",
    version="1.0.0",
    description="A retro Space Invaders homage created with Pygame",
    author="Ian",
    author_email="your@email.com",
    packages=find_packages(),
    include_package_data=True,
    install_requires=[
        "pygame>=2.5.0",
        "numpy>=1.24.0",
    ],
    entry_points={
        "console_scripts": [
            "invaders=space_invaders.main:main",
        ],
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Topic :: Games/Entertainment :: Arcade",
    ],
    python_requires=">=3.8",
)