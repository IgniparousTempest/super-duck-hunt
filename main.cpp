#include <iostream>
#include "cleanup.hpp"
#include "dog.hpp"
#include "level_logic.hpp"
#include "cutscene.hpp"

//const int SCREEN_WIDTH  = 960;
//const int SCREEN_HEIGHT = 540;
const int SCREEN_WIDTH  = 256 * 3;
const int SCREEN_HEIGHT = 224 * 3;
Uint64 NOW;
Uint64 LAST;
double deltaTime;

int main(int argc, char* argv []) {
    // Start SDL
    putenv(const_cast<char *>("SDL_VIDEO_CENTERED=1"));
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        logSDLError(std::cout, "SDL_Init");
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Duck Hunt", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        logSDLError(std::cout, "CreateWindow");
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        logSDLError(std::cout, "CreateRenderer");
        cleanup(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *background = loadTexture("textures/background.png", renderer);
    SDL_Texture *foreground = loadTexture("textures/foreground.png", renderer);
    UI_Textures ui_textures = loadUiTexturesRemake(renderer);
    Game_Textures game_textures = loadGameTexturesRemake(renderer);
    if (background == nullptr || foreground == nullptr || !validateGameTextures(&game_textures) || !validateUiTextures(&ui_textures)){
        cleanup(background, foreground, &ui_textures, &game_textures, renderer, window);
        SDL_Quit();
        return 1;
    }

    Drawer drawer(background, renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    Player_Stats player_stats;
    MainMenu mainMenu(&drawer, &game_textures, &ui_textures);
    mainMenu.start();
    if (mainMenu.resultGameType() == SINGLE)
        player_stats = singleDuckGame();
    else if (mainMenu.resultGameType() == DOUBLE)
        player_stats = doubleDuckGame();
    else
        int iii = 0; // TODO: Quit
    std::vector<Duck> ducks = {};
    DuckHatchery hatchery(&game_textures, &drawer);


    bool quit = false;
    IntroCutScene introCutScene(&drawer, background, foreground, &player_stats, &game_textures, &ui_textures);
    quit = introCutScene.start();

    trySpawnDuck(&hatchery, &ducks, &player_stats);

    NOW = SDL_GetPerformanceCounter();
    SDL_Event e;
    while (!quit) {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = ((NOW - LAST)*1000 / (double)SDL_GetPerformanceFrequency());

        // Handle user input
        while (SDL_PollEvent(&e) != 0){
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (player_stats.shots_left > 0) {
                    player_stats.shots_left -= 1;

                    // See if duck was hit
                    int mX, mY;
                    SDL_GetMouseState(&mX, &mY);
                    for (auto &duck : ducks) {
                        int wX = mX, wY = mY;
                        drawer.screenPointToWorldPoint(&wX, &wY);
                        if (duck.alive && wX > duck.x && wX < duck.x + duck.width() && wY > duck.y && wY < duck.y + duck.height()) {
                            killDuck(&duck, &player_stats);
                            if (player_stats.duck_next == 10)
                                startNewRound(&player_stats);
                            break;
                        }
                    }
                }
            }
        }

        // Game Object updates
        for (auto &duck : ducks)
            duck.update(deltaTime);

        auto i = std::begin(ducks);
        while (i != ducks.end()) {
            i->update(deltaTime);
            if (i->y > hatchery.spawnY) {
                // Show success cut scene
                if (ducks.size() == 1) {
                    SuccessCutScene successCutScene(&drawer, &player_stats, &game_textures, &ui_textures, background,
                                                    foreground, &(*i));
                    if (successCutScene.start())
                        quit = true;
                    NOW = SDL_GetPerformanceCounter(); // TODO: Not very accurate
                }
                i = ducks.erase(i);
                // Launch new ducks
                if (trySpawnDuck(&hatchery, &ducks, &player_stats))
                    player_stats.shots_left = 3;
            }
            else
                i++;
        }

        // Render
        SDL_RenderClear(renderer); // Flush buffer

        drawer.renderTexture(background, 0, 0);

        for (auto &duck : ducks)
            duck.render(&drawer, deltaTime);

        drawer.renderTexture(foreground, 0, 0);

        for (auto &duck : ducks)
            if (duck.isFalling())
                duck.renderScore(&drawer);

        drawer.renderUI(deltaTime, &ui_textures, &player_stats);

        SDL_RenderPresent(renderer); // Update screen
    }

    cleanup(background, foreground, &ui_textures, &game_textures, renderer, window);
    SDL_Quit();
}