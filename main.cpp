#include <iostream>
#include "cleanup.hpp"
#include "dog.hpp"
#include "level.hpp"

//const int SCREEN_WIDTH  = 960;
//const int SCREEN_HEIGHT = 540;
const int SCREEN_WIDTH  = 256 * 3;
const int SCREEN_HEIGHT = 224 * 3;

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

    Textures textures = loadTexturesRemake(renderer);
    if (!validateTextures(&textures)) {
        cleanup(&textures, renderer, window);
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    Drawer drawer(textures.background, renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    MainMenu mainMenu(&drawer, &textures);
    quit = mainMenu.start();

    Player_Stats player_stats;
    if (!quit) {
        if (mainMenu.resultGameType() == SINGLE)
            player_stats = Level::singleDuckGame();
        else if (mainMenu.resultGameType() == DOUBLE)
            player_stats = Level::doubleDuckGame();
        else
            quit = true;
    }

    if (!quit)
        quit = IntroCutScene(&drawer, &player_stats, &textures).start();

    if (!quit)
        SinglePlayerGame(&drawer, &player_stats, &textures).start();

    cleanup(&textures, renderer, window);
    SDL_Quit();
}