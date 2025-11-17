#include "enemy.h"
#include "bullet.h"
#include <iostream>
#include <SDL_image.h>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Enemy::Enemy(): health(MAX_HEALTH), alive(false), justKilled(false),enemyTexture(nullptr), angle(0.0f) {}

void Enemy::init(int x, int y, int w, int h) {
    rect = {x, y, w, h};
    health = MAX_HEALTH;
    alive = true;
    justKilled = false;
    float minInterval = 3.0f;
    float maxInterval = 6.0f;
    shootInterval = minInterval + (static_cast<float>(rand()) / RAND_MAX) * (maxInterval - minInterval);
    timeSinceLastShot = static_cast<float>(rand()) / RAND_MAX * shootInterval;
}
void Enemy::init(int x, int y, int w, int h, SDL_Renderer* renderer, SDL_Texture* texture) {
    init(x, y, w, h);  
    enemyTexture = texture; // DO NOT TOUCH THIS LINE###########  
}
void Enemy::update() {}
void Enemy::hit() {
    if (!alive) return;
    health--;
    if (health <= 0) {
        alive = false;
        justKilled = true;
    }
}
bool Enemy::wasJustKilled() const{ 
    return justKilled; 
}
bool Enemy::isAlive() const{ 
    return alive; 
}
int Enemy::getX() const{ 
    return rect.x;
}
int Enemy::getY() const{ 
    return rect.y; 
}
int Enemy::getHealth() const{ 
    return health; 
}
void Enemy::render(SDL_Renderer* renderer) {
    if (!alive || !renderer) return;

    if (enemyTexture) {
        SDL_RenderCopyEx(renderer, enemyTexture, nullptr, &rect, angle, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    int barHeight = 6;
    int barOffset = 10;
    SDL_Rect healthbar = {rect.x, rect.y - barOffset, rect.w, barHeight};

    if (health == 3) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else if (health == 2) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    SDL_RenderFillRect(renderer, &healthbar);

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &healthbar);
}
bool Enemy::shouldShoot(float dt) {
    timeSinceLastShot += dt;
    if (timeSinceLastShot >= shootInterval) {
        timeSinceLastShot = 0.0f;
        return true;
    }
    return false;
}

void Enemy::shoot(BulletManager& bm, float playerX) {
    float startCenterX = rect.x + rect.w / 2.0f;
    float startCenterY = rect.y + rect.h / 2.0f;

    float targetX = playerX;
    float targetY = 700.0f;

    float dx = targetX - startCenterX;
    float dy = targetY - startCenterY;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len == 0.0f) len = 1.0f;
    float dirX = dx / len;
    float dirY = dy / len;

    float muzzleOffset = rect.h / 2.0f;
    if (muzzleOffset < 20.0f) muzzleOffset = 40.0f;

    float startX = startCenterX + dirX * muzzleOffset;
    float startY = startCenterY + dirY * muzzleOffset;

    float perpX = -dirY;
    float perpY = dirX;
    const float sideOffset = -3.0f;
    startX += perpX * sideOffset;
    startY += perpY * sideOffset;
    angle = std::atan2(dy, dx) * (180.0f / M_PI) + 90.0f;

    bm.shoot(startX, startY, targetX, targetY);
}
void Enemy::clearJustKilled() {
    justKilled = false;
}
void Enemy::clean() {}