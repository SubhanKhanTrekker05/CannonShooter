#include "login.h"
#include <SDL_ttf.h>
#include<SDL_image.h>
#include <iostream>

Login::Login():renderer(nullptr), windowW(0), windowH(0), done(false),playerName(""), font(nullptr), bgTexture(nullptr){} 
Login::~Login() {
    clean(); 
}

void Login::init(SDL_Renderer* ren, int w, int h) {
    renderer = ren;
    windowW = w;
    windowH = h;
    playerName = "";

    if (TTF_Init() == -1) {
        std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
    }
    font = TTF_OpenFont("arial.ttf", 28);
    if (!font) {
        std::cout << "Font load error: " << TTF_GetError() << std::endl;
    }
    SDL_Surface* bg = IMG_Load("assets/login_bg.png");
    if (!bg) {
        std::cout << "Login background load warning: " << IMG_GetError() << std::endl;
    } else {
        bgTexture = SDL_CreateTextureFromSurface(renderer, bg);
        SDL_FreeSurface(bg);
    }
    SDL_StartTextInput();
}

LoginResult Login::updateAndRender() {
    LoginResult result = LoginResult::NONE;
    SDL_Rect inputRect = { windowW / 4, 300, windowW / 2, 50 };
    SDL_Rect startRect = { windowW / 2 - 100, 400, 200, 50 };  
    SDL_Rect exitRect  = { windowW / 2 - 100, 480, 200, 50 };
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            done = true;
        }
        else if (event.type == SDL_TEXTINPUT) {
            playerName += event.text.text;  
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && !playerName.empty()) {
                playerName.pop_back();
            }
            else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_RETURN2) {
                if (!playerName.empty()) done = true;  
            }
        }else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x;
            int my = event.button.y;
            if (!playerName.empty() && mx >= startRect.x && mx <= startRect.x + startRect.w && my >= startRect.y && my <= startRect.y + startRect.h) {
                result = LoginResult::START;
                done = true;
            }
            else if (mx >= exitRect.x && mx <= exitRect.x + exitRect.w && my >= exitRect.y && my <= exitRect.y + exitRect.h) {
                result = LoginResult::EXIT;
                done = true;
            }
        }
    }
    if (bgTexture) {
        SDL_Rect full = {0, 0, windowW, windowH};
        SDL_RenderCopy(renderer, bgTexture, nullptr, &full);
    }else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &inputRect);

    if (font && !playerName.empty()) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Solid(font, playerName.c_str(), white);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect = { inputRect.x + 10, inputRect.y + 10, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 200);
    SDL_RenderFillRect(renderer, &startRect);
    if (font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* s = TTF_RenderText_Solid(font, "START GAME", white);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {
            startRect.x + (startRect.w - s->w) / 2,
            startRect.y + (startRect.h - s->h) / 2,
            s->w, s->h
        };
        SDL_RenderCopy(renderer, t, nullptr, &r);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    SDL_SetRenderDrawColor(renderer, 150, 0, 0, 200);
    SDL_RenderFillRect(renderer, &exitRect);
    if (font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* s = TTF_RenderText_Solid(font, "EXIT", white);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {
            exitRect.x + (exitRect.w - s->w) / 2,
            exitRect.y + (exitRect.h - s->h) / 2,
            s->w, s->h
        };
        SDL_RenderCopy(renderer, t, nullptr, &r);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    SDL_RenderPresent(renderer);
    return result;
}
std::string Login::getPlayerName() const {
    return playerName;
}
void Login::clean() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    SDL_StopTextInput();
    TTF_Quit();
    if (bgTexture) {
        SDL_DestroyTexture(bgTexture);
        bgTexture = nullptr;
    }
}