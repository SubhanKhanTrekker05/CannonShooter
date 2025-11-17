#ifndef BULLET_H
#define BULLET_H
#include <SDL.h>
#include <vector>

class Enemy;

struct Bullet {
    float x, y;       
    float vx, vy;     
    bool active;     
    bool fromPlayer; 
    Bullet* next;     
    Bullet() : x(0), y(0), vx(0), vy(0), active(false),fromPlayer(false), next(nullptr) {}
};

class BulletManager {
public:
    static void init();
    static SDL_Texture* bulletTexture;
    static void initTexture(SDL_Renderer* renderer, const char* imagePath);
    static void cleanupTexture();
    static void update(float deltaTime, std::vector<Enemy*>& enemies,const SDL_Rect& playerRect);
    static void render(SDL_Renderer* renderer);
    static void shoot(float startX, float startY, float targetX, float targetY,bool fromPlayer=false);
    static void shootNearest(float startX, float startY, std::vector<Enemy*>& enemies);
    static bool playerHitThisFrame;
    static void shootDirection(float startX, float startY, float dx, float dy, bool fromPlayer);
private:
    static const int MAX_BULLETS = 100;   
    static Bullet bullets[MAX_BULLETS];   
    static int freeQueue[MAX_BULLETS];    
    static int front, rear, count;        
    static Bullet* activeHead;           
    static const int BULLET_WIDTH;       
    static const int BULLET_HEIGHT;      
    static const float BULLET_SPEED;     
    static int getFreeBulletIndex();
    static void freeBullet(int index);
};

#endif 