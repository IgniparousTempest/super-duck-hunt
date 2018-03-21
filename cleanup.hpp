#ifndef DUCKHUNT_CLEANUP_HPP
#define DUCKHUNT_CLEANUP_HPP

#include "SDL2/SDL.h"
#include <SDL2/SDL_image.h>
#include "textures.hpp"

void cleanup_textures(Textures* textures) {
    SDL_DestroyTexture(textures->ui_bullet);
    SDL_DestroyTexture(textures->ui_duck_lit);
    SDL_DestroyTexture(textures->ui_duck_white);
    SDL_DestroyTexture(textures->ui_ducks_needed_bar);
    SDL_DestroyTexture(textures->ui_hit);
    SDL_DestroyTexture(textures->ui_message_fly_away);
    SDL_DestroyTexture(textures->ui_numbers_green);
    SDL_DestroyTexture(textures->ui_numbers_white);
    SDL_DestroyTexture(textures->ui_score);
    SDL_DestroyTexture(textures->ui_shot);
    SDL_DestroyTexture(textures->ui_round);
    SDL_DestroyTexture(textures->background);
    SDL_DestroyTexture(textures->background_fail);
    SDL_DestroyTexture(textures->dog_failure);
    SDL_DestroyTexture(textures->dog_jumping);
    SDL_DestroyTexture(textures->dog_sniffing);
    SDL_DestroyTexture(textures->dog_success);
    SDL_DestroyTexture(textures->duck_blue_dead);
    SDL_DestroyTexture(textures->duck_blue_diagonal);
    SDL_DestroyTexture(textures->duck_blue_falling);
    SDL_DestroyTexture(textures->duck_blue_horizontal);
    SDL_DestroyTexture(textures->duck_blue_vertical);
    SDL_DestroyTexture(textures->duck_brown_dead);
    SDL_DestroyTexture(textures->duck_brown_diagonal);
    SDL_DestroyTexture(textures->duck_brown_falling);
    SDL_DestroyTexture(textures->duck_brown_horizontal);
    SDL_DestroyTexture(textures->duck_brown_vertical);
    SDL_DestroyTexture(textures->duck_red_dead);
    SDL_DestroyTexture(textures->duck_red_diagonal);
    SDL_DestroyTexture(textures->duck_red_falling);
    SDL_DestroyTexture(textures->duck_red_horizontal);
    SDL_DestroyTexture(textures->duck_red_vertical);
    SDL_DestroyTexture(textures->duck_score);
    SDL_DestroyTexture(textures->foreground);
    SDL_DestroyTexture(textures->main_menu_background);
}

void cleanup(SDL_Window *window) {
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void cleanup(Textures* textures, SDL_Renderer *renderer, SDL_Window *window) {
    cleanup_textures(textures);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

#endif //DUCKHUNT_CLEANUP_HPP
