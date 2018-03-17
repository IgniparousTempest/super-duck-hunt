#ifndef DUCKHUNT_DUCK_HPP
#define DUCKHUNT_DUCK_HPP

#include <random>
#include <array>
#include "textures.hpp"
#include "timer.hpp"
#include "drawing.hpp"

enum DuckColours { blue, brown, red };
const double pi = std::acos(-1);

class Duck {
public:
    int index;
    double x;
    double y;
    bool alive;
    DuckColours colour;
private:
    std::mt19937* mt;
    int xDied;
    int yDied;
    int score;
    double angle;
    double speed;
    SDL_Texture* texture;
    std::array<SDL_Rect, 11> frames;
    SDL_Texture* scoreTexture;
    SDL_Rect scoreFrame;

    // animation
    /// True when the duck is first out of the bush it spawns in.
    bool isFreeOfBush;
    int currentFrame;
    Timer animTimer;

    double scaledLeftBoundary;
    double scaledRightBoundary;

public:
    Duck(int index, DuckColours colour, int spawn_x, int spawn_y, double speed, int score, SDL_Texture* texture,
         std::array<SDL_Rect, 11> frames, int framesPerSecond, double scaledLeftBoundary, double scaledRightBoundary,
         SDL_Texture* scoreTexture, SDL_Rect scoreFrame, std::mt19937* mt) :
        animTimer(1000.0 / framesPerSecond) {
        this->mt = mt;
        this->index = index;
        this->colour = colour;
        x = spawn_x;
        y = spawn_y;
        xDied = 0;
        yDied = 0;
        this->speed = speed;
        angle = randAngle(pi / 4.0, 3.0 * pi / 4.0);
        this->score = score;
        this->texture = texture;
        this->frames = frames;
        isFreeOfBush = false;
        currentFrame = 3;
        this->scaledLeftBoundary = scaledLeftBoundary;
        this->scaledRightBoundary = scaledRightBoundary;
        alive = true;
        this->scoreTexture = scoreTexture;
        this->scoreFrame = scoreFrame;
        std::cout << "Spawning a new duck" << std::endl;
    }

    void update(double deltaTime) {
        double new_x, new_y;
        double newAngle = std::numeric_limits<double>::infinity();

        new_x = x + std::cos(angle) * speed * deltaTime;
        new_y = y - std::sin(angle) * speed * deltaTime;

        // Bounce duck on screen edge
        if (alive) {
            if (new_y < 0) // top
                newAngle = randAngle(7.0 * pi / 6.0, 11.0 * pi / 6.0);
            if (new_y > 155 - frames[0].h) { // bottom
                if (isFreeOfBush)
                    newAngle = randAngle(pi / 4.0, 3.0 * pi / 4.0);
            }
            else
                isFreeOfBush = true;
            if (new_x < scaledLeftBoundary) // left
                newAngle = std::fmod(randAngle(5.0 * pi / 6.0, 18.0 * pi / 6.0), 2.0 * pi);
            if (new_x > scaledRightBoundary) // right
                newAngle = randAngle(2.0 * pi / 3.0, 4.0 * pi / 3.0);

            // Apply new angle or position
            if (!std::isinf(newAngle)) {
                angle = newAngle;
                // Set duck's animation according to its angle
                new_x = std::abs(std::cos(angle));
                new_y = std::abs(std::sin(angle));
                if (new_x > new_y)
                    currentFrame = 0;
                else
                    currentFrame = 3;
            }
        }
        // Move duck
        if (std::isinf(newAngle)) {
            x = new_x;
            y = new_y;
        }
    }

    void render(Drawer* drawer, double deltaTime) {
        // Select next frame
        if (animTimer.tick(deltaTime)) {
            if (currentFrame < 3)
                currentFrame = (currentFrame + 1) % 3;
            else if (currentFrame < 6)
                currentFrame = (currentFrame - 3 + 1) % 3 + 3;
            else if (currentFrame == 6) {
                animTimer.reset(100.0);
                currentFrame = 7;
                speed = 0.05;
            }
            else if (currentFrame < 11)
                currentFrame = (currentFrame - 7 + 1) % 4 + 7;
        }

        // Flip texture if going left
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (std::cos(angle) < 0.0)
            flip = SDL_FLIP_HORIZONTAL;

        drawer->renderTexture(texture, static_cast<int>(x), static_cast<int>(y), &frames[currentFrame], 0.0, nullptr, flip);
    }

    void renderScore(Drawer* drawer) {
        drawer->renderTexture(scoreTexture, xDied, yDied, &scoreFrame);
    }

    int kill() {
        alive = false;
        currentFrame = 6;
        animTimer.reset(500.0);
        speed = 0.0;
        angle = 3.0 * pi / 2.0;
        xDied = static_cast<int>(x);
        yDied = static_cast<int>(y);
        return score;
    }

    /// Checks whether the duck has died and is falling back to the ground.
    /// \return true if falling, false otherwise.
    bool isFalling() {
        return !alive && currentFrame > 6 && currentFrame < 11;
    }

    int width() {
        return frames[0].w;
    }

    int height() {
        return frames[0].h;
    }

private:
    double randAngle(double min, double max)
    {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(*mt);
    }
};

/// A duck factory ;)
class DuckHatchery {
public:
    const int spawnY = 155;
    const int spawnXLow = 133;
    const int spawnXHigh = 272;
private:
    std::random_device rd;
    std::mt19937 mt;
    std::array<SDL_Rect, 11> blueDuckFrames;
    std::array<SDL_Rect, 11> brownDuckFrames;
    std::array<SDL_Rect, 11> redDuckFrames;
    std::array<SDL_Rect, 8> duckScoreFrames;
    SDL_Texture* blueDuckTexture;
    SDL_Texture* brownDuckTexture;
    SDL_Texture* redDuckTexture;
    SDL_Texture* duckScoreTexture;

    double scaledLeftBoundary;
    double scaledRightBoundary;

public:
    DuckHatchery(Game_Textures* game_textures, Drawer* drawer) {
        mt = std::mt19937(rd());
        blueDuckTexture = game_textures->duck_blue;
        brownDuckTexture = game_textures->duck_brown;
        redDuckTexture = game_textures->duck_red;
        duckScoreTexture = game_textures->duck_score;
        blueDuckFrames = spriteStripRects<11>(blueDuckTexture);
        brownDuckFrames = spriteStripRects<11>(brownDuckTexture);
        redDuckFrames = spriteStripRects<11>(redDuckTexture);
        duckScoreFrames = spriteStripRects<8>(duckScoreTexture);

        scaledLeftBoundary = -drawer->x_offset / drawer->scale;
        scaledRightBoundary = drawer->window_width / drawer->scale + scaledLeftBoundary - blueDuckFrames[0].w;
    }

    Duck newDuck(DuckColours duck_colour, int score, int round, int duckIndex) {
        SDL_Texture* texture;
        std::array<SDL_Rect, 11>* frames;
        SDL_Rect* scoreFrame;
        switch (duck_colour) {
            case blue:
                scoreFrame = &duckScoreFrames[2];
                texture = blueDuckTexture;
                frames = &blueDuckFrames;
                break;
            case red:
                scoreFrame = &duckScoreFrames[3];
                texture = redDuckTexture;
                frames = &redDuckFrames;
                break;
            default:
                scoreFrame = &duckScoreFrames[0];
                texture = brownDuckTexture;
                frames = &brownDuckFrames;
                break;
        }
        double speed = 0.05 + 0.01 * round;
        std::uniform_int_distribution<int> dist(spawnXLow, spawnXHigh);
        int spawn_x = dist(mt);
        int spawn_y = spawnY;
        return {duckIndex, duck_colour, spawn_x, spawn_y, speed, score, texture, *frames, 10 + round, scaledLeftBoundary, scaledRightBoundary, duckScoreTexture, *scoreFrame, &mt};
    }
};

#endif //DUCKHUNT_DUCK_HPP
