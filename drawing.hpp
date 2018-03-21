#ifndef DUCKHUNT_DRAWING_HPP
#define DUCKHUNT_DRAWING_HPP

#include <algorithm>
#include "errors.hpp"
#include "player_stats.hpp"
#include "timer.hpp"
#include "SDL2/SDL.h"

class Drawer {
private:
    SDL_Renderer *renderer;
    bool isFlickering = false;
    Timer flickerTimer = Timer(500);
public:
    /// The factor to scale textures by to match the window.
    float scale;
    /// The to the left to start drawing textures.
    int x_offset;
    int window_width;


public:
    /// Creates aspect ratio invariant drawing functions.
    /// \param background The background image.
    /// \param ren The renderer we want to draw to.
    /// \param window_width The width of the window.
    /// \param window_height The height of the window.
    Drawer(SDL_Texture *background, SDL_Renderer *ren, const int window_width, const int window_height) {
        this->renderer = ren;
        this->window_width = window_width;

        int w, h;
        SDL_QueryTexture(background, nullptr, nullptr, &w, &h);
        this->scale = static_cast<float>(window_height) / static_cast<float>(h);
        w = static_cast<int>(w * scale);
        this->x_offset = static_cast<int>((static_cast<float>(window_width) - static_cast<float>(w)) / 2.0f);
    }

    SDL_Renderer* getRenderer() {
        return renderer;
    }

    void renderCharacter(SDL_Texture* numbers_texture, char character, int x, int y) {
        // Get width and height of each character
        int w, h;
        SDL_QueryTexture(numbers_texture, nullptr, nullptr, &w, &h);
        w /= 10;

        std::string numbers = "0123456789";
        std::size_t pos = numbers.find(character);
        if (pos == std::string::npos)
            pos = 0;

        SDL_Rect clip = {.x = static_cast<int>(pos * w), .y = 0, .w = w, .h = h};
        renderTexture(numbers_texture, x, y, &clip);
    }

    /// Draws a lit up duck to the UI.
    /// \param ducks_texture The texture to use for the lit ducks.
    /// \param i The number of the duck.
    /// \param x The x coordinate to draw to.
    /// \param y The y coordinate to draw to.
    void renderLitDuck(SDL_Texture* ducks_texture, int i, int x, int y) {
        // Get width and height of each duck
        int w, h;
        SDL_QueryTexture(ducks_texture, nullptr, nullptr, &w, &h);
        w /= 10;

        SDL_Rect clip = {.x = i * w, .y = 0, .w = w, .h = h};
        renderTexture(ducks_texture, x, y, &clip);
    }

    /// Draw an SDL_Texture to the renderer at position x, y with the specified width and height.
    /// \param tex The source texture we want to draw.
    /// \param x The x coordinate to draw to.
    /// \param y The y coordinate to draw to.
    /// \param w The width of the rendered texture in pixels.
    /// \param h The height of the rendered texture in pixels.
    void renderTexture(SDL_Texture *tex, int x, int y, int w, int h, const SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) {
        //Setup the destination rectangle to be at the position we want
        SDL_Rect dst = {.x = x, .y = y, .w = w, .h = h};
        SDL_RenderCopyEx(renderer, tex, clip, &dst, angle, center, flip);
    }

    /// Draw an SDL_Texture to the renderer at position x, y, scaling the texture's width and height accordingly.
    /// \param tex The source texture we want to draw.
    /// \param x The x coordinate to draw to.
    /// \param y The y coordinate to draw to.
    void renderTexture(SDL_Texture *tex, int x, int y, const SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) {
        int w, h;
        //Query the texture to get its width and height to use
        if (clip != nullptr) {
            w = clip->w;
            h = clip->h;
        }
        else
            SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        x = static_cast<int>(x * scale) + x_offset;
        y = static_cast<int>(y * scale);
        w = static_cast<int>(w * scale);
        h = static_cast<int>(h * scale);
        renderTexture(tex, x, y, w, h, clip, angle, center, flip);
    }

    void renderUI(double deltaTime, Textures* textures, Player_Stats *player_stats) {
        // Draw the shots left
        renderTexture(textures->ui_shot, 110, 217);
        if (player_stats->shots_left >= 3)
            renderTexture(textures->ui_bullet, 127, 208);
        if (player_stats->shots_left >= 2)
            renderTexture(textures->ui_bullet, 119, 208);
        if (player_stats->shots_left >= 1)
            renderTexture(textures->ui_bullet, 111, 208);

        // Draw the shots left
        renderTexture(textures->ui_hit, 149, 209);

        // Draw duck icons
        bool flickerTicked = flickerTimer.tick(deltaTime);
        bool isCurrentDuck;
        for (int i = 0; i < 10; ++i) {
            isCurrentDuck = std::find(player_stats->ducks_current.begin(), player_stats->ducks_current.end(), i) != player_stats->ducks_current.end();
            if (player_stats->ducks_hit[i])
                renderLitDuck(textures->ui_duck_lit, i, 181 + i * 8, 210);
            else if (!isCurrentDuck || isFlickering)
                renderTexture(textures->ui_duck_white, 181 + i * 8, 210);
        }
        if (flickerTicked)
            isFlickering = !isFlickering;

        // Draw ducks needed bar
        for (int i = 0; i < player_stats->ducks_needed; ++i)
            renderTexture(textures->ui_ducks_needed_bar, 181 + i * 8, 219);

        // Draw round counter
        std::string round_string = std::to_string(player_stats->round);
        renderTexture(textures->ui_round, 109, 192);
        for (int i = 0; i < round_string.size(); ++i)
            renderCharacter(textures->ui_numbers_green, round_string[i], 124 + i * 8, 192);

        // Draw score
        std::string score_string = std::to_string(player_stats->score);
        if (score_string.size() <= 6)
            score_string.insert(0, 6 - score_string.size(), '0');
        std::reverse(score_string.begin(), score_string.end());
        renderTexture(textures->ui_score, 285, 216);
        for (int i = 0; i < score_string.size(); ++i)
            renderCharacter(textures->ui_numbers_white, score_string[i], 317 - i * 8, 208);

    }

    void screenPointToWorldPoint(int* x, int* y) {
        double screenX = (*x  - x_offset) / scale;
        double screenY = *y / scale;

        *x = static_cast<int>(screenX);
        *y = static_cast<int>(screenY);
    }
};

#endif //DUCKHUNT_DRAWING_HPP
