
#pragma once

#include <SDL3/SDL.h>
#include <vector>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& v) const {
        return {x + v.x, y + v.y};
    }

    Vec2 operator-(const Vec2& v) const {
        return {x - v.x, y - v.y};
    }

    Vec2 operator*(float s) const {
        return {x * s, y * s};
    }
};

struct Bullet {
    Vec2 pos;
    Vec2 vel;
    float life = 1.5f;
};

struct Enemy {
    Vec2 pos;
    float radius = 15.0f;
    float speed = 80.0f;
};

class Game {
public:
    bool init();
    void run();
    void shutdown();

private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;

    int width = 1280;
    int height = 720;

    Vec2 player;
    Vec2 aim;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    float health = 100.0f;
    float shootTimer = 0.0f;
    float spawnTimer = 0.0f;
    float elapsed = 0.0f;

    int score = 0;
    bool running = true;
    bool gameOver = false;

    void reset();
    void update(float dt);
    void render();

    void drawCircle(float x, float y, float r,
                    float cr, float cg, float cb,
                    int segments = 24);

    void drawRect(float x, float y, float w, float h,
                  float r, float g, float b);

    void drawLine(float x1, float y1, float x2, float y2,
                  float r, float g, float b, float thickness = 2.0f);

    void drawDigit(int digit, float x, float y, float scale);
    void drawNumber(int number, float x, float y, float scale);
    void drawHUD();
};
