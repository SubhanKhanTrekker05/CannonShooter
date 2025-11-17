#include "game.h"
#include<iostream>
#include<ctime>
#include <cmath>
#include "storage_hash.h"
#include "leaderboard.h"
#include<SDL_ttf.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Game::Game(){}
Game::~Game() {}

bool Game::init(const char* title, int width, int height) {
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return false;
        }
    }
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0) {
            std::cout<<"TTF_Init Error: "<<TTF_GetError()<<"\n";
        }
    }

    srand(static_cast<unsigned int>(time(0)));
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if (!window) { std::cout << "Window Error: " << SDL_GetError() << std::endl; return false; }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { std::cout << "Renderer Error: " << SDL_GetError() << std::endl; return false; }

    font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        std::cout << "Score font load failed: " << TTF_GetError() << std::endl;
    }
    leaderboardFont = TTF_OpenFont("arial.ttf", 28);
    if (!leaderboardFont) {
        std::cout << "Leaderboard font load failed: " << TTF_GetError() << std::endl;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cout << "IMG_Init Warning: " << IMG_GetError() << std::endl;
    }
    player = Player(400, 610, 90, 90);  
    storage.loadFromFile("profiles.dat");

    player.initTexture(renderer, "assets/player_cannon.png");
    SDL_Surface* surf = IMG_Load("assets/enemy_cannon.png");
    if (!surf) {
        std::cout << "IMG_Load Warning (enemy): " << IMG_GetError() << std::endl;
    } else {
        enemyTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    enemies.clear();
    enemies.emplace_back(); enemies.back().init(100, 550, 70, 70, renderer, enemyTexture);
    enemies.emplace_back(); enemies.back().init(600, 250, 70, 70, renderer, enemyTexture);
    enemies.emplace_back(); enemies.back().init(700, 350, 70, 70, renderer, enemyTexture);
    totalDeployed = 3;
    totalDeaths = 0;
    SDL_Surface* wbSurf = IMG_Load("assets/welcome_bg.png");
    if (!wbSurf) {
        std::cout << "Welcome bg load warning: " << IMG_GetError() << std::endl;
    } else {
        welcomeBg = SDL_CreateTextureFromSurface(renderer, wbSurf);
        SDL_FreeSurface(wbSurf);
        if (!welcomeBg) {
            std::cout << "CreateTexture (welcomeBg) failed: " << SDL_GetError() << std::endl;
        }
        if (welcomeBg) {
            std::cout << "[DEBUG] welcomeBg loaded successfully.\n";
        } else {
        std::cout << "[DEBUG] welcomeBg failed: " << SDL_GetError() << "\n";
        }
    }
    SDL_Surface* ySurf = IMG_Load("assets/yellow_star.png");
    if (ySurf) {
        yellowStarTex = SDL_CreateTextureFromSurface(renderer, ySurf);
        SDL_FreeSurface(ySurf);
    }
    SDL_Surface* gSurf = IMG_Load("assets/grey_star.png");
    if (gSurf) {
        grayStarTex = SDL_CreateTextureFromSurface(renderer, gSurf);
        SDL_FreeSurface(gSurf);
    }
    bulletManager.init();
    bulletManager.initTexture(renderer, "assets/player_bullet.png");
    
    isRunning = true;
    std::cout<<"[GAME] init done\n";
    return true;
}
void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int mx = event.button.x;
            int my = event.button.y;
            std::cout << "[INPUT] Mouse click at: " << mx << "," << my << std::endl;

            if (currentState == GameState::VICTORY_SCREEN || currentState == GameState::DEFEAT_SCREEN) {
                SDL_Rect leaderboardBtn = {350, 480, 200, 60};
                if (mx >= leaderboardBtn.x && mx <= leaderboardBtn.x + leaderboardBtn.w &&
                    my >= leaderboardBtn.y && my <= leaderboardBtn.y + leaderboardBtn.h) {
                    prevEndState = currentState;  
                    currentState = GameState::SHOW_LEADERBOARD;
                    leaderboard.setCurrentPlayer(playerName); 
                    storage.loadFromFile("profiles.dat");  
                    leaderboard.loadFromFile("profiles.dat");  
                    std::cout << "[STATE] Changing to SHOW_LEADERBOARD\n";
                    continue; // go to next event
                }
            }

            if (currentState == GameState::SHOW_LEADERBOARD) {
                SDL_Rect backBtn = {350, 620, 200, 50}; 
                if (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                    my >= backBtn.y && my <= backBtn.y + backBtn.h) {
                    currentState = prevEndState;
                    continue;
                }
            }
        } 
        if ((currentState == GameState::VICTORY_SCREEN || currentState == GameState::DEFEAT_SCREEN) && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
            player = Player(400, 610, 90, 90);
            player.initTexture(renderer, "assets/player_cannon.png");
            enemies.clear();
            totalDeployed = 0;
            totalDeaths = 0;
            currentState = GameState::PLAYING;
            leaderboardSaved = false;   
            spawnEnemies(3);
        }
        else if (currentState == GameState::PLAYING) {
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                sKeyIsDown = true;
            }
            if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s) {
                sKeyIsDown = false;
            }
        }
    } 
}
void Game::update() {
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    player.handleInput(keystate);
    player.update();
    SDL_Rect prect = player.getRect();
    float playerCenterX = prect.x + prect.w/2.0f;
    float playerCenterY = prect.y;
    std::vector<Enemy*> enemyPtrs;
    enemyPtrs.reserve(enemies.size());
    for (auto &e:enemies) enemyPtrs.push_back(&e);

    if (sKeyIsDown && !sKeyWasDown) {
        Enemy* nearest = nullptr;
        float bestDistSq = 1e9f;
        for (Enemy* e : enemyPtrs) {
            if (!e->isAlive()) continue;

            float ecx = e->getX() + 35.0f;
            float ecy = e->getY() + 35.0f;
            float dx = ecx - playerCenterX;
            float dy = ecy - playerCenterY;
            float d2 = dx * dx + dy * dy;
            if (d2 < bestDistSq) {
                bestDistSq = d2;
                nearest = e;
            }
        }

        if (nearest) {
            float targetX = nearest->getX() + 35.0f;
            float targetY = nearest->getY() + 35.0f;
            float dx = targetX - playerCenterX;
            float dy = targetY - playerCenterY;
            float len = sqrtf(dx * dx + dy * dy);
            if (len == 0.0f) len = 1.0f;

            float dirX = dx / len;
            float dirY = dy / len;

            float muzzleOffset = prect.h / 2.0f;
            if (muzzleOffset < 20.0f) muzzleOffset = 40.0f;

            float startX = playerCenterX + dirX * muzzleOffset;
            float startY = playerCenterY + dirY * muzzleOffset;
            float perpX = -dirY;
            float perpY = dirX;
            startX -= perpX * 5.0f;
            startY -= perpY * 5.0f;
            player.setAngle(atan2f(dy, dx) * (180.0f / M_PI) + 90.0f);
            bulletManager.shootDirection(startX, startY, dirX, dirY, true);
        }
    } 

    if (!sKeyIsDown) {
        float current = player.getAngle();
        float target = 0.0f;  
        float rotationSpeed = 0.5f;

        float diff = target - current;
        if (diff > 180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;

        if (fabs(diff) < rotationSpeed)
            player.setAngle(target);
        else
            player.setAngle(current + rotationSpeed * (diff > 0 ? 1 : -1));
    }

    sKeyWasDown = sKeyIsDown;
    float dt = 1.0f/60.0f;
    bulletManager.update(dt,enemyPtrs,player.getRect());
    if (BulletManager::playerHitThisFrame) player.takeDamage();
    int newlyKilled = 0;
    for (auto &enemy:enemies) {
        if (enemy.wasJustKilled()) { 
            newlyKilled++; 
            enemy.clearJustKilled(); 
        }
    }
    if (newlyKilled>0) player.addScore(10*newlyKilled);

    totalDeaths += newlyKilled;
    if (totalDeaths == 1 && totalDeployed < 5) spawnEnemies(2);
    else if (totalDeaths == 2 && totalDeployed < 7) spawnEnemies(2);
    else if (totalDeaths == 4 && totalDeployed < 10) spawnEnemies(3);

    for (auto &enemy:enemies) {
        if (enemy.isAlive() && enemy.shouldShoot(dt)) {
            enemy.shoot(bulletManager, playerCenterX);
        }
    }
    if (currentState == GameState::PLAYING) {
        if (player.getHealth() <= 0) {
            currentState = GameState::DEFEAT_SCREEN;
        } else if (totalDeaths >= 10) {
            currentState = GameState::VICTORY_SCREEN;
        }
    }
    if ((currentState == GameState::DEFEAT_SCREEN || currentState == GameState::VICTORY_SCREEN) && !leaderboardSaved) {
        std::string user = playerName.empty() ? "Player" : playerName;
        Profile p;
        p.name = user;
        p.score = player.getScore();
        storage.insertOrUpdate(p);            
        storage.saveToFile("profiles.dat");   
        leaderboardSaved = true;              
    }
}
void Game::render() {
    if (!renderer) return;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    player.render(renderer);
    for (auto &e : enemies) e.render(renderer);
    bulletManager.render(renderer);

    if (font) {
        std::string scoreText = "Score: " + std::to_string(player.getScore());
        if (!scoreText.empty()) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), white);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect dst = {700, 20, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, nullptr, &dst);
                    SDL_DestroyTexture(texture);
                } else {
                    std::cout << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
                }
                SDL_FreeSurface(surface);
            } else {
                std::cout << "TTF_RenderText_Solid failed: " << TTF_GetError() << "\n";
            }
        }
    }

    if (font) {
        std::string levelText = "Level: " + std::to_string(level);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* sLevel = TTF_RenderText_Solid(font, levelText.c_str(), white);
        if (sLevel) {
            SDL_Texture* tLevel = SDL_CreateTextureFromSurface(renderer, sLevel);
            SDL_Rect rLevel = {(900 - sLevel->w) / 2,20,sLevel->w,sLevel->h};
            SDL_RenderCopy(renderer, tLevel, nullptr, &rLevel);
            SDL_DestroyTexture(tLevel);
            SDL_FreeSurface(sLevel);
        }
    }

    if (currentState == GameState::VICTORY_SCREEN || currentState == GameState::DEFEAT_SCREEN) {
        if (welcomeBg) {
            SDL_Rect r = {0, 0, 900, 700};
            SDL_RenderCopy(renderer, welcomeBg, nullptr, &r);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }

        std::string title = (currentState == GameState::VICTORY_SCREEN) ? "You Won!" : "You Lost!";
        SDL_Color color = {255, 255, 255, 255};

        if (font) {
            SDL_Surface* sTitle = TTF_RenderText_Solid(font, title.c_str(), color);
            if (sTitle) {
                SDL_Texture* tTitle = SDL_CreateTextureFromSurface(renderer, sTitle);
                SDL_Rect rTitle = {(900 - sTitle->w) / 2,200,sTitle->w,sTitle->h};
                SDL_RenderCopy(renderer, tTitle, nullptr, &rTitle);
                SDL_DestroyTexture(tTitle);
                SDL_FreeSurface(sTitle);
            }
        }
        std::string subtext = (currentState == GameState::VICTORY_SCREEN) ? "Press SPACE to restart" : "Press SPACE to try again";
        if (font) {
            SDL_Surface* sSub = TTF_RenderText_Solid(font, subtext.c_str(), color);
            if (sSub) {
                SDL_Texture* tSub = SDL_CreateTextureFromSurface(renderer, sSub);
                SDL_Rect rSub = {(900 - sSub->w) / 2,280,sSub->w,sSub->h};
                SDL_RenderCopy(renderer, tSub, nullptr, &rSub);
                SDL_DestroyTexture(tSub);
                SDL_FreeSurface(sSub);
            }
        }

        float killRatio = totalDeaths / 10.0f;  
        int starsEarned = 0;
        if (killRatio > 0.3f) starsEarned = 1;
        if (killRatio > 0.4f) starsEarned = 2;
        if (killRatio > 0.9f) starsEarned = 3;

        int starY = 360;
        int starSize = 30;
        for (int i = 0; i < 3; ++i) {
            SDL_Texture* starTex = (i < starsEarned) ? yellowStarTex : grayStarTex;
            if (starTex) {
                SDL_Rect starRect = {
                    400 + i * 50 - starSize/2,
                    starY,
                    starSize,
                    starSize
                };
                SDL_RenderCopy(renderer, starTex, nullptr, &starRect);
            } else {
                SDL_Color color = (i < starsEarned) 
                    ? SDL_Color{255, 255, 0, 255} 
                    : SDL_Color{100, 100, 100, 255};
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_Rect r = {400 + i * 50 - starSize/2, starY, starSize, starSize};
                SDL_RenderFillRect(renderer, &r);
            }        
        }
        SDL_Rect leaderboardBtn = { 350, 480, 200, 60 }; // position and size
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(renderer, 40, 40, 120, 220);
        SDL_RenderFillRect(renderer, &leaderboardBtn);

        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 200);
        SDL_RenderDrawRect(renderer, &leaderboardBtn);

        SDL_Color white = {255,255,255,255};
        TTF_Font* btnFont = leaderboardFont ? leaderboardFont : font;
        if (btnFont) {
            SDL_Surface* sBtn = TTF_RenderText_Solid(btnFont, "LEADERBOARD", white);
            if (sBtn) {
                SDL_Texture* tBtn = SDL_CreateTextureFromSurface(renderer, sBtn);
                if (tBtn) {
                    SDL_Rect rBtn = {
                        leaderboardBtn.x + (leaderboardBtn.w - sBtn->w) / 2,
                        leaderboardBtn.y + (leaderboardBtn.h - sBtn->h) / 2,
                        sBtn->w, sBtn->h
                    };
                    SDL_RenderCopy(renderer, tBtn, nullptr, &rBtn);
                    SDL_DestroyTexture(tBtn);
                }
                SDL_FreeSurface(sBtn);
            }
        }

    }
    if (currentState == GameState::SHOW_LEADERBOARD) {
        if (welcomeBg) {
            SDL_Rect full = {0, 0, 900, 700};
            SDL_RenderCopy(renderer, welcomeBg, nullptr, &full);
        } else {
            SDL_SetRenderDrawColor(renderer, 12, 12, 12, 255);
            SDL_RenderClear(renderer);
        }
        SDL_Rect frame = {50, 70, 800, 540};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160); 
        SDL_RenderFillRect(renderer, &frame);
        TTF_Font* useFont = leaderboardFont ? leaderboardFont : font;
        if (useFont) {
            SDL_Color titleCol = {255, 215, 0, 255};
            SDL_Surface* sTitle = TTF_RenderText_Solid(useFont, "LEADERBOARD - Top 10", titleCol);
            if (sTitle) {
                SDL_Texture* tTitle = SDL_CreateTextureFromSurface(renderer, sTitle);
                if (tTitle) {
                    SDL_Rect rTitle = { (900 - sTitle->w) / 2, 90, sTitle->w, sTitle->h };
                    SDL_RenderCopy(renderer, tTitle, nullptr, &rTitle);
                    SDL_DestroyTexture(tTitle);
                }
                SDL_FreeSurface(sTitle);
            }
        }
        int listX = 120;
        int listY = 160;
        int maxEntries = 10;
        leaderboard.render(renderer, useFont, listX, listY, maxEntries);
        SDL_Rect backBtn = { (900 - 200)/2, 600, 200, 50 };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 60, 60, 140, 220);
        SDL_RenderFillRect(renderer, &backBtn);
        if (useFont) {
            SDL_Color white = {255,255,255,255};
            SDL_Surface* sBack = TTF_RenderText_Solid(useFont, "BACK", white);
            if (sBack) {
                SDL_Texture* tBack = SDL_CreateTextureFromSurface(renderer, sBack);
                if (tBack) {
                    SDL_Rect rBack = { backBtn.x + (backBtn.w - sBack->w)/2, backBtn.y + (backBtn.h - sBack->h)/2, sBack->w, sBack->h };
                    SDL_RenderCopy(renderer, tBack, nullptr, &rBack);
                    SDL_DestroyTexture(tBack);
                }
                SDL_FreeSurface(sBack);
            }
        }
        SDL_RenderPresent(renderer);
        return;
    }
    SDL_RenderPresent(renderer);
}
void Game::clean() {
    if (font) { TTF_CloseFont(font); font = nullptr; }
    player.clean();
    if (enemyTexture) {
        SDL_DestroyTexture(enemyTexture);
        enemyTexture = nullptr;
    }
    bulletManager.cleanupTexture();
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    std::cout<<"[GAME] cleaned\n";
    IMG_Quit();
    if (welcomeBg) {
        SDL_DestroyTexture(welcomeBg);
        welcomeBg = nullptr;
    }
    if (yellowStarTex) {
        SDL_DestroyTexture(yellowStarTex);
        yellowStarTex = nullptr;
    }
    if (grayStarTex) {
        SDL_DestroyTexture(grayStarTex);
        grayStarTex = nullptr;
    }
    if (leaderboardFont) { TTF_CloseFont(leaderboardFont); leaderboardFont = nullptr; }

}
void Game::spawnEnemyRandomly() {
    int x = 50 + rand() % 800;
    int y = 50 + rand() % 250;
    enemies.emplace_back();
    enemies.back().init(x, y, 70, 70, renderer, enemyTexture); 
}
void Game::spawnEnemies(int count) {
    for (int i=0;i<count && totalDeployed<10;++i) {
        spawnEnemyRandomly();
        totalDeployed++;
    }
}
bool Game::running() const { return isRunning; }