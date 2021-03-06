#ifndef DUCKHUNT_DOG_HPP
#define DUCKHUNT_DOG_HPP

#include <SDL2/SDL.h>
#include "drawing.hpp"
#include "duck.hpp"
#include "animation.hpp"

class DogSniffing {
private:
    SDL_Texture* tex_sniffing;
    std::vector<SDL_Rect> frames;
    int currentFrame;
    double frameTime;
    double frameLength;
    std::vector<int> sniffingAnimation = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 0, 4, 4, 0, 0, 4, 4, 0, 0, 4, 4, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 0, 4, 4, 0, 0, 4, 4, 0, 0, 4, 4, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 0, 4, 4, 0, 0, 4, 4, 0, 0, 4, 4, 5, 5, 5};
    std::vector<int> sniffingMovement  = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
public:
    int x;
    int y;

public:
    DogSniffing(SDL_Texture* tex_sniffing, int framesPerSecond) {
        this->tex_sniffing = tex_sniffing;

        // Get rect of each frame
        frames = spriteStripRects(tex_sniffing, 6);

        x = 88;
        y = 145;
        currentFrame = 1;
        frameTime = 0.0;
        frameLength = 1000.0 / framesPerSecond;
    }

    bool render(Drawer* drawer, double deltaTime) {
        frameTime += deltaTime;
        if (frameTime > frameLength) {
            frameTime = 0.0;
            currentFrame++;
            if (currentFrame < sniffingAnimation.size())
                x += sniffingMovement[currentFrame];
        }
        if (currentFrame < sniffingAnimation.size())
            drawer->renderTexture(tex_sniffing, x, y, &frames[sniffingAnimation[currentFrame]]);
        else
            return true;
        return false;
    }
};

class DogJumping {
private:
    SDL_Texture* tex_jumping;
    SDL_Rect frames[2];
    int currentFrame;
    double frameTime;
    double frameLength;
public:
    int x;
    int y;

public:
    DogJumping(SDL_Texture* tex_sniffing, int framesPerSecond) {
        this->tex_jumping = tex_sniffing;

        // Get rect of each frame
        int w, h;
        SDL_QueryTexture(tex_sniffing, nullptr, nullptr, &w, &h);
        w /= 2;
        for (int i = 0; i < 2; ++i) {
            frames[i].x = w * i;
            frames[i].y = 0;
            frames[i].w = w;
            frames[i].h = h;
        }

        currentFrame = 1;
        frameTime = 0.0;
        frameLength = 1000.0 / framesPerSecond;
    }

    bool renderJump(Drawer* drawer, double deltaTime) {
        frameTime += deltaTime;
        if (frameTime > frameLength) {
            frameTime = 0.0;
            currentFrame++;
            if (currentFrame < 45) {
                x += 1;
                y -= 1;
            }
            else
                return true;
        }
        drawer->renderTexture(tex_jumping, x, y, &frames[0]);
        return false;
    }

    bool renderFall(Drawer* drawer, double deltaTime) {
        frameTime += deltaTime;
        if (frameTime > frameLength) {
            frameTime = 0.0;
            currentFrame++;
            if (currentFrame < 90) {
                x += 1;
                y += 1;
            }
            else
                return true;
        }
        drawer->renderTexture(tex_jumping, x, y, &frames[1]);
        return false;
    }
};

enum DogSuccessState {
    UP, STOPPED, DOWN
};

class DogSuccess {
private:
    int x;
    double y;
    double speed;
    SDL_Texture* texture;
    SDL_Rect frame;
    DogSuccessState state;
    Timer timer = Timer(0);
    int yTopLimit;
    int yBottomLimit;

public:
    DogSuccess(int x, int yBottom, int yTop, SDL_Texture* texture_success, DuckColours colour, double speed) : DogSuccess(x, yBottom, yTop, texture_success, speed) {
        auto frames = spriteStripRects(texture_success, 12);
        switch (colour) {
            case BLUE:
                frame = frames[1];
                break;
            case RED:
                frame = frames[2];
                break;
            default:
                frame = frames[0];
                break;
        }
    }

    DogSuccess(int x, int yBottom, int yTop, SDL_Texture* texture_success, DuckColours colour1, DuckColours colour2, double speed) : DogSuccess(x, yBottom, yTop, texture_success, speed) {
        int index;
        switch (colour1) {
            case BLUE:
                index = 6;
                break;
            case RED:
                index = 9;
                break;
            default:
                index = 3;
                break;
        }
        switch (colour2) {
            case BLUE:
                index += 1;
                break;
            case RED:
                index += 2;
                break;
            default:
                index += 0;
                break;
        }
        frame = spriteStripRects(texture_success, 12)[index];
    }

    ///
    /// \param drawer
    /// \param deltaTime
    /// \return true if animation is complete, false otherwise.
    bool render(Drawer* drawer, double deltaTime) {
        if (state == UP)
            y += -speed * deltaTime;
        else if (state == DOWN)
            y += speed * deltaTime;

        drawer->renderTexture(texture, x, static_cast<int>(y), &frame);

        if (state == UP && y < yTopLimit) {
            state = STOPPED;
            timer.reset(200.0);
        }
        else if (state == STOPPED && timer.tick(deltaTime))
            state = DOWN;
        else if (state == DOWN && y > yBottomLimit)
            return true;
        return false;
    }

private:
    DogSuccess(int x, int yBottom, int yTop, SDL_Texture* texture_success, double speed) {
        this->x = x;
        y = yBottom;
        yBottomLimit = yBottom;
        yTopLimit = yTop;
        texture = texture_success;
        this->speed = speed;
        state = UP;
    }
};

class DogFailure {
private:
    int x;
    double y;
    double speed;
    DogSuccessState state;
    Timer timer = Timer(0);
    Animation animation;
    int yTopLimit;
    int yBottomLimit;

public:
    DogFailure(int x, int yBottom, int yTop, SDL_Texture* texture_failure, double speed, int framePerSecond)
        : animation(texture_failure, 2, framePerSecond) {
        this->x = x;
        y = yBottom;
        yBottomLimit = yBottom;
        yTopLimit = yTop;
        this->speed = speed;
        state = UP;
    }

    ///
    /// \param drawer
    /// \param deltaTime
    /// \return true if animation is complete, false otherwise.
    bool render(Drawer* drawer, double deltaTime) {
        if (state == UP)
            y += -speed * deltaTime * 0.5;
        else if (state == DOWN)
            y += speed * deltaTime;

        drawer->renderTexture(animation.texture, x, static_cast<int>(y), animation.advance(deltaTime));

        if (state == UP && y < yTopLimit) {
            state = STOPPED;
            timer.reset(200.0);
        }
        else if (state == STOPPED && timer.tick(deltaTime))
            state = DOWN;
        else if (state == DOWN && y > yBottomLimit)
            return true;
        return false;
    }
};

class DogGameOver {
private:
    int x;
    double y;
    double speed;
    DogSuccessState state;
    Timer timer = Timer(0);
    Animation animation;
    int yTopLimit;

public:
    DogGameOver(int x, int yBottom, int yTop, SDL_Texture* texture_failure, double speed, int framePerSecond)
        : animation(texture_failure, 2, framePerSecond) {
        this->x = x;
        y = yBottom;
        yTopLimit = yTop;
        this->speed = speed;
        state = UP;
    }

    bool render(Drawer* drawer, double deltaTime) {
        if (state == UP)
            y += -speed * deltaTime * 0.5;
        else if (state == DOWN)
            y += speed * deltaTime;

        drawer->renderTexture(animation.texture, x, static_cast<int>(y), animation.advance(deltaTime));

        if (state == UP && y < yTopLimit) {
            state = STOPPED;
            timer.reset(4000.0);
        }
        else if (state == STOPPED && timer.tick(deltaTime))
            return true;
        return false;
    }
};

#endif //DUCKHUNT_DOG_HPP
