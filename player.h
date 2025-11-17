#pragma once
#include <SDL.h>
#include <SDL_image.h>  
class Player {
public:
    Player();
    Player(int x, int y, int w, int h);
    void initTexture(SDL_Renderer* renderer, const char* imagePath);
    void clean(); 
    void handleInput(const Uint8* keystate);
    void update();
    void render(SDL_Renderer* renderer);
    const SDL_Rect& getRect() const { return rect; }
    int getHealth();
    int getScore() const;
    void addScore(int points);
    void takeDamage();
    float getAngle() const { return angle; }
    void setAngle(float a) { angle = a; }
private:
    SDL_Rect rect;
    int speed;
    int health;
    static const int PLAYER_MAX_HEALTH = 3;
    int score;
    SDL_Texture* texture = nullptr;
    float angle = 0.0f; 
};