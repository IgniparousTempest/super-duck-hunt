#ifndef DUCKHUNT_CUTSCENE_HPP
#define DUCKHUNT_CUTSCENE_HPP

#include <algorithm>
#include <SDL2/SDL.h>
#include "dog.hpp"
#include "textures.hpp"

class Environment {
protected:
    Uint64 now;
    Uint64 last;
    Drawer* drawer;
    SDL_Texture* background;
    SDL_Texture* foreground;
    Player_Stats* player_stats;
    Game_Textures* game_textures;
    UI_Textures* ui_textures;

public:
    Environment(Drawer* drawer, SDL_Texture* background, SDL_Texture* foreground, Player_Stats* player_stats, Game_Textures* game_textures, UI_Textures* ui_textures) {
        this->drawer = drawer;
        this->background = background;
        this->foreground = foreground;
        this->player_stats = player_stats;
        this->game_textures = game_textures;
        this->ui_textures = ui_textures;
    }

    /// Renders the background for this environment.
    /// \param deltaTime The time since the last frame in ms.
    /// \param returnValue The value to return.
    /// \return true if something should be returned, false otherwise.
    virtual bool renderBackground(double deltaTime, bool* returnValue) {
        drawer->renderTexture(background, 0, 0);
        return false;
    }

    virtual bool renderForeground(double deltaTime, bool* returnValue) {
        drawer->renderTexture(foreground, 0, 0);
        return false;
    }

    virtual void renderUI(double deltaTime) {
        drawer->renderUI(deltaTime, ui_textures, player_stats);
    }

    virtual bool handleInput(SDL_Event e, bool* returnValue) {
        if (e.type == SDL_QUIT) {
            *returnValue = true;
            return true;
        }
        return false;
    }

    /// Starts the environment.
    /// \return true if a quit event was triggered, false otherwise.
    bool start() {
        now = SDL_GetPerformanceCounter();
        double deltaTime;

        SDL_Event e;
        while (true) {
            bool returnValue;
            last = now;
            now = SDL_GetPerformanceCounter();
            deltaTime = ((now - last)*1000 / (double)SDL_GetPerformanceFrequency() );

            while (SDL_PollEvent(&e) != 0) {
                if (handleInput(e, &returnValue))
                    return returnValue;
            }
            SDL_RenderClear(drawer->getRenderer()); // Flush buffer

            if (renderBackground(deltaTime, &returnValue))
                return returnValue;

            if (renderForeground(deltaTime, &returnValue))
                return returnValue;

            renderUI(deltaTime);

            SDL_RenderPresent(drawer->getRenderer()); // Update screen
        }
    }
};

enum IntroCutSceneState {
    SNIFFING,
    JUMPING,
    FALLING
};

class IntroCutScene : public Environment {
private:
    IntroCutSceneState cutSceneState;
    DogSniffing dogSniffing;
    DogJumping dogJumping;
public:
    IntroCutScene(Drawer *drawer, SDL_Texture *background, SDL_Texture *foreground, Player_Stats *player_stats,
                  Game_Textures *game_textures, UI_Textures *ui_textures)
        : Environment(drawer, background, foreground, player_stats, game_textures, ui_textures),
          dogSniffing(game_textures->dog_sniffing, 7),
          dogJumping(game_textures->dog_jumping, 90) {
        cutSceneState = SNIFFING;
    }

    bool handleInput(SDL_Event e, bool* returnValue) override {
        if (Environment::handleInput(e, returnValue))
            return true;
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                *returnValue = false;
                return true;
            }
        }
        return false;
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        Environment::renderBackground(deltaTime, returnValue);

        // Dog falling down
        if (cutSceneState == FALLING)
            if (dogJumping.renderFall(drawer, deltaTime)) {
                *returnValue = false;
                return true;
            }
        return false;
    }

    bool renderForeground(double deltaTime, bool* returnValue) override {
        Environment::renderForeground(deltaTime, returnValue);

        // Dog walking from left to centre sniffing
        if (cutSceneState == SNIFFING)
            if (dogSniffing.render(drawer, deltaTime)) {
                cutSceneState = JUMPING;
                dogJumping.x = dogSniffing.x;
                dogJumping.y = dogSniffing.y;
            }

        // Dog jumping up
        if (cutSceneState == JUMPING)
            if (dogJumping.renderJump(drawer, deltaTime))
                cutSceneState = FALLING;

        return false;
    }
};

class SuccessCutScene : public Environment {
private:
    DogSuccess dogSuccess;
public:
    SuccessCutScene(Drawer *drawer, Player_Stats *player_stats, Game_Textures* game_textures, UI_Textures* ui_textures,
                    SDL_Texture* background, SDL_Texture* foreground, Duck* duck)
        : Environment(drawer, background, foreground, player_stats, game_textures, ui_textures),
          dogSuccess(std::max(120, std::min(static_cast<int>(duck->x), 210)), 157, 120, game_textures->dog_success,
                     duck->colour, 0.1) {
    }
    SuccessCutScene(Drawer *drawer, Player_Stats *player_stats, Game_Textures* game_textures, UI_Textures* ui_textures,
                    SDL_Texture* background, SDL_Texture* foreground, Duck* duck1, Duck* duck2)
        : Environment(drawer, background, foreground, player_stats, game_textures, ui_textures),
          dogSuccess(std::max(120, std::min(static_cast<int>(duck1->x), 210)), 157, 120, game_textures->dog_success,
                     duck1->colour, duck2->colour, 0.1) {
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        Environment::renderBackground(deltaTime, returnValue);

        if (dogSuccess.render(drawer, deltaTime)) {
            *returnValue = false;
            return true;
        }
        return false;
    }
};

enum GameType {NONE, SINGLE, DOUBLE};

class MainMenu : public Environment {
private:
    GameType gameType = NONE;

public:
    MainMenu(Drawer *drawer, Game_Textures* game_textures, UI_Textures* ui_textures)
        : Environment(drawer, nullptr, nullptr, nullptr, game_textures, ui_textures) {
    }

    bool handleInput(SDL_Event e, bool* returnValue) override {
        if (Environment::handleInput(e, returnValue))
            return true;
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mX, mY;
            SDL_GetMouseState(&mX, &mY);
            drawer->screenPointToWorldPoint(&mX, &mY);
            // Start single duck game
            if (mX > 106 && mX < 182 && mY > 127 && mY < 196) {
                gameType = SINGLE;
                *returnValue = false;
                return true;
            }
            // Start dual duck game
            else if (mX > 237 && mX < 313 && mY > 127 && mY < 196) {
                gameType = DOUBLE;
                *returnValue = false;
                return true;
            }
        }
        return false;
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        drawer->renderTexture(game_textures->main_menu_background, 0, 0);
        return false;
    }

    bool renderForeground(double deltaTime, bool* returnValue) override {
        return false;
    }

    void renderUI(double deltaTime) override {

    }

    GameType resultGameType() {
        return gameType;
    }
};

#endif //DUCKHUNT_CUTSCENE_HPP
