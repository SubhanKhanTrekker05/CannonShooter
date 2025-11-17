#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

enum class LoginResult {
    NONE,
    START,
    EXIT
};
class Login {
public:
    Login();
    ~Login();  
    void init(SDL_Renderer* ren, int w, int h);
    LoginResult updateAndRender();
    std::string getPlayerName() const;
    void clean();

private:
    SDL_Renderer* renderer;
    int windowW, windowH;
    bool done;
    std::string playerName;
    TTF_Font* font;
    SDL_Texture* bgTexture = nullptr;
};
