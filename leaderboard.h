#pragma once
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include "storage_hash.h"

class Leaderboard {
public:
    Leaderboard();
    ~Leaderboard();
    bool loadFromFile(const std::string& path);
    void setCurrentPlayer(const std::string& name);
    void render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int maxEntries = 20);
private:
    std::vector<Profile> entries;
    std::vector<int> ranks;       
    std::string currentPlayer;    
    std::string currentPlayerKey; 
};
