#include "leaderboard.h"
#include <algorithm>
#include <iostream>
#include <cctype>
#include <locale>

static inline std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
    return s.substr(start, end - start + 1);
}
static inline std::string toLower(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}
static inline std::string normalizeKey(const std::string& s) {
    return toLower(trim(s));
}

Leaderboard::Leaderboard() {}
Leaderboard::~Leaderboard() {}
bool Leaderboard::loadFromFile(const std::string& path) {
    StorageHash s;
    if (!s.loadFromFile(path)) {
        std::cout << "[Leaderboard] Warning: storage load failed for " << path << "\n";
    }

    auto all = s.getAllProfiles();

    std::sort(all.begin(), all.end(), [](const Profile& a, const Profile& b) {
        if (a.score != b.score) return a.score > b.score;
        std::string an = toLower(a.name);
        std::string bn = toLower(b.name);
        return an < bn;
    });

    entries.clear();
    ranks.clear();
    const int MAX_ENTRIES = 10;
    size_t topN = std::min<size_t>(all.size(), (size_t)MAX_ENTRIES);
    for (size_t i = 0; i < topN; ++i) {
        entries.push_back(all[i]);
        ranks.push_back(static_cast<int>(i) + 1);
    }

    if (!currentPlayerKey.empty()) {
        bool found = false;
        for (const auto &p : entries) {
            if (normalizeKey(p.name) == currentPlayerKey) { found = true; break; }
        }
        if (!found) {
            auto opt = s.find(currentPlayer);
            if (opt.has_value()) {
                Profile p = opt.value();
                int r = s.getRank(p.name);
                entries.push_back(p);
                ranks.push_back(r);
            }
        }
    }
    return true;
}
void Leaderboard::setCurrentPlayer(const std::string& name) {
    currentPlayer = trim(name);
    currentPlayerKey = normalizeKey(name);
}
void Leaderboard::render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int maxEntries) {
    if (!renderer) return;
    if (!font) {
        std::cout << "[Leaderboard] Null font passed to render().\n";
        return;
    }

    SDL_Color white = {255,255,255,255};
    SDL_Color gold  = {255,215,0,255};
    SDL_Color gray  = {200,200,200,255};

    int rowH = 0;
    {
        int tw, th;
        TTF_SizeText(font, "Ay", &tw, &th);
        rowH = th + 8;
        if (rowH < 24) rowH = 24;
    }

    int toDraw = static_cast<int>(std::min<size_t>(entries.size(), (size_t)maxEntries));
    int rankX = x; 
    int nameX = x + 60;
    int scoreCenterX = x + 520;

    for (int i = 0, drawIndex = 0; drawIndex < toDraw && i < (int)entries.size(); ++i) {
        bool isAppended = (i == (int)entries.size() - 1) && (entries.size() > (size_t)maxEntries);

        if (drawIndex >= maxEntries) break;
        if (isAppended && (int)entries.size() > maxEntries) {
            if (drawIndex >= maxEntries - 1) {
                int sepY = y + drawIndex * rowH;
                SDL_Surface* sDot = TTF_RenderText_Solid(font, "...", gray);
                if (sDot) {
                    SDL_Texture* tDot = SDL_CreateTextureFromSurface(renderer, sDot);
                    if (tDot) {
                        SDL_Rect rDot = { nameX, sepY, sDot->w, sDot->h };
                        SDL_RenderCopy(renderer, tDot, nullptr, &rDot);
                        SDL_DestroyTexture(tDot);
                    }
                    SDL_FreeSurface(sDot);
                }
                drawIndex++; 

                if (drawIndex >= maxEntries) break;
                int rowY = y + drawIndex * rowH;
                const Profile& p = entries[i];
                int rankToShow = ranks.size() > (size_t)i ? ranks[i] : (drawIndex + 1);

                bool isCurrent = (!currentPlayerKey.empty() && normalizeKey(p.name) == currentPlayerKey);

                if (isCurrent) {
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 90); 
                    SDL_Rect hl = { x - 20, rowY - 6, 740 + 40, rowH };
                    SDL_RenderFillRect(renderer, &hl);
                }
                std::string rankStr = std::to_string(rankToShow) + ".";
                SDL_Surface* sRank = TTF_RenderText_Solid(font, rankStr.c_str(), isCurrent ? gold : gray);
                if (sRank) {
                    SDL_Texture* tRank = SDL_CreateTextureFromSurface(renderer, sRank);
                    if (tRank) {
                        SDL_Rect rRank = { rankX, rowY, sRank->w, sRank->h };
                        SDL_RenderCopy(renderer, tRank, nullptr, &rRank);
                        SDL_DestroyTexture(tRank);
                    }
                    SDL_FreeSurface(sRank);
                }
                SDL_Surface* sName = TTF_RenderText_Solid(font, p.name.c_str(), isCurrent ? gold : white);
                if (sName) {
                    SDL_Texture* tName = SDL_CreateTextureFromSurface(renderer, sName);
                    if (tName) {
                        SDL_Rect rName = { nameX, rowY, sName->w, sName->h };
                        SDL_RenderCopy(renderer, tName, nullptr, &rName);
                        SDL_DestroyTexture(tName);
                    }
                    SDL_FreeSurface(sName);
                }

                std::string scoreStr = std::to_string(p.score);
                SDL_Surface* sScore = TTF_RenderText_Solid(font, scoreStr.c_str(), isCurrent ? gold : white);
                if (sScore) {
                    SDL_Texture* tScore = SDL_CreateTextureFromSurface(renderer, sScore);
                    if (tScore) {
                        SDL_Rect rScore = { scoreCenterX - sScore->w/2, rowY, sScore->w, sScore->h };
                        SDL_RenderCopy(renderer, tScore, nullptr, &rScore);
                        SDL_DestroyTexture(tScore);
                    }
                    SDL_FreeSurface(sScore);
                }

                break;
            }
        }

        int rowY = y + drawIndex * rowH;
        const Profile& p = entries[i];
        int rankToShow = ranks.size() > (size_t)i ? ranks[i] : (i + 1);
        bool isCurrent = (!currentPlayerKey.empty() && normalizeKey(p.name) == currentPlayerKey);

        if (isCurrent) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 90); 
            SDL_Rect hl = { x - 20, rowY - 6, 740 + 40, rowH };
            SDL_RenderFillRect(renderer, &hl);
        }

        std::string rankStr = std::to_string(rankToShow) + ".";
        SDL_Surface* sRank = TTF_RenderText_Solid(font, rankStr.c_str(), isCurrent ? gold : gray);
        if (sRank) {
            SDL_Texture* tRank = SDL_CreateTextureFromSurface(renderer, sRank);
            if (tRank) {
                SDL_Rect rRank = { rankX, rowY, sRank->w, sRank->h };
                SDL_RenderCopy(renderer, tRank, nullptr, &rRank);
                SDL_DestroyTexture(tRank);
            }
            SDL_FreeSurface(sRank);
        }

        SDL_Surface* sName = TTF_RenderText_Solid(font, p.name.c_str(), isCurrent ? gold : white);
        if (sName) {
            SDL_Texture* tName = SDL_CreateTextureFromSurface(renderer, sName);
            if (tName) {
                SDL_Rect rName = { nameX, rowY, sName->w, sName->h };
                SDL_RenderCopy(renderer, tName, nullptr, &rName);
                SDL_DestroyTexture(tName);
            }
            SDL_FreeSurface(sName);
        }

        std::string scoreStr = std::to_string(p.score);
        SDL_Surface* sScore = TTF_RenderText_Solid(font, scoreStr.c_str(), isCurrent ? gold : white);
        if (sScore) {
            SDL_Texture* tScore = SDL_CreateTextureFromSurface(renderer, sScore);
            if (tScore) {
                SDL_Rect rScore = { scoreCenterX - sScore->w/2, rowY, sScore->w, sScore->h };
                SDL_RenderCopy(renderer, tScore, nullptr, &rScore);
                SDL_DestroyTexture(tScore);
            }
            SDL_FreeSurface(sScore);
        }

        ++drawIndex;
    }
    if (entries.empty()) {
        SDL_Surface* sMsg = TTF_RenderText_Solid(font, "No scores yet.", gray);
        if (sMsg) {
            SDL_Texture* tMsg = SDL_CreateTextureFromSurface(renderer, sMsg);
            if (tMsg) {
                SDL_Rect r = { (900 - sMsg->w)/2, y + 10, sMsg->w, sMsg->h };
                SDL_RenderCopy(renderer, tMsg, nullptr, &r);
                SDL_DestroyTexture(tMsg);
            }
            SDL_FreeSurface(sMsg);
        }
    }
}