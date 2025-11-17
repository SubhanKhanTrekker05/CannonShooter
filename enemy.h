#pragma once
#include <SDL.h>
#include <SDL_image.h>

class BulletManager;

class Enemy {
    SDL_Rect rect;
    int health;
    bool alive;
    bool justKilled;
    static const int MAX_HEALTH = 3;
    float timeSinceLastShot = 0.0f;
    float shootInterval = 2.0f;
    float angle = 0.0f;           
    SDL_Texture* enemyTexture = nullptr;  

public:
    Enemy();
    void init(int x, int y, int w, int h);
    void init(int x, int y, int w, int h, SDL_Renderer* renderer, SDL_Texture* texture);
    void update();
    void render(SDL_Renderer* renderer);
    void hit();
    bool isAlive() const;
    int getX() const;
    int getY() const;
    int getHealth() const;
    bool wasJustKilled() const;
    void clearJustKilled();
    bool shouldShoot(float dt);
    void shoot(BulletManager& bm, float playerX);
    void clean();
};