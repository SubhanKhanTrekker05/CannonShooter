#include "player.h"
#include <SDL_image.h>
#include <iostream>
#include <cmath>

Player::Player(): speed(2), health(PLAYER_MAX_HEALTH), score(0), texture(nullptr), angle(0.0f) {
    rect = {0, 0, 50, 50};
}
Player::Player(int x, int y, int w, int h): speed(2), health(PLAYER_MAX_HEALTH), score(0), texture(nullptr), angle(0.0f) { 
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
}
void Player::initTexture(SDL_Renderer* renderer, const char* imagePath) {
    SDL_Surface* loaded = IMG_Load(imagePath);
    if (!loaded) {
        std::cout << "IMG_Load Error (player): " << IMG_GetError() << std::endl;
        return;
    }

    SDL_Surface* surf = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);

    if (!surf) {
        std::cout << "Surface conversion failed: " << SDL_GetError() << std::endl;
        return;
    }

    Uint32* pixels = static_cast<Uint32*>(surf->pixels);
    int total = surf->w * surf->h;
    Uint32 white = SDL_MapRGBA(surf->format, 255, 255, 255, 255);
    Uint32 black = SDL_MapRGBA(surf->format, 0, 0, 0, 255);

    for (int i = 0; i < total; ++i) {
        if (pixels[i] == white)
            pixels[i] = black;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    if (!texture) {
        std::cout << "Texture creation failed (player): " << SDL_GetError() << std::endl;
    }
}
void Player::handleInput(const Uint8* keystate) {
    if (keystate[SDL_SCANCODE_LEFT]) {
        rect.x -= speed;
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
        rect.x += speed;
    }
}
void Player::update() {
    if (rect.x < 0) rect.x = 0;
    if (rect.x + rect.w > 900) rect.x = 900 - rect.w;

    while (angle >= 360.0f) angle -= 360.0f;
    while (angle < 0.0f) angle += 360.0f;
}
void Player::render(SDL_Renderer* renderer) {
    if (health <= 0) return;

    if (texture) {
        SDL_RenderCopyEx(renderer, texture, nullptr, &rect, angle, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    int barHeight = 20;
    SDL_Rect playerHealth = {50, 10, 200, barHeight};
    if (health == 3) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
    } else if (health == 2) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); 
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
    }
    SDL_RenderFillRect(renderer, &playerHealth);
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &playerHealth);
}
void Player::clean() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
}
int Player::getHealth() { 
    return health; 
}
int Player::getScore() const {
    return score;
}
void Player::addScore(int points) {
    score += points;
}
void Player::takeDamage() {
    if (health <= 0) return;
    if (health > 0) {
        health--;
        score -= 5;
        if (score < 0) score = 0;
    }
}