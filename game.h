#pragma once
#include <vector>
#include <SDL.h>
#include<SDL_ttf.h>
#include<SDL_image.h>
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "leaderboard.h"
#include "storage_hash.h"

enum class GameState {
    PLAYING,
    VICTORY_SCREEN,
    DEFEAT_SCREEN,
    SHOW_LEADERBOARD
};

class Game {
public:
    Game();
    ~Game();
    bool init(const char* title, int width, int height);
    void handleEvents();
    void update();
    void render();
    void clean();
    bool running() const;
    int getPlayerScore() const {
        return player.getScore(); 
    }
    void setPermanentScore(int score) { 
        permanentScore = score; 
    }
    void setPlayerName(const std::string& n) { 
        playerName = n; 
    }
    std::string getPlayerName() const { 
        return playerName; 
    } 
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;
    TTF_Font* font = nullptr;
    Player player;
    std::vector<Enemy> enemies;
    BulletManager bulletManager;
    int totalDeployed = 0;
    int totalDeaths = 0;
    void spawnEnemyRandomly();
    void spawnEnemies(int count);
    GameState currentState = GameState::PLAYING;
    int level = 1;
    int permanentScore = 0;
    bool sKeyIsDown = false;
    bool sKeyWasDown = false;
    SDL_Texture* enemyTexture = nullptr;
    SDL_Texture* welcomeBg = nullptr;
    SDL_Texture* yellowStarTex = nullptr;
    SDL_Texture* grayStarTex = nullptr;
    StorageHash storage;        
    Leaderboard leaderboard;   
    TTF_Font* leaderboardFont = nullptr;
    bool leaderboardSaved = false;
    GameState prevEndState = GameState::DEFEAT_SCREEN; 
    std::string playerName;     
};
