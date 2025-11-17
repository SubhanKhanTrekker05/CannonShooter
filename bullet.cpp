#include "bullet.h"
#include "enemy.h"
#include<iostream>
#include<cmath>
#include<limits>
#include<SDL.h>
#include<SDL_image.h>

bool BulletManager::playerHitThisFrame = false;
Bullet BulletManager::bullets[BulletManager::MAX_BULLETS];
int BulletManager::freeQueue[BulletManager::MAX_BULLETS];
int BulletManager::front = 0;
int BulletManager::rear = -1;
int BulletManager::count = 0;
Bullet* BulletManager::activeHead = nullptr;
SDL_Texture* BulletManager::bulletTexture = nullptr;  

const int BulletManager::BULLET_WIDTH  = 13;
const int BulletManager::BULLET_HEIGHT = 13;
const float BulletManager::BULLET_SPEED =30.0f;

void BulletManager::init() {
    activeHead = nullptr;
    front = 0;
    rear = -1;
    count = 0;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        freeBullet(i);
    }
}
void BulletManager::initTexture(SDL_Renderer* renderer, const char* imagePath) {
    if (!renderer || bulletTexture) return; 
    SDL_Surface* surf = IMG_Load(imagePath);
    if (!surf) {
        std::cout << "IMG_Load Error (bullet): " << IMG_GetError() << " for " << imagePath << std::endl;
        return;
    }
    SDL_Surface* surf32 = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surf);
    if (!surf32) {
        std::cout << "Surface convert failed (bullet): " << SDL_GetError() << std::endl;
        return;
    }
    bulletTexture = SDL_CreateTextureFromSurface(renderer, surf32);
    SDL_FreeSurface(surf32);
    if (!bulletTexture) {
        std::cout << "CreateTextureFromSurface failed (bullet): " << SDL_GetError() << std::endl;
    }
}
void BulletManager::cleanupTexture() {
    if (bulletTexture) {
        SDL_DestroyTexture(bulletTexture);
        bulletTexture = nullptr;
    }
}
int BulletManager::getFreeBulletIndex() {
    if (count == 0) return -1;
    int idx = freeQueue[front];
    front = (front + 1) % MAX_BULLETS;
    --count;
    return idx;
}
void BulletManager::freeBullet(int index) {
    bullets[index].active = false;
    bullets[index].next = nullptr;
    rear = (rear + 1) % MAX_BULLETS;
    freeQueue[rear] = index;
    ++count;
}
void BulletManager::shoot(float startX, float startY, float targetX, float targetY , bool fromPlayer) {
    int idx = getFreeBulletIndex();
    if (idx < 0) return;

    Bullet& b = bullets[idx];
    b.x = startX;
    b.y = startY;
    float dx = targetX - startX;
    float dy = targetY - startY;
    float dist = std::sqrt(dx*dx + dy*dy);
    if (dist > 0.0f) {
        b.vx = (dx / dist) * BULLET_SPEED;
        b.vy = (dy / dist) * BULLET_SPEED;
    } else {
        b.vx = 0.0f;
        b.vy = 0.0f;
    }

    b.active = true;
    b.fromPlayer=fromPlayer;
    b.next = activeHead;
    activeHead = &b;
}
void BulletManager::shootNearest(float startX, float startY, std::vector<Enemy*>& enemies) {
    Enemy* nearest = nullptr;
    float bestDistSq = std::numeric_limits<float>::max();

    for (Enemy* e : enemies) {
        if (!e->isAlive()) continue;

        float dx = e->getX() - startX;
        float dy = e->getY() - startY;
        float distSq = dx * dx + dy * dy;

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            nearest = e;
        }
    }

    if (nearest) {
        shoot(startX, startY, nearest->getX(), nearest->getY(),true);
    }
}
void BulletManager::update(float deltaTime, std::vector<Enemy*>& enemies, const SDL_Rect& playerRect) {
    playerHitThisFrame = false; 
    Bullet* b = activeHead;
    Bullet* prev = nullptr;

    while (b != nullptr) {
        b->x += b->vx * deltaTime;
        b->y += b->vy * deltaTime;
        
        if (b->x < -BULLET_WIDTH || b->x > 900 || b->y < -BULLET_HEIGHT || b->y > 700) {
            Bullet* toRemove = b;
            if (prev) prev->next = b->next;
            else activeHead = b->next;

            b = b->next;
            int index = static_cast<int>(toRemove - bullets);
            freeBullet(index);
            continue;
        }

        bool hit = false;

        if (b->fromPlayer) {
            for (Enemy* e : enemies) {
                if (!e->isAlive()) continue;

                SDL_Rect bulletRect = {static_cast<int>(b->x),static_cast<int>(b->y),BULLET_WIDTH,BULLET_HEIGHT};
                SDL_Rect enemyRect = {e->getX(),e->getY(),70,70};
                if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
                    e->hit();
                    hit = true;
                    break;
                }
            }
        }
        else {  
            SDL_Rect bulletRect = {
                static_cast<int>(b->x),
                static_cast<int>(b->y),
                BULLET_WIDTH,
                BULLET_HEIGHT
            };

            if (SDL_HasIntersection(&bulletRect, &playerRect)) {
                playerHitThisFrame = true;  
                hit = true;
            }
        }

        if (hit) {
            Bullet* toRemove = b;
            if (prev) prev->next = b->next;
            else activeHead = b->next;

            b = b->next;
            int index = static_cast<int>(toRemove - bullets);
            freeBullet(index);
            continue;
        } else {
            prev = b;
            b = b->next;
        }
    }
}

void BulletManager::render(SDL_Renderer* renderer) {
    if (!renderer) return;  

    Bullet* b = activeHead;
    while (b != nullptr) {
        SDL_Rect rect = {
            static_cast<int>(b->x),
            static_cast<int>(b->y),
            BULLET_WIDTH,
            BULLET_HEIGHT
        };

        if (bulletTexture) {
            SDL_RenderCopy(renderer, bulletTexture, nullptr, &rect);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &rect);
        }

        b = b->next;
    }
}
void BulletManager::shootDirection(float startX, float startY, float dx, float dy, bool fromPlayer) {
    int idx = getFreeBulletIndex();
    if (idx < 0) return;

    Bullet& b = bullets[idx];
    b.x = startX;
    b.y = startY;
    b.vx = dx * BULLET_SPEED;   
    b.vy = dy * BULLET_SPEED;
    b.active = true;
    b.fromPlayer = fromPlayer;
    b.next = activeHead;
    activeHead = &b;
}