#ifndef DUCKHUNT_LEVEL_LOGIC_HPP
#define DUCKHUNT_LEVEL_LOGIC_HPP

#include "duck.hpp"

int scoreForDuck(int round, DuckColours colour) {
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

int ducksNeededForRound(int round) {
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

Player_Stats singleDuckGame() {
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

int livingDucks(std::vector<Duck>* ducks) {
    int count = 0;
    for (auto &duck : *ducks)
        if (duck.alive)
            count++;
    return count;
}

Player_Stats doubleDuckGame() {
    Player_Stats stats = singleDuckGame();
    stats.ducks_simultaneous = 2;
    return stats;
}

void killDuck (Duck* duck, Player_Stats* player_stats) {
    player_stats->score += duck->kill();
    // Remove duck from vector
    player_stats->ducks_current.erase(std::remove(player_stats->ducks_current.begin(), player_stats->ducks_current.end(), duck->index), player_stats->ducks_current.end());
    player_stats->ducks_hit[duck->index] = true;
}

void startNewRound(Player_Stats* player_stats) {
    player_stats->round++;
    player_stats->duck_next = 0;
    player_stats->ducks_needed = ducksNeededForRound(player_stats->round);
    for (bool &duck_hit : player_stats->ducks_hit)
        duck_hit = false;
}

void spawnDuck(DuckHatchery* hatchery, std::vector<Duck>* ducks, Player_Stats* player_stats) {
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
        ducks->push_back(hatchery->newDuck(colour, scoreForDuck(player_stats->round, colour), player_stats->round, player_stats->duck_next));
        player_stats->ducks_current.push_back(player_stats->duck_next++);
    }
}

/// Spawns a duck if there are still ducks left in the round and no ducks on screen.
/// \param hatchery
/// \param ducks
/// \param player_stats
/// \return true if a duck was spawned, false otherwise.
bool trySpawnDuck(DuckHatchery* hatchery, std::vector<Duck>* ducks, Player_Stats* player_stats) {
    if (ducks->empty() && player_stats->duck_next < 10) {
        spawnDuck(hatchery, ducks, player_stats);
        return true;
    }
    return false;
}

#endif //DUCKHUNT_LEVEL_LOGIC_HPP
