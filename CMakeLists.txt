
cmake_minimum_required(VERSION 3.20)

project(NeonSurvivor
    VERSION 1.0.0
    DESCRIPTION "Neon Survivor - 2D Arcade Game"
    LANGUAGES CXX
)

# C++ configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build SDL3 from the vendored source.
# IMPORTANT: vendored/SDL must contain SDL's CMakeLists.txt.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# OpenGL
find_package(OpenGL REQUIRED)

# Game executable
add_executable(neon_survivor
    src/main.cpp
    src/Game.cpp
    src/Game.hpp
)

target_link_libraries(neon_survivor PRIVATE
    SDL3::SDL3
    OpenGL::GL
)

target_compile_options(neon_survivor PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
)

# Output executable directly into the build directory
set_target_properties(neon_survivor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Installation
include(GNUInstallDirs)

# Install the game executable
install(TARGETS neon_survivor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Bundle SDL3
install(TARGETS SDL3-shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Let the installed executable find bundled SDL3
set_target_properties(neon_survivor PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
)

# Install the game icon
install(FILES "${PROJECT_SOURCE_DIR}/assets/neon-survivor.png"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
)

# Install the desktop launcher
install(FILES "${PROJECT_SOURCE_DIR}/packaging/neon-survivor.desktop"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)



# Find bundled SDL3 beside the installed executable's normal lib directory.
set_target_properties(neon_survivor PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
)


# Debian package (.deb)
set(CPACK_PACKAGE_NAME "neon-survivor")
set(CPACK_PACKAGE_VENDOR "Neon Survivor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "A 2D neon arcade survival game built with SDL3 and OpenGL"
)
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "your-email@example.com")
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_${CMAKE_SYSTEM_PROCESSOR}"
)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Neon Survivor Maintainer")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# Detect shared-library dependencies for the Debian package.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
