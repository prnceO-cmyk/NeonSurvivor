## Neon Survivor

A 2D arcade survival game built with C++, SDL3, and OpenGL.

## Controls

WASD / Arrow Keys: Move
Mouse: Aim
Left Mouse Button / Space: Shoot
R: Restart after game over
Esc: Quit
Build

## Requirements: CMake, a C++17 compiler, SDL3, and OpenGL development libraries.

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
./build/neon_survivor

## Packaging

Build the Debian package with:

cd build
cpack -G DEB

Technologies
C++17
SDL3
OpenGL
CMake