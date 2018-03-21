#ifndef DUCKHUNT_TEXTURES_HPP
#define DUCKHUNT_TEXTURES_HPP

#include "SDL2/SDL.h"
#include <SDL2/SDL_image.h>
#include <array>
#include <vector>
#include <algorithm>
#include "errors.hpp"

struct Textures {
    SDL_Texture* ui_bullet;
    SDL_Texture* ui_duck_lit;
    SDL_Texture* ui_duck_white;
    SDL_Texture* ui_ducks_needed_bar;
    SDL_Texture* ui_hit;
    SDL_Texture* ui_message_fly_away;
    SDL_Texture* ui_message_game_over;
    SDL_Texture* ui_numbers_green;
    SDL_Texture* ui_numbers_white;
    SDL_Texture* ui_score;
    SDL_Texture* ui_shot;
    SDL_Texture* ui_round;
    SDL_Texture* background;
    SDL_Texture* background_fail;
    SDL_Texture* dog_failure;
    SDL_Texture* dog_jumping;
    SDL_Texture* dog_sniffing;
    SDL_Texture* dog_success;
    SDL_Texture* duck_blue_dead;
    SDL_Texture* duck_blue_diagonal;
    SDL_Texture* duck_blue_falling;
    SDL_Texture* duck_blue_horizontal;
    SDL_Texture* duck_blue_vertical;
    SDL_Texture* duck_brown_dead;
    SDL_Texture* duck_brown_diagonal;
    SDL_Texture* duck_brown_falling;
    SDL_Texture* duck_brown_horizontal;
    SDL_Texture* duck_brown_vertical;
    SDL_Texture* duck_red_dead;
    SDL_Texture* duck_red_diagonal;
    SDL_Texture* duck_red_falling;
    SDL_Texture* duck_red_horizontal;
    SDL_Texture* duck_red_vertical;
    SDL_Texture* duck_score;
    SDL_Texture* foreground;
    SDL_Texture* main_menu_background;
};

/**
* Loads an image into a texture on the rendering device
* @param file The image file to load
* @param ren The renderer to load the texture onto
* @return the loaded texture, or nullptr if something went wrong.
*/
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
    //Initialize to nullptr to avoid dangling pointer issues
    SDL_Texture *texture = nullptr;
    //Load the image
    SDL_Surface *loadedImage = IMG_Load(file.c_str());
    //If the loading went ok, convert to texture and return the texture
    if (loadedImage != nullptr){
        texture = SDL_CreateTextureFromSurface(ren, loadedImage);
        SDL_FreeSurface(loadedImage);
        //Make sure converting went ok too
        if (texture == nullptr){
            logSDLError(std::cout, "CreateTextureFromSurface");
        }
    }
    else {
        logSDLError(std::cout, "LoadBMP");
    }
    return texture;
}

/// Determines the rects for each frame on a animation strip, this assumes equal width and no gaps.
/// \param texture The texture to split.
/// \param size the number of frames.
/// \return The frames.
std::vector<SDL_Rect> spriteStripRects(SDL_Texture* texture, size_t size) {
    std::vector<SDL_Rect> frames;
    frames.reserve(size);
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    w /= size;
    for (int i = 0; i < size; ++i) {
        // x, y, w, h
        frames.push_back({w * i, 0, w, h});
    }

    return frames;
}

template <size_t size>
std::array<SDL_Rect, size> spriteStripRects(SDL_Texture* texture) {
    std::array<SDL_Rect, size> frames;

    auto v = spriteStripRects(texture, size);
    std::copy_n(std::make_move_iterator(v.begin()), size, frames.begin());

    return frames;
}

Textures loadTexturesRemake(SDL_Renderer* renderer) {
    return {
        .ui_bullet = loadTexture("textures/ui_bullet.png", renderer),
        .ui_duck_lit = loadTexture("textures/ui_duck_lit.png", renderer),
        .ui_duck_white = loadTexture("textures/ui_duck_white.png", renderer),
        .ui_ducks_needed_bar = loadTexture("textures/ui_ducks_needed_bar.png", renderer),
        .ui_hit = loadTexture("textures/ui_hit.png", renderer),
        .ui_message_fly_away = loadTexture("textures/ui_message_fly_away.png", renderer),
        .ui_message_game_over = loadTexture("textures/ui_message_game_over.png", renderer),
        .ui_numbers_green = loadTexture("textures/ui_numbers_green.png", renderer),
        .ui_numbers_white = loadTexture("textures/ui_numbers_white.png", renderer),
        .ui_score = loadTexture("textures/ui_score.png", renderer),
        .ui_shot = loadTexture("textures/ui_shot.png", renderer),
        .ui_round = loadTexture("textures/ui_round.png", renderer),
        .background = loadTexture("textures/background.png", renderer),
        .background_fail = loadTexture("textures/background_fail.png", renderer),
        .dog_failure = loadTexture("textures/dog_failure.png", renderer),
        .dog_jumping = loadTexture("textures/dog_jumping.png", renderer),
        .dog_sniffing = loadTexture("textures/dog_sniffing.png", renderer),
        .dog_success = loadTexture("textures/dog_success.png", renderer),
        .duck_blue_dead = loadTexture("textures/duck_blue_dead.png", renderer),
        .duck_blue_diagonal = loadTexture("textures/duck_blue_diagonal.png", renderer),
        .duck_blue_falling = loadTexture("textures/duck_blue_falling.png", renderer),
        .duck_blue_horizontal = loadTexture("textures/duck_blue_horizontal.png", renderer),
        .duck_blue_vertical = loadTexture("textures/duck_blue_vertical.png", renderer),
        .duck_brown_dead = loadTexture("textures/duck_brown_dead.png", renderer),
        .duck_brown_diagonal = loadTexture("textures/duck_brown_diagonal.png", renderer),
        .duck_brown_falling = loadTexture("textures/duck_brown_falling.png", renderer),
        .duck_brown_horizontal = loadTexture("textures/duck_brown_horizontal.png", renderer),
        .duck_brown_vertical = loadTexture("textures/duck_brown_vertical.png", renderer),
        .duck_red_dead = loadTexture("textures/duck_red_dead.png", renderer),
        .duck_red_diagonal = loadTexture("textures/duck_red_diagonal.png", renderer),
        .duck_red_falling = loadTexture("textures/duck_red_falling.png", renderer),
        .duck_red_horizontal = loadTexture("textures/duck_red_horizontal.png", renderer),
        .duck_red_vertical = loadTexture("textures/duck_red_vertical.png", renderer),
        .duck_score = loadTexture("textures/duck_score.png", renderer),
        .foreground = loadTexture("textures/foreground.png", renderer),
        .main_menu_background = loadTexture("textures/main_menu_background.png", renderer)
    };
}

bool validateTextures(Textures* textures) {
    return (
        textures->ui_bullet != nullptr ||
        textures->ui_duck_lit != nullptr ||
        textures->ui_duck_white != nullptr ||
        textures->ui_ducks_needed_bar != nullptr ||
        textures->ui_hit != nullptr ||
        textures->ui_message_fly_away != nullptr ||
        textures->ui_message_game_over != nullptr ||
        textures->ui_numbers_green != nullptr ||
        textures->ui_numbers_white != nullptr ||
        textures->ui_score != nullptr ||
        textures->ui_shot != nullptr ||
        textures->ui_round != nullptr ||
        textures->background != nullptr ||
        textures->background_fail != nullptr ||
        textures->dog_failure != nullptr ||
        textures->dog_jumping != nullptr ||
        textures->dog_sniffing != nullptr ||
        textures->dog_success != nullptr ||
        textures->duck_blue_dead != nullptr ||
        textures->duck_blue_diagonal != nullptr ||
        textures->duck_blue_falling != nullptr ||
        textures->duck_blue_horizontal != nullptr ||
        textures->duck_blue_vertical != nullptr ||
        textures->duck_brown_dead != nullptr ||
        textures->duck_brown_diagonal != nullptr ||
        textures->duck_brown_falling != nullptr ||
        textures->duck_brown_horizontal != nullptr ||
        textures->duck_brown_vertical != nullptr ||
        textures->duck_red_dead != nullptr ||
        textures->duck_red_diagonal != nullptr ||
        textures->duck_red_falling != nullptr ||
        textures->duck_red_horizontal != nullptr ||
        textures->duck_red_vertical != nullptr ||
        textures->duck_score != nullptr ||
        textures->foreground != nullptr ||
        textures->main_menu_background != nullptr
    );
};

#endif //DUCKHUNT_TEXTURES_HPP
