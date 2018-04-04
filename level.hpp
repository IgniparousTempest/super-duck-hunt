#ifndef DUCKHUNT_LEVEL_HPP
#define DUCKHUNT_LEVEL_HPP

#include "scene.hpp"

class Level : public Scene {
protected:
    std::vector<Duck> ducks;
    DuckHatchery hatchery;
    DuckColours firstDuckColour;

public:
    Level(Drawer *drawer, Player_Stats *player_stats, Textures *textures)
        : Scene(drawer, player_stats, textures), hatchery(textures, drawer) {
        ducks = {};
        firstDuckColour = NO_COLOUR;
        trySpawnDuck();
    }

    int livingDucks() {
        int count = 0;
        for (auto &duck : ducks)
            if (duck.alive)
                count++;
        return count;
    }

    /// Spawns a duck
    void spawnDuck() {
        std::random_device rd;
        auto mt = std::mt19937(rd());
        std::uniform_int_distribution<int> dist(0, 10);
        for (int i = 0; i < player_stats->ducks_simultaneous; ++i) {
            DuckColours colour = BROWN;
            int duckColourRandom = dist(mt);
            if (duckColourRandom < 1)
                colour = RED;
            else if (duckColourRandom < 5)
                colour = BLUE;
            ducks.push_back(hatchery.newDuck(colour, scoreForDuck(player_stats->round, colour), player_stats->round, player_stats->duck_next));
            player_stats->ducks_current.push_back(player_stats->duck_next++);
        }
    }

    /// Spawns a duck if there are still ducks left in the round and no ducks on screen.
    /// \return true if a duck was spawned, false otherwise.
    bool trySpawnDuck() {
        if (ducks.empty() && player_stats->duck_next < 10) {
            spawnDuck();
            return true;
        }
        return false;
    }

    void startNewRound() {
        player_stats->round++;
        player_stats->duck_next = 0;
        player_stats->ducks_needed = ducksNeededForRound(player_stats->round);
        for (bool &duck_hit : player_stats->ducks_hit)
            duck_hit = false;
    }

    bool areDucksFinished() {
        return ducks.empty() && player_stats->duck_next == 10;
    }

    int ducksHit() {
        int count = 0;
        for (bool i : player_stats->ducks_hit) {
            if (i)
                count++;
        }
        return count;
    }

    void killDuck (Duck* duck) {
        player_stats->score += duck->kill();
        // Remove duck from vector
        player_stats->ducks_current.erase(std::remove(player_stats->ducks_current.begin(), player_stats->ducks_current.end(), duck->index), player_stats->ducks_current.end());
        player_stats->ducks_hit[duck->index] = true;
    }

    static int scoreForDuck(int round, DuckColours colour) {
        double multiplier;
        if (round <= 5)
            multiplier = 1.0;
        else if (round <= 10)
            multiplier = 1.6;
        else
            multiplier = 2.0;
        switch (colour) {
            case BLUE:return static_cast<int>(1000 * multiplier);
            case RED:return static_cast<int>(1500 * multiplier);
            default:return static_cast<int>(500 * multiplier);
        }
    }

    static int ducksNeededForRound(int round) {
        if (round <= 10)
            return 6;
        if (round <= 12)
            return 7;
        if (round <= 14)
            return 8;
        if (round <= 19)
            return 9;
        return 10;
    }

    static Player_Stats singleDuckGame() {
        return {
            .ducks_hit = {},
            .ducks_current={},
            .ducks_needed = ducksNeededForRound(1),
            .duck_next = 0, .ducks_simultaneous = 1,
            .round = 1,
            .score = 0,
            .shots_left = 3
        };
    }

    static Player_Stats doubleDuckGame() {
        Player_Stats stats = singleDuckGame();
        stats.ducks_simultaneous = 2;
        return stats;
    }
};

class SinglePlayerGame : public Level {
public:
    SinglePlayerGame(Drawer *drawer, Player_Stats *player_stats, Textures *textures)
    : Level(drawer, player_stats, textures) {
    }

    bool update(double deltaTime) override {
        for (auto &duck : ducks)
            duck.update(deltaTime);

        auto iter = begin(ducks);
        while (iter != ducks.end()) {
            iter->update(deltaTime);
            if (iter->y > hatchery.spawnY) {
                if (ducks.size() == 2)
                    firstDuckColour = iter->colour;
                // Show success cut scene
                else if (ducks.size() == 1) {
                    auto x = static_cast<int>(iter->x);
                    SuccessCutScene* successCutScene;
                    if (player_stats->ducks_simultaneous == 2)
                        successCutScene = new SuccessCutScene(this, x, firstDuckColour, iter->colour);
                    else
                        successCutScene = new SuccessCutScene(this, x, iter->colour);
                    successCutScene->start();
                    delete successCutScene;
                    now = SDL_GetPerformanceCounter(); // TODO: Not very accurate
                }
                iter = ducks.erase(iter);

                if (trySpawnDuckOrStartNewRound())
                    return true;
            }
            else
                iter++;
        }
    }

    bool handleInput(SDL_Event e) override {
        Scene::handleInput(e);
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
                        killDuck(&duck);
                        break;
                    }
                }
                // Handle no shots left
                if (player_stats->shots_left == 0 && livingDucks() > 0) {
                    player_stats->ducks_current = {};
                    Duck* duck2 = nullptr;
                    if (ducks.size() == 2)
                        duck2 = &ducks.at(1);
                    FlyAwayDuck(this, &ducks.at(0), duck2).start();
                    ducks = {};
                    FailureCutScene(this).start();
                    now = SDL_GetPerformanceCounter(); // TODO: Not very accurate

                    if (trySpawnDuckOrStartNewRound())
                        return true;
                }
            }
        }
        return false;
    }

    bool renderBackground(double deltaTime) override {
        Scene::renderBackground(deltaTime);

        for (auto &duck : ducks)
            duck.render(drawer, deltaTime);

        return false;
    }

    bool renderForeground(double deltaTime) override {
        Scene::renderForeground(deltaTime);

        for (auto &duck : ducks)
            if (duck.isFalling())
                duck.renderScore(drawer);

        return false;
    }

    bool trySpawnDuckOrStartNewRound() {
        // Start new round
        if (areDucksFinished()) {
            DuckUICoalesce(this).start();
            now = SDL_GetPerformanceCounter(); // TODO: Not very accurate
            if (ducksHit() >= player_stats->ducks_needed) {
                DuckUIFlash(this).start();
                now = SDL_GetPerformanceCounter(); // TODO: Not very accurate
                startNewRound();
            }
            else {
                GameOver(this).start();
                return true;
            }
        }
        // Launch new ducks
        if (trySpawnDuck())
            player_stats->shots_left = 3;

        return false;
    }
};

#endif //DUCKHUNT_LEVEL_HPP