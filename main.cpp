#define SDL_MAIN_HANDLED
#include "game.h"
#include "login.h"
#include "storage_hash.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Cannon Shooter Login",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,900, 700, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    StorageHash storage;
    storage.loadFromFile("profiles.dat");

    Login login;
    login.init(renderer, 900, 700);

    LoginResult loginResult = LoginResult::NONE;
    std::string playerName;
    SDL_StartTextInput();
    while (loginResult == LoginResult::NONE) {
        loginResult = login.updateAndRender();
        SDL_Delay(16); 
    }
    SDL_StopTextInput();

    if (loginResult == LoginResult::EXIT) {
        std::cout << "User chose to exit. Goodbye!\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    playerName = login.getPlayerName();
    if (playerName.empty()) {
        std::cout << "No name entered. Exiting...\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
    std::cout << "Player logged in: " << playerName << std::endl;

    auto profileOpt = storage.find(playerName);
    if (!profileOpt.has_value()) {
        Profile p;
        p.name = playerName;
        p.score = 0;
        storage.insertOrUpdate(p);
    }
    int score = storage.getScore(playerName);
    int rank = storage.getRank(playerName);

    TTF_Font* welcomeFont = TTF_OpenFont("arial.ttf", 28);
    SDL_Texture* welcomeBg = nullptr;
    SDL_Surface* bgSurf = IMG_Load("assets/welcome_bg.png");
    if (!bgSurf) {
        std::cout << "Welcome background load warning: " << IMG_GetError() << std::endl;
    } else {
        welcomeBg = SDL_CreateTextureFromSurface(renderer, bgSurf);
        SDL_FreeSurface(bgSurf);
    }

    if (!welcomeFont) {
        std::cout << "Welcome font load error: " << TTF_GetError() << std::endl;
    } else {
        Uint32 startMs = SDL_GetTicks();
        bool welcomeDone = false;
        while (!welcomeDone) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_QUIT) {
                    TTF_CloseFont(welcomeFont);
                    if (welcomeBg) SDL_DestroyTexture(welcomeBg);  
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                }
                if (ev.type == SDL_KEYDOWN) {
                    welcomeDone = true; 
                }
            }
            if (SDL_GetTicks() - startMs > 2000) welcomeDone = true;

            if (welcomeBg) {
                SDL_Rect bgRect = {0, 0, 900, 700};
                SDL_RenderCopy(renderer, welcomeBg, nullptr, &bgRect);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
            }

            SDL_Color white = {255,255,255,255};
            std::string title = "Welcome to Cannon Shooter!";
            SDL_Surface* sTitle = TTF_RenderText_Solid(welcomeFont, title.c_str(), white);
            SDL_Texture* tTitle = SDL_CreateTextureFromSurface(renderer, sTitle);
            SDL_Rect rTitle = { 100, 80, sTitle->w, sTitle->h };
            SDL_RenderCopy(renderer, tTitle, NULL, &rTitle);
            SDL_FreeSurface(sTitle);
            SDL_DestroyTexture(tTitle);

            std::string line1 = "Welcome, " + playerName;
            std::string line2 = "Your Current Score = " + std::to_string(score);
            std::string line3 = "Your Rank in Leaderboard = " + std::to_string(rank);

            SDL_Surface* st1 = TTF_RenderText_Solid(welcomeFont, line1.c_str(), white);
            SDL_Texture* tt1 = SDL_CreateTextureFromSurface(renderer, st1);
            SDL_Rect r1 = { 120, 160, st1->w, st1->h };
            SDL_RenderCopy(renderer, tt1, NULL, &r1);
            SDL_FreeSurface(st1);
            SDL_DestroyTexture(tt1);

            SDL_Surface* st2 = TTF_RenderText_Solid(welcomeFont, line2.c_str(), white);
            SDL_Texture* tt2 = SDL_CreateTextureFromSurface(renderer, st2);
            SDL_Rect r2 = { 120, 200, st2->w, st2->h };
            SDL_RenderCopy(renderer, tt2, NULL, &r2);
            SDL_FreeSurface(st2);
            SDL_DestroyTexture(tt2);

            SDL_Surface* st3 = TTF_RenderText_Solid(welcomeFont, line3.c_str(), white);
            SDL_Texture* tt3 = SDL_CreateTextureFromSurface(renderer, st3);
            SDL_Rect r3 = { 120, 240, st3->w, st3->h };
            SDL_RenderCopy(renderer, tt3, NULL, &r3);
            SDL_FreeSurface(st3);
            SDL_DestroyTexture(tt3);

            TTF_Font* hintFont = TTF_OpenFont("arial.ttf", 18);
            if (hintFont) {
                std::string hint = "Press any key to start (or wait 2s)";
                SDL_Surface* sh = TTF_RenderText_Solid(hintFont, hint.c_str(), white);
                SDL_Texture* th = SDL_CreateTextureFromSurface(renderer, sh);
                SDL_Rect rh = { 120, 320, sh->w, sh->h };
                SDL_RenderCopy(renderer, th, NULL, &rh);
                SDL_FreeSurface(sh);
                SDL_DestroyTexture(th);
                TTF_CloseFont(hintFont);
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        } 

        TTF_CloseFont(welcomeFont);
        if (welcomeBg) SDL_DestroyTexture(welcomeBg);  
    }     
    storage.saveToFile("profiles.dat");
    int savedScore = storage.getScore(playerName);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Game game;
    game.setPermanentScore(savedScore);
    if (!game.init("Epic Cannon Shooter", 900, 700)) return -1;

    while (game.running()) {
        game.handleEvents();
        game.update();
        game.render();
        SDL_Delay(16);
    }
    Profile p;
    p.name = playerName;
    p.score = game.getPlayerScore();  
    storage.insertOrUpdate(p);
    storage.saveToFile("profiles.dat");
    game.clean();
    return 0;
}
