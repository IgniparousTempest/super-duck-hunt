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

    Environment(Environment* other) {
        this->drawer = other->getDrawer();
        this->background = other->getBackground();
        this->foreground = other->getForeground();
        this->player_stats = other->getPlayerStats();
        this->game_textures = other->getGameTextures();
        this->ui_textures = other->getUiTextures();
    }

    /// Renders the background for this environment.
    /// \param deltaTime The time since the last frame in ms.
    /// \param returnValue The value to return.
    /// \return true if something should be returned, false otherwise.
    virtual bool renderBackground(double deltaTime, bool* returnValue) {
        drawer->renderTexture(background, 0, 0);
        return false;
    }

    virtual bool update(double deltaTime, bool* returnValue) {
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

            // User input
            while (SDL_PollEvent(&e) != 0) {
                if (handleInput(e, &returnValue))
                    return returnValue;
            }

            // Game Object updates
            if (update(deltaTime, &returnValue))
                return returnValue;

            // Rendering
            SDL_RenderClear(drawer->getRenderer()); // Flush buffer

            if (renderBackground(deltaTime, &returnValue))
                return returnValue;

            if (renderForeground(deltaTime, &returnValue))
                return returnValue;

            renderUI(deltaTime);

            SDL_RenderPresent(drawer->getRenderer()); // Update screen
        }
    }

    Drawer* getDrawer() {
        return drawer;
    }

    SDL_Texture* getBackground() {
        return background;
    }

    SDL_Texture* getForeground() {
        return foreground;
    }

    Player_Stats* getPlayerStats() {
        return player_stats;
    }

    Game_Textures* getGameTextures() {
        return game_textures;
    }

    UI_Textures* getUiTextures() {
        return ui_textures;
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
    SuccessCutScene(Environment* env, int duckX, DuckColours duckColour)
        : Environment(env),
          dogSuccess(std::max(120, std::min(duckX, 210)), 157, 120, game_textures->dog_success, duckColour, 0.1) {
    }
    SuccessCutScene(Environment* env, int duckX, DuckColours duck1Colour, DuckColours duck2Colour)
        : Environment(env),
          dogSuccess(std::max(120, std::min(duckX, 210)), 157, 120, game_textures->dog_success, duck1Colour, duck2Colour, 0.1) {
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

class FailureCutScene : public Environment {
private:
    DogFailure dogFailure;
public:
    FailureCutScene(Environment* env)
        : Environment(env), dogFailure(213, 157, 120, game_textures->dog_failure, 0.1, 10) {
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        Environment::renderBackground(deltaTime, returnValue);

        if (dogFailure.render(drawer, deltaTime)) {
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

class Level : public Environment {
private:
    std::vector<Duck> ducks;
    DuckHatchery hatchery;
    DuckColours previousDuckColour;

public:
    Level(Drawer *drawer, SDL_Texture *background, SDL_Texture *foreground, Player_Stats *player_stats,
          Game_Textures *game_textures, UI_Textures *ui_textures)
        : Environment(drawer, background, foreground, player_stats, game_textures, ui_textures), hatchery(game_textures, drawer) {
        ducks = {};
        trySpawnDuck(&hatchery, &ducks, player_stats);
    }

    bool update(double deltaTime, bool* returnValue) override {
        for (auto &duck : ducks)
            duck.update(deltaTime);

        auto iter = std::begin(ducks);
        while (iter != ducks.end()) {
            iter->update(deltaTime);
            if (iter->y > hatchery.spawnY) {
                if (ducks.size() == 2)
                    previousDuckColour = iter->colour;
                // Show success cut scene
                else if (ducks.size() == 1) {
                    auto x = static_cast<int>(iter->x);
                    SuccessCutScene* successCutScene;
                    if (player_stats->ducks_simultaneous == 2)
                        successCutScene = new SuccessCutScene(this, x, previousDuckColour, iter->colour);
                    else
                        successCutScene = new SuccessCutScene(this, x, iter->colour);
                    if (successCutScene->start()) {
                        *returnValue = true;
                        return true;
                    }
                    delete successCutScene;
                    now = SDL_GetPerformanceCounter(); // TODO: Not very accurate
                }
                iter = ducks.erase(iter);
                // Start new round
                if (ducks.empty() && player_stats->duck_next == 10)
                    startNewRound(player_stats);
                // Launch new ducks
                if (trySpawnDuck(&hatchery, &ducks, player_stats))
                    player_stats->shots_left = 3;
            }
            else
                iter++;
        }
    }

    bool handleInput(SDL_Event e, bool* returnValue) override {
        if (Environment::handleInput(e, returnValue))
            return true;
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (player_stats->shots_left > 0) {
                player_stats->shots_left -= 1;

                // See if duck was hit
                int mX, mY;
                SDL_GetMouseState(&mX, &mY);
                for (auto &duck : ducks) {
                    int wX = mX, wY = mY;
                    drawer->screenPointToWorldPoint(&wX, &wY);
                    if (duck.alive && wX > duck.x && wX < duck.x + duck.width() && wY > duck.y && wY < duck.y + duck.height()) {
                        killDuck(&duck, player_stats);
                        break;
                    }
                }
                // Handle no shots left
                if (player_stats->shots_left == 0 && livingDucks(&ducks) > 0) {
                    FailureCutScene(this).start();
                }
            }
        }
        return false;
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        Environment::renderBackground(deltaTime, returnValue);

        for (auto &duck : ducks)
            duck.render(drawer, deltaTime);

        return false;
    }

    bool renderForeground(double deltaTime, bool* returnValue) override {
        Environment::renderForeground(deltaTime, returnValue);

        for (auto &duck : ducks)
            if (duck.isFalling())
                duck.renderScore(drawer);

        return false;
    }
};

#endif //DUCKHUNT_CUTSCENE_HPP
