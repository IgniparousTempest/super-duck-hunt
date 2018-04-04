#ifndef DUCKHUNT_CUTSCENE_HPP
#define DUCKHUNT_CUTSCENE_HPP

#include <algorithm>
#include <SDL2/SDL.h>
#include "dog.hpp"
#include "textures.hpp"
#include "message.hpp"

class Scene {
protected:
    Uint64 now;
    Uint64 last;
    Drawer* drawer;
    Player_Stats* player_stats;
    Textures* textures;

public:
    Scene(Drawer* drawer, Player_Stats* player_stats, Textures* textures) {
        this->drawer = drawer;
        this->player_stats = player_stats;
        this->textures = textures;
        now = 0;
        last = 0;
    }

    explicit Scene(Scene* other) :
        Scene(other->getDrawer(), other->getPlayerStats(), other->getTextures()) {}

    /// Renders the background for this environment.
    /// \param deltaTime The time since the last frame in ms.
    /// \return true if environment should end, false otherwise.
    /// \throws QuitTrigger if the user tried to quit the game.
    virtual bool renderBackground(double deltaTime) {
        drawer->renderTexture(textures->background, 0, 0);
        return false;
    }

    virtual bool update(double deltaTime) {
        return false;
    }

    /// Renders the foreground for this environment. Called after ::renderBackground(double deltaTime).
    /// \param deltaTime The time since the last frame in ms.
    /// \return true if environment should end, false otherwise.
    /// \throws QuitTrigger if the user tried to quit the game.
    virtual bool renderForeground(double deltaTime) {
        drawer->renderTexture(textures->foreground, 0, 0);
        return false;
    }

    /// Renders the UI for this environment.
    /// \param deltaTime The time since the last frame in ms.
    /// \return true if environment should end, false otherwise.
    /// \throws QuitTrigger if the user tried to quit the game.
    virtual void renderUI(double deltaTime) {
        drawer->renderUI(deltaTime, textures, player_stats);
    }

    /// handles the user input for this environment.
    /// \param deltaTime The time since the last frame in ms.
    /// \return true if environment should end, false otherwise.
    /// \throws QuitTrigger if the user tried to quit the game.
    virtual bool handleInput(SDL_Event e) {
        if (e.type == SDL_QUIT)
            throw QuitTrigger();
        return false;
    }

    /// Starts the environment.
    /// \throws QuitTrigger if the user tried to quit the game.
    void start() {
        now = SDL_GetPerformanceCounter();
        double deltaTime;

        SDL_Event e{};
        while (true) {
            last = now;
            now = SDL_GetPerformanceCounter();
            deltaTime = ((now - last)*1000 / (double)SDL_GetPerformanceFrequency() );

            // User input
            while (SDL_PollEvent(&e) != 0) {
                if (handleInput(e))
                    return;
            }

            // Game Object updates
            if (update(deltaTime))
                return;

            // Rendering
            SDL_RenderClear(drawer->getRenderer()); // Flush buffer

            if (renderBackground(deltaTime))
                return;

            if (renderForeground(deltaTime))
                return;

            renderUI(deltaTime);

            SDL_RenderPresent(drawer->getRenderer()); // Update screen
        }
    }

    Drawer* getDrawer() {
        return drawer;
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

/// Shows the intro to the game, i.e. the dog sniffing and jumping into the bushes.
class IntroCutScene : public Scene {
private:
    IntroCutSceneState cutSceneState;
    DogSniffing dogSniffing;
    DogJumping dogJumping;
    RoundMessage roundMessage;

public:
    IntroCutScene(Drawer *drawer, Player_Stats *player_stats, Textures *textures)
        : Scene(drawer, player_stats, textures), dogSniffing(textures->dog_sniffing, 7),
          dogJumping(textures->dog_jumping, 90), roundMessage(189, 52, 2500.0, textures->ui_message_round, 1, textures->ui_numbers_white) {
        cutSceneState = SNIFFING;
    }

    bool handleInput(SDL_Event e) override {
        if (Scene::handleInput(e))
            return true;
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                return true;
        }
        return false;
    }

    bool renderBackground(double deltaTime) override {
        Scene::renderBackground(deltaTime);

        // Dog falling down
        if (cutSceneState == FALLING)
            if (dogJumping.renderFall(drawer, deltaTime))
                return true;

        roundMessage.render(drawer, deltaTime);
        return false;
    }

    bool renderForeground(double deltaTime) override {
        Scene::renderForeground(deltaTime);

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

/// Shows the cut scene for successfully shooting a duck or ducks, i.e. the dog rising from the bushes holding the dead ducks.
class SuccessCutScene : public Scene {
private:
    DogSuccess dogSuccess;
public:
    SuccessCutScene(Scene* env, int duckX, DuckColours duckColour) : Scene(env),
          dogSuccess(std::max(120, std::min(duckX, 210)), 157, 120, textures->dog_success, duckColour, 0.1) {
    }
    SuccessCutScene(Scene* env, int duckX, DuckColours duck1Colour, DuckColours duck2Colour)
        : Scene(env),
          dogSuccess(std::max(120, std::min(duckX, 210)), 157, 120, textures->dog_success, duck1Colour, duck2Colour, 0.1) {
    }

    bool renderBackground(double deltaTime) override {
        Scene::renderBackground(deltaTime);

        return dogSuccess.render(drawer, deltaTime);
    }
};

/// Shows the cut scene for failing to shoot any ducks, i.e. the dog rising from the bushes and laughing.
class FailureCutScene : public Scene {
private:
    DogFailure dogFailure;
public:
    explicit FailureCutScene(Scene* env)
        : Scene(env), dogFailure(213, 157, 120, textures->dog_failure, 0.1, 10) {
    }

    bool renderBackground(double deltaTime) override {
        Scene::renderBackground(deltaTime);

        return dogFailure.render(drawer, deltaTime);
    }
};

enum GameType {SINGLE, DOUBLE};

/// The main menu environment.
class MainMenu : public Scene {
private:
    GameType gameType;
    std::string highScore;

public:
    MainMenu(Drawer *drawer, Textures* textures, int highScore)
        : Scene(drawer, nullptr, textures) {
        this->highScore = std::to_string(highScore);
        gameType = SINGLE;
    }

    bool handleInput(SDL_Event e) override {
        if (Scene::handleInput(e))
            return true;
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mX, mY;
            SDL_GetMouseState(&mX, &mY);
            drawer->screenPointToWorldPoint(&mX, &mY);
            // Start single duck game
            if (mX > 106 && mX < 182 && mY > 127 && mY < 196) {
                gameType = SINGLE;
                return true;
            }
            // Start dual duck game
            else if (mX > 237 && mX < 313 && mY > 127 && mY < 196) {
                gameType = DOUBLE;
                return true;
            }
        }
        return false;
    }

    bool renderBackground(double deltaTime) override {
        drawer->renderTexture(textures->main_menu_background, 0, 0);
        return false;
    }

    bool renderForeground(double deltaTime) override {
        return false;
    }

    void renderUI(double deltaTime) override {
        for (int i = 0; i < highScore.size(); ++i)
            drawer->renderCharacter(textures->ui_numbers_green, highScore[i], 238 + i * 8, 209);
    }

    GameType resultGameType() {
        return gameType;
    }
};

/// When the duck flew away after the player runs out of bullets.
class FlyAwayDuck : public Scene {
private:
    Duck* duck1;
    Duck* duck2;

public:
    FlyAwayDuck(Scene* env, Duck* duck1, Duck* duck2 = nullptr) : Scene(env) {
        this->duck1 = duck1;
        this->duck2 = duck2;

        this->duck1->flyUp();
        if (this->duck2 != nullptr)
            this->duck2->flyUp();
    }

    bool update(double deltaTime) override {
        Scene::update(deltaTime);

        duck1->update(deltaTime);
        if (duck2 != nullptr)
            duck2->update(deltaTime);

        if ((!duck1->isOnScreen() && duck2 == nullptr) || (duck2 != nullptr && !duck2->isOnScreen()))
            return true;
    }

    bool renderBackground(double deltaTime) override {
        drawer->renderTexture(textures->background_fail, 0, 0);

        duck1->render(drawer, deltaTime);
        if (duck2 != nullptr)
            duck2->render(drawer, deltaTime);

        return false;
    }

    bool renderForeground(double deltaTime) override {
        Scene::renderForeground(deltaTime);

        drawer->renderTexture(textures->ui_message_fly_away, 177, 60);

        return false;
    }
};

/// The dog laughs at the player and the game over message is displayed.
class GameOver : public Scene {
private:
    DogGameOver dogGameOver;

public:
    explicit GameOver(Scene* env) : Scene(env), dogGameOver(213, 157, 120, textures->dog_failure, 0.1, 10) {
    }

    bool renderBackground(double deltaTime) override {
        drawer->renderTexture(textures->background_fail, 0, 0);

        return dogGameOver.render(drawer, deltaTime);

    }

    bool renderForeground(double deltaTime) override {
        Scene::renderForeground(deltaTime);

        drawer->renderTexture(textures->ui_message_game_over, 173, 44);

        return false;
    }
};

/// Flashes the ducks' hit ui after the round is complete.
class DuckUIFlash : public Scene {
private:
    Player_Stats stats_template;
    Player_Stats stats;
    Timer timer;
    bool showTemplate;
    int flashes;
    const int maxFlashes = 5 * 2;

public:
    explicit DuckUIFlash(Scene* env) : Scene(env), timer(500.0) {
        stats_template = *player_stats;
        stats_template.ducks_current = {};
        stats = stats_template;
        showTemplate = true;
        flashes = 0;
    }

    bool update(double deltaTime) override {
        Scene::update(deltaTime);

        return flashes >= maxFlashes;
    }

    void renderUI(double deltaTime) override {
        Scene::renderUI(deltaTime);

        if (timer.tick(deltaTime)) {
            for (int i = 0; i < stats.ducks_hit.size(); ++i) {
                if (showTemplate)
                    stats.ducks_hit[i] = stats_template.ducks_hit[i];
                else
                    stats.ducks_hit[i] = false;
            }
            showTemplate = !showTemplate;
            flashes++;
        }

        drawer->renderUI(deltaTime, textures, &stats);
    }
};

/// Moves ducks' hit ui to the left after a round is complete.
class DuckUICoalesce : public Scene {
private:
    Timer timer;
    bool done;

public:
    explicit DuckUICoalesce(Scene* env) : Scene(env), timer(500.0) {
        done = false;
    }

    bool update(double deltaTime) override {
        Scene::update(deltaTime);

        return done;
    }

    void renderUI(double deltaTime) override {
        Scene::renderUI(deltaTime);

        if (timer.tick(deltaTime)) {
            done = true;
            for (int i = 1; i < player_stats->ducks_hit.size(); ++i)
                if (!player_stats->ducks_hit[i - 1] && player_stats->ducks_hit[i]) {
                    player_stats->ducks_hit[i] = false;
                    player_stats->ducks_hit[i - 1] = true;
                    done = false;
                }
        }

        drawer->renderUI(deltaTime, textures, player_stats);
    }
};

#endif //DUCKHUNT_CUTSCENE_HPP
