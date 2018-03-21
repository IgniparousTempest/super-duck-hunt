#ifndef DUCKHUNT_LEVEL_HPP
#define DUCKHUNT_LEVEL_HPP

#include "environment.hpp"

class Level : public Environment {
protected:
    std::vector<Duck> ducks;
    DuckHatchery hatchery;
    DuckColours previousDuckColour;

public:
    Level(Drawer *drawer, Player_Stats *player_stats, Textures *textures)
        : Environment(drawer, player_stats, textures), hatchery(textures, drawer) {
        ducks = {};
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
            DuckColours colour = brown;
            int duckColourRandom = dist(mt);
            if (duckColourRandom < 1)
                colour = red;
            else if (duckColourRandom < 5)
                colour = blue;
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

    bool trySpawnDuckOrStartNewRound() {
        // Start new round
        if (ducks.empty() && player_stats->duck_next == 10 && ducksHit() > player_stats->ducks_needed)
            startNewRound();
        // Launch new ducks
        if (trySpawnDuck())
            player_stats->shots_left = 3;

        return !player_stats->ducks_current.empty();
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
            case blue:return static_cast<int>(1000 * multiplier);
            case red:return static_cast<int>(1500 * multiplier);
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

    bool update(double deltaTime, bool* returnValue) override {
        for (auto &duck : ducks)
            duck.update(deltaTime);

        auto iter = begin(ducks);
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

                if (!trySpawnDuckOrStartNewRound()) {
                    GameOver(this).start();
                    *returnValue = false;
                    return true;
                }
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
                        killDuck(&duck);
                        break;
                    }
                }
                // Handle no shots left
                if (player_stats->shots_left == 0 && livingDucks() > 0) {
                    player_stats->ducks_current = {};
                    if (ducks.size() == 1) {
                        if (FlyAwayDuck(this, &ducks.at(0)).start()) {
                            *returnValue = true;
                            return true;
                        }
                    }
                    else if (ducks.size() == 2)
                        if (FlyAwayDuck(this, &ducks.at(0), &ducks.at(1)).start()) {
                            *returnValue = true;
                            return true;
                        }
                    ducks = {};
                    if (FailureCutScene(this).start()) {
                        *returnValue = true;
                        return true;
                    }
                    now = SDL_GetPerformanceCounter(); // TODO: Not very accurate

                    if (!trySpawnDuckOrStartNewRound()) {
                        GameOver(this).start();
                        *returnValue = false;
                        return true;
                    }
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

#endif //DUCKHUNT_LEVEL_HPP