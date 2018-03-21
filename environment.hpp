#ifndef DUCKHUNT_CUTSCENE_HPP
#define DUCKHUNT_CUTSCENE_HPP

#include <algorithm>
#include <SDL2/SDL.h>
#include "dog.hpp"
#include "textures.hpp"
#include "message.hpp"

class Environment {
protected:
    Uint64 now;
    Uint64 last;
    Drawer* drawer;
    Player_Stats* player_stats;
    Textures* textures;

public:
    Environment(Drawer* drawer, Player_Stats* player_stats, Textures* textures) {
        this->drawer = drawer;
        this->player_stats = player_stats;
        this->textures = textures;
    }

    Environment(Environment* other) :
        Environment(other->getDrawer(), other->getPlayerStats(), other->getTextures()) {}

    /// Renders the background for this environment.
    /// \param deltaTime The time since the last frame in ms.
    /// \param returnValue The value to return.
    /// \return true if something should be returned, false otherwise.
    virtual bool renderBackground(double deltaTime, bool* returnValue) {
        drawer->renderTexture(textures->background, 0, 0);
        return false;
    }

    virtual bool update(double deltaTime, bool* returnValue) {
        return false;
    }

    virtual bool renderForeground(double deltaTime, bool* returnValue) {
        drawer->renderTexture(textures->foreground, 0, 0);
        return false;
    }

    virtual void renderUI(double deltaTime) {
        drawer->renderUI(deltaTime, textures, player_stats);
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
        return textures->background;
    }

    SDL_Texture* getForeground() {
        return textures->foreground;
    }

    Player_Stats* getPlayerStats() {
        return player_stats;
    }

    Textures* getTextures() {
        return textures;
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
    RoundMessage roundMessage;

public:
    IntroCutScene(Drawer *drawer, Player_Stats *player_stats, Textures *textures)
        : Environment(drawer, player_stats, textures), dogSniffing(textures->dog_sniffing, 7),
          dogJumping(textures->dog_jumping, 90), roundMessage(189, 52, 2500.0, textures->ui_message_round, 1, textures->ui_numbers_white) {
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

        roundMessage.render(drawer, deltaTime);
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
    SuccessCutScene(Environment* env, int duckX, DuckColours duckColour) : Environment(env),
          dogSuccess(std::max(120, std::min(duckX, 210)), 157, 120, textures->dog_success, duckColour, 0.1) {
    }
    SuccessCutScene(Environment* env, int duckX, DuckColours duck1Colour, DuckColours duck2Colour)
        : Environment(env),
          dogSuccess(std::max(120, std::min(duckX, 210)), 157, 120, textures->dog_success, duck1Colour, duck2Colour, 0.1) {
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
    explicit FailureCutScene(Environment* env)
        : Environment(env), dogFailure(213, 157, 120, textures->dog_failure, 0.1, 10) {
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
    MainMenu(Drawer *drawer, Textures* textures)
        : Environment(drawer, nullptr, textures) {
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
        drawer->renderTexture(textures->main_menu_background, 0, 0);
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

class FlyAwayDuck : public Environment {
private:
    Duck* duck1;
    Duck* duck2;

public:
    FlyAwayDuck(Environment* env, Duck* duck1, Duck* duck2 = nullptr) : Environment(env) {
        this->duck1 = duck1;
        this->duck2 = duck2;

        this->duck1->flyUp();
        if (this->duck2 != nullptr)
            this->duck2->flyUp();
    }

    bool update(double deltaTime, bool* returnValue) override {
        if (Environment::update(deltaTime, returnValue))
            return *returnValue;

        duck1->update(deltaTime);
        if (duck2 != nullptr)
            duck2->update(deltaTime);

        if ((!duck1->isOnScreen() && duck2 == nullptr) || (duck2 != nullptr && !duck2->isOnScreen())) {
            *returnValue = false;
            return true;
        }
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        drawer->renderTexture(textures->background_fail, 0, 0);

        duck1->render(drawer, deltaTime);
        if (duck2 != nullptr)
            duck2->render(drawer, deltaTime);

        return false;
    }

    bool renderForeground(double deltaTime, bool* returnValue) override {
        if (Environment::renderForeground(deltaTime, returnValue))
            return *returnValue;

        drawer->renderTexture(textures->ui_message_fly_away, 177, 60);

        return false;
    }
};

class GameOver : public Environment {
private:
    DogGameOver dogGameOver;

public:
    GameOver(Environment* env) : Environment(env), dogGameOver(213, 157, 120, textures->dog_failure, 0.1, 10) {
    }

    bool renderBackground(double deltaTime, bool* returnValue) override {
        drawer->renderTexture(textures->background_fail, 0, 0);

        if (dogGameOver.render(drawer, deltaTime)) {
            *returnValue = false;
            return true;
        }

        return false;
    }

    bool renderForeground(double deltaTime, bool* returnValue) override {
        if (Environment::renderForeground(deltaTime, returnValue))
            return *returnValue;

        drawer->renderTexture(textures->ui_message_game_over, 173, 44);

        return false;
    }
};

#endif //DUCKHUNT_CUTSCENE_HPP
