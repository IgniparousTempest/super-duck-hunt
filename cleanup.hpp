#ifndef DUCKHUNT_CLEANUP_HPP
#define DUCKHUNT_CLEANUP_HPP

#include "SDL2/SDL.h"
#include <SDL2/SDL_image.h>
#include "textures.hpp"

void cleanup_ui(UI_Textures* ui_textures) {
    SDL_DestroyTexture(ui_textures->bullet);
    SDL_DestroyTexture(ui_textures->duck_lit);
    SDL_DestroyTexture(ui_textures->duck_white);
    SDL_DestroyTexture(ui_textures->ducks_needed_bar);
    SDL_DestroyTexture(ui_textures->hit);
    SDL_DestroyTexture(ui_textures->numbers_green);
    SDL_DestroyTexture(ui_textures->numbers_white);
    SDL_DestroyTexture(ui_textures->score);
    SDL_DestroyTexture(ui_textures->round);
    SDL_DestroyTexture(ui_textures->shot);
}

void cleanup_game(Game_Textures* game_textures) {
    SDL_DestroyTexture(game_textures->dog_jumping);
    SDL_DestroyTexture(game_textures->dog_sniffing);
    SDL_DestroyTexture(game_textures->dog_success);
    SDL_DestroyTexture(game_textures->duck_blue);
    SDL_DestroyTexture(game_textures->duck_brown);
    SDL_DestroyTexture(game_textures->duck_red);
    SDL_DestroyTexture(game_textures->duck_score);
    SDL_DestroyTexture(game_textures->main_menu_background);
}

void cleanup(SDL_Window *window) {
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void cleanup(SDL_Texture *background, SDL_Texture *foreground, UI_Textures* ui_textures, Game_Textures* game_textures, SDL_Renderer *renderer, SDL_Window *window) {
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(foreground);
    cleanup_ui(ui_textures);
    cleanup_game(game_textures);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

#endif //DUCKHUNT_CLEANUP_HPP
