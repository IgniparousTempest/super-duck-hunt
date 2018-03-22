#ifndef DUCKHUNT_PLAYER_STATS_HPP
#define DUCKHUNT_PLAYER_STATS_HPP

#include <vector>

struct Player_Stats {
    std::array<bool, 10> ducks_hit;
    /// The ducks currently on screen, e.g. duck 5 and duck 6.
    std::vector<int> ducks_current;
    int ducks_needed;
    /// The next duck to release
    int duck_next;
    int ducks_simultaneous;
    int round;
    int score;
    int shots_left;
};

#endif //DUCKHUNT_PLAYER_STATS_HPP
