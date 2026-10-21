
#include "Game.hpp"

#include <SDL3/SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

namespace {
constexpr float PI = 3.14159265358979323846f;

float length(Vec2 v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vec2 normalize(Vec2 v) {
    float len = length(v);
    if (len < 0.0001f) return {};
    return {v.x / len, v.y / len};
}

float randomFloat(float min, float max) {
    return min + (max - min) *
        (static_cast<float>(std::rand()) / RAND_MAX);
}
}

bool Game::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(
        "NEON SURVIVOR",
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    glContext = SDL_GL_CreateContext(window);

    if (!glContext) {
        SDL_Log("OpenGL context failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
        return false;
    }

    SDL_GL_SetSwapInterval(1);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.025f, 0.035f, 0.075f, 1.0f);

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    reset();

    return true;
}

void Game::reset() {
    player = {width * 0.5f, height * 0.5f};
    aim = {player.x + 1.0f, player.y};
    bullets.clear();
    enemies.clear();

    health = 100.0f;
    shootTimer = 0.0f;
    spawnTimer = 0.0f;
    elapsed = 0.0f;
    score = 0;
    gameOver = false;
}

void Game::shutdown() {
    if (glContext) {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void Game::run() {
    Uint64 previous = SDL_GetPerformanceCounter();
    const double frequency =
        static_cast<double>(SDL_GetPerformanceFrequency());

    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN &&
                !event.key.repeat) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }

                if (gameOver &&
                    (event.key.key == SDLK_R ||
                     event.key.key == SDLK_SPACE)) {
                    reset();
                }
            }
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(
            static_cast<double>(now - previous) / frequency
        );
        previous = now;

        // Avoid huge simulation jumps after window dragging or pauses.
        dt = std::min(dt, 0.033f);

        update(dt);
        render();
        SDL_GL_SwapWindow(window);
    }
}

void Game::update(float dt) {
    if (gameOver) return;

    elapsed += dt;
    shootTimer -= dt;
    spawnTimer -= dt;

    int newWidth = width;
    int newHeight = height;
    SDL_GetWindowSize(window, &newWidth, &newHeight);

    width = std::max(newWidth, 1);
    height = std::max(newHeight, 1);

    const bool* keys = SDL_GetKeyboardState(nullptr);

    Vec2 movement{};

    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
        movement.y -= 1.0f;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
        movement.y += 1.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
        movement.x -= 1.0f;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
        movement.x += 1.0f;

    movement = normalize(movement);
    player = player + movement * (300.0f * dt);

    player.x = std::clamp(player.x, 16.0f,
                         static_cast<float>(width) - 16.0f);
    player.y = std::clamp(player.y, 16.0f,
                         static_cast<float>(height) - 16.0f);

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);

    aim = normalize(Vec2{
        mouseX - player.x,
        mouseY - player.y
    });

    if (length(aim) < 0.001f) aim = {1.0f, 0.0f};

    const SDL_MouseButtonFlags mouse =
        SDL_GetMouseState(nullptr, nullptr);

    if ((mouse & SDL_BUTTON_LMASK ||
         keys[SDL_SCANCODE_SPACE]) && shootTimer <= 0.0f) {

        Bullet bullet;
        bullet.pos = player + aim * 20.0f;
        bullet.vel = aim * 650.0f;
        bullet.life = 1.3f;

        bullets.push_back(bullet);
        shootTimer = 0.16f;
    }

    // Spawn enemies around the arena edges.
    if (spawnTimer <= 0.0f) {
        Enemy enemy;

        int side = std::rand() % 4;

        if (side == 0) {
            enemy.pos = {randomFloat(0, width), -20};
        } else if (side == 1) {
            enemy.pos = {randomFloat(0, width),
                         static_cast<float>(height) + 20};
        } else if (side == 2) {
            enemy.pos = {-20, randomFloat(0, height)};
        } else {
            enemy.pos = {static_cast<float>(width) + 20,
                         randomFloat(0, height)};
        }

        enemy.radius = randomFloat(12.0f, 19.0f);
        enemy.speed = randomFloat(65.0f, 105.0f)
                    + std::min(elapsed * 1.3f, 100.0f);

        enemies.push_back(enemy);
        spawnTimer = std::max(0.22f, 0.85f - elapsed * 0.008f);
    }

    for (auto& bullet : bullets) {
        bullet.pos = bullet.pos + bullet.vel * dt;
        bullet.life -= dt;
    }

    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(),
            [this](const Bullet& b) {
                return b.life <= 0.0f ||
                    b.pos.x < -10 || b.pos.x > width + 10 ||
                    b.pos.y < -10 || b.pos.y > height + 10;
            }),
        bullets.end()
    );

    for (auto& enemy : enemies) {
        Vec2 direction = normalize(player - enemy.pos);
        enemy.pos = enemy.pos + direction * (enemy.speed * dt);

        if (length(player - enemy.pos) <
            enemy.radius + 13.0f) {
            health -= 32.0f * dt;
        }
    }

    // Bullet-enemy collisions.
    for (auto& enemy : enemies) {
        for (auto& bullet : bullets) {
            if (bullet.life <= 0.0f) continue;

            if (length(enemy.pos - bullet.pos) <
                enemy.radius + 4.0f) {
                enemy.radius = -1.0f;
                bullet.life = 0.0f;
                score += 10;
                break;
            }
        }
    }

    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e) { return e.radius < 0.0f; }),
        enemies.end()
    );

    if (health <= 0.0f) {
        health = 0.0f;
        gameOver = true;
    }
}

void Game::drawCircle(float x, float y, float r,
                      float cr, float cg, float cb,
                      int segments) {
    glColor3f(cr, cg, cb);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (int i = 0; i <= segments; ++i) {
        float a = 2.0f * PI * i / segments;
        glVertex2f(x + std::cos(a) * r,
                   y + std::sin(a) * r);
    }

    glEnd();
}

void Game::drawRect(float x, float y, float w, float h,
                    float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Game::drawLine(float x1, float y1, float x2, float y2,
                    float r, float g, float b, float thickness) {
    glLineWidth(thickness);
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void Game::drawDigit(int digit, float x, float y, float s) {
    // Seven-segment digits, drawn as small rectangles.
    static const int segments[10][7] = {
        {1,1,1,1,1,1,0},
        {0,1,1,0,0,0,0},
        {1,1,0,1,1,0,1},
        {1,1,1,1,0,0,1},
        {0,1,1,0,0,1,1},
        {1,0,1,1,0,1,1},
        {1,0,1,1,1,1,1},
        {1,1,1,0,0,0,0},
        {1,1,1,1,1,1,1},
        {1,1,1,1,0,1,1}
    };

    static const float lines[7][4] = {
        {2,0,10,0}, {12,2,12,9}, {12,11,12,18},
        {2,20,10,20}, {0,11,0,18}, {0,2,0,9},
        {2,10,10,10}
    };

    if (digit < 0 || digit > 9) return;

    for (int i = 0; i < 7; ++i) {
        if (!segments[digit][i]) continue;

        drawLine(
            x + lines[i][0] * s,
            y + lines[i][1] * s,
            x + lines[i][2] * s,
            y + lines[i][3] * s,
            0.2f, 0.95f, 1.0f, 3.0f
        );
    }
}

void Game::drawNumber(int number, float x, float y, float scale) {
    number = std::max(0, number);

    int divisor = 1;
    while (number / divisor >= 10 && divisor <= 100000000)
        divisor *= 10;

    do {
        drawDigit((number / divisor) % 10, x, y, scale);
        x += 18.0f * scale;
        divisor /= 10;
    } while (divisor > 0);
}

void Game::drawHUD() {
    drawRect(18, 18, 204, 16, 0.15f, 0.18f, 0.25f);
    drawRect(20, 20, 200 * health / 100.0f, 12,
             health > 35 ? 0.1f : 1.0f,
             health > 35 ? 0.9f : 0.15f,
             0.35f);

    drawNumber(score, 20, 48, 1.5f);

    // Small elapsed-time indicator.
    drawRect(20, 92, std::fmod(elapsed, 60.0f) / 60.0f
             * 180.0f, 3, 0.85f, 0.25f, 1.0f);
}

void Game::render() {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT);

    // Subtle arena grid.
    for (int x = 0; x < width; x += 40)
        drawLine(x, 0, x, height, 0.055f, 0.08f, 0.14f);

    for (int y = 0; y < height; y += 40)
        drawLine(0, y, width, y, 0.055f, 0.08f, 0.14f);

    // Bullets.
    for (const auto& b : bullets) {
        drawCircle(b.pos.x, b.pos.y, 7, 0.0f, 0.65f, 1.0f);
        drawCircle(b.pos.x, b.pos.y, 3, 0.8f, 1.0f, 1.0f);
    }

    // Enemies.
    for (const auto& e : enemies) {
        drawCircle(e.pos.x, e.pos.y, e.radius + 5,
                   0.25f, 0.04f, 0.12f);
        drawCircle(e.pos.x, e.pos.y, e.radius,
                   1.0f, 0.12f, 0.32f);
        drawCircle(e.pos.x, e.pos.y, e.radius * 0.42f,
                   0.3f, 0.02f, 0.12f);
    }

    // Player: a directional triangle.
    float angle = std::atan2(aim.y, aim.x);

    Vec2 forward{std::cos(angle), std::sin(angle)};
    Vec2 side{-forward.y, forward.x};

    Vec2 tip = player + forward * 20.0f;
    Vec2 left = player - forward * 12.0f + side * 12.0f;
    Vec2 right = player - forward * 12.0f - side * 12.0f;

    drawCircle(player.x, player.y, 23, 0.0f, 0.12f, 0.25f);

    glColor3f(0.05f, 0.85f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(tip.x, tip.y);
    glVertex2f(left.x, left.y);
    glVertex2f(right.x, right.y);
    glEnd();

    drawCircle(player.x, player.y, 4, 1.0f, 1.0f, 1.0f);

    drawHUD();

    if (gameOver) {
        // Dark translucent overlay.
        glColor4f(0.01f, 0.02f, 0.06f, 0.88f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(width, 0);
        glVertex2f(width, height);
        glVertex2f(0, height);
        glEnd();

        // A geometric game-over screen, no external font needed.
        float cx = width * 0.5f;
        float cy = height * 0.5f;

        drawCircle(cx, cy - 65, 25, 1.0f, 0.08f, 0.25f);
        drawRect(cx - 3, cy - 80, 6, 30, 0.02f, 0.02f, 0.05f);
        drawRect(cx - 3, cy - 42, 6, 6, 0.02f, 0.02f, 0.05f);

        drawNumber(score, cx - 36, cy - 10, 2.0f);

        // Restart prompt represented by an illuminated bar.
        drawRect(cx - 75, cy + 45, 150, 5,
                 0.0f, 0.8f, 1.0f);
        drawRect(cx - 55, cy + 65, 110, 3,
                 0.8f, 0.2f, 1.0f);
    }
}
