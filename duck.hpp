#ifndef DUCKHUNT_DUCK_HPP
#define DUCKHUNT_DUCK_HPP

#include <random>
#include <array>
#include <utility>
#include "textures.hpp"
#include "timer.hpp"
#include "drawing.hpp"
#include "animation.hpp"

enum DuckColours { NO_COLOUR, BLUE, BROWN, RED };
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
    SDL_Texture* scoreTexture;
    SDL_Rect scoreFrame;

    // animation
    /// True when the duck is first out of the bush it spawns in.
    bool isFreeOfBush;
    bool stayOnScreen;
    Timer deadTimer;
    Animation* current = nullptr;
    Animation dead;
    Animation falling;
    Animation flyDiagonal;
    Animation flyHorizontal;
    Animation flyVertical;

    double scaledLeftBoundary;
    double scaledRightBoundary;

public:
    Duck(int index, DuckColours colour, int spawn_x, int spawn_y, double speed, int score, int framesPerSecond,
         Animation dead, Animation falling, Animation flyDiagonal, Animation flyHorizontal, Animation flyVertical,
         double scaledLeftBoundary, double scaledRightBoundary, SDL_Texture* scoreTexture, SDL_Rect scoreFrame,
         std::mt19937* mt) : dead(std::move(dead)), falling(std::move(falling)), flyDiagonal(std::move(flyDiagonal)),
                             flyHorizontal(std::move(flyHorizontal)), flyVertical(std::move(flyVertical)),
                             scoreFrame(scoreFrame), deadTimer(500.0) {
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
        stayOnScreen = true;
        isFreeOfBush = false;
        deadTimer.disable();
        this->dead.reset(framesPerSecond);
        this->falling.reset(framesPerSecond);
        this->flyDiagonal.reset(framesPerSecond);
        this->flyHorizontal.reset(framesPerSecond);
        this->flyVertical.reset(framesPerSecond);
        this->scaledLeftBoundary = scaledLeftBoundary;
        this->scaledRightBoundary = scaledRightBoundary;
        alive = true;
        this->scoreTexture = scoreTexture;
        std::cout << "Spawning a new duck" << std::endl;
    }

    void update(double deltaTime) {
        double new_x, new_y;
        double newAngle = std::numeric_limits<double>::infinity();

        new_x = x + std::cos(angle) * speed * deltaTime;
        new_y = y - std::sin(angle) * speed * deltaTime;

        // Bounce duck on screen edge
        if (alive) {
            if (stayOnScreen && new_y < 0) // top
                newAngle = randAngle(7.0 * pi / 6.0, 11.0 * pi / 6.0);
            if (new_y > 155 - dead.frameHeight()) { // bottom
                if (isFreeOfBush)
                    newAngle = randAngle(pi / 4.0, 3.0 * pi / 4.0);
            }
            else
                isFreeOfBush = true;
            if (stayOnScreen && new_x < scaledLeftBoundary) // left
                newAngle = std::fmod(randAngle(5.0 * pi / 6.0, 18.0 * pi / 6.0), 2.0 * pi);
            if (stayOnScreen && new_x > scaledRightBoundary) // right
                newAngle = randAngle(2.0 * pi / 3.0, 4.0 * pi / 3.0);

            // Apply new angle or position
            if (!std::isinf(newAngle)) {
                angle = newAngle;
                // Set duck's animation according to its angle
                new_x = std::abs(std::cos(angle));
                new_y = std::abs(std::sin(angle));
                if (new_x > new_y)
                    current = &flyHorizontal;
                else
                    current = &flyDiagonal;
            }
        }
        // Move duck
        if (std::isinf(newAngle)) {
            x = new_x;
            y = new_y;
        }
    }

    void render(Drawer* drawer, double deltaTime) {
        // TODO: I can't initialise current in the constructor?
        if (current == nullptr)
            current = &this->flyDiagonal;

        // Select next frame
        if (deadTimer.tick(deltaTime)) {
            current = &falling;
            deadTimer.disable();
            speed = 0.05;
        }

        // Flip texture if going left
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (std::cos(angle) < 0.0)
            flip = SDL_FLIP_HORIZONTAL;

        drawer->renderTexture(current->texture, static_cast<int>(x), static_cast<int>(y), current->advance(deltaTime), 0.0, nullptr, flip);
    }

    void renderScore(Drawer* drawer) {
        drawer->renderTexture(scoreTexture, xDied, yDied, &scoreFrame);
    }

    int kill() {
        alive = false;
        deadTimer.enable();
        current = &dead;
        speed = 0.0;
        angle = 3.0 * pi / 2.0;
        xDied = static_cast<int>(x);
        yDied = static_cast<int>(y);
        return score;
    }

    void flyUp() {
        stayOnScreen = false;
        angle = pi / 2.0;
        current = &flyVertical;
    }

    bool isOnScreen() {
        if (y < 0 - height())
            return false;
        else if (x < scaledLeftBoundary)
            return false;
        else if (x > scaledRightBoundary)
            return false;
        return true;
    }

    /// Checks whether the duck has died and is falling back to the ground.
    /// \return true if falling, false otherwise.
    bool isFalling() {
        return !alive && current == &falling;
    }

    int width() {
        return dead.frameWidth();
    }

    int height() {
        return dead.frameHeight();
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
    Animation blueDead;
    Animation blueFalling;
    Animation blueFlyingDiagonal;
    Animation blueFlyingHorizontal;
    Animation blueFlyingVertical;
    Animation brownDead;
    Animation brownFalling;
    Animation brownFlyingDiagonal;
    Animation brownFlyingHorizontal;
    Animation brownFlyingVertical;
    Animation redDead;
    Animation redFalling;
    Animation redFlyingDiagonal;
    Animation redFlyingHorizontal;
    Animation redFlyingVertical;
    std::vector<SDL_Rect> duckScoreFrames;
    SDL_Texture* duckScoreTexture;

    double scaledLeftBoundary;
    double scaledRightBoundary;

public:
    DuckHatchery(Textures* textures, Drawer* drawer)
        : blueDead(textures->duck_blue_dead, 1), blueFalling(textures->duck_blue_falling, 4),
          blueFlyingDiagonal(textures->duck_blue_diagonal, 3), blueFlyingHorizontal(textures->duck_blue_horizontal, 3),
          blueFlyingVertical(textures->duck_blue_vertical, 3),
          brownDead(textures->duck_brown_dead, 1), brownFalling(textures->duck_brown_falling, 4),
          brownFlyingDiagonal(textures->duck_brown_diagonal, 3), brownFlyingHorizontal(textures->duck_brown_horizontal, 3),
          brownFlyingVertical(textures->duck_brown_vertical, 3),
          redDead(textures->duck_red_dead, 1), redFalling(textures->duck_red_falling, 4),
          redFlyingDiagonal(textures->duck_red_diagonal, 3), redFlyingHorizontal(textures->duck_red_horizontal, 3),
          redFlyingVertical(textures->duck_red_vertical, 3) {
        mt = std::mt19937(rd());
        duckScoreTexture = textures->duck_score;
        duckScoreFrames = spriteStripRects(duckScoreTexture, 8);

        scaledLeftBoundary = -drawer->x_offset / drawer->scale;
        scaledRightBoundary = drawer->window_width / drawer->scale + scaledLeftBoundary - blueDead.frameWidth();
    }

    Duck newDuck(DuckColours duck_colour, int score, int round, int duckIndex) {
        Animation* dead;
        Animation* falling;
        Animation* flyDiagonal;
        Animation* flyHorizontal;
        Animation* flyVertical;
        SDL_Rect* scoreFrame;
        switch (duck_colour) {
            case BLUE:
                dead = &blueDead;
                falling = &blueFalling;
                flyDiagonal = &blueFlyingDiagonal;
                flyHorizontal = &blueFlyingHorizontal;
                flyVertical = &blueFlyingVertical;
                break;
            case RED:
                dead = &redDead;
                falling = &redFalling;
                flyDiagonal = &redFlyingDiagonal;
                flyHorizontal = &redFlyingHorizontal;
                flyVertical = &redFlyingVertical;
                break;
            default:
                dead = &brownDead;
                falling = &brownFalling;
                flyDiagonal = &brownFlyingDiagonal;
                flyHorizontal = &brownFlyingHorizontal;
                flyVertical = &brownFlyingVertical;
                break;
        }
        switch (score) {
            default:
                scoreFrame = &duckScoreFrames[0];
                break;
            case 800:
                scoreFrame = &duckScoreFrames[1];
                break;
            case 1000:
                scoreFrame = &duckScoreFrames[2];
                break;
            case 1500:
                scoreFrame = &duckScoreFrames[3];
                break;
            case 1600:
                scoreFrame = &duckScoreFrames[4];
                break;
            case 2000:
                scoreFrame = &duckScoreFrames[5];
                break;
            case 2400:
                scoreFrame = &duckScoreFrames[6];
                break;
            case 3000:
                scoreFrame = &duckScoreFrames[7];
                break;
        }

        double speed = 0.05 + 0.01 * round;
        std::uniform_int_distribution<int> dist(spawnXLow, spawnXHigh);
        int spawn_x = dist(mt);
        int spawn_y = spawnY;
        return {duckIndex, duck_colour, spawn_x, spawn_y, speed, score, 10 + round, *dead, *falling, *flyDiagonal,
            *flyHorizontal, *flyVertical, scaledLeftBoundary, scaledRightBoundary, duckScoreTexture, *scoreFrame, &mt};
    }
};

#endif //DUCKHUNT_DUCK_HPP
