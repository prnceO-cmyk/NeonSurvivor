# Neon Survivor
My first low-level game ever, it took me such a long  time to do it. Very simple game, but no Game engine used I made everything from scratch, it was such a hard work but worth it.

# Technologies
- C++
- CMake
- SDL3
- OpenGL

# Controls

WASD / Arrow Keys: Move
Mouse: Aim
Left Mouse Button / Space: Shoot
R: Restart after game over
Esc: Quit

# Build

Requirements: CMake, a C++ compiler, SDL3, and OpenGL development libraries.

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build -j"$(nproc)"

./build/neon_survivor

# Packaging

Build the Debian package with:

cd build
cpack -G DEB

# Download link

If you wanna to download it directly and test it, when website is published i'll paste the link here.
It's only available for Linux, deb file based on Debian. In the future I'll add exe file for windows.
