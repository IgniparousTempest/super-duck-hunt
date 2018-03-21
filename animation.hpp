#ifndef DUCKHUNT_ANIMATION_HPP
#define DUCKHUNT_ANIMATION_HPP

#include <algorithm>
#include <vector>
#include "SDL2/SDL.h"
#include "timer.hpp"
#include "textures.hpp"

class Animation {
private:
    long currentFrame;
    Timer timer;
    std::vector<SDL_Rect> frames;
public:
    SDL_Texture* texture;
public:
    Animation(SDL_Texture* texture, size_t numberOfFrames, int framesPerSecond = 1) : timer(1000.0 / framesPerSecond) {
        this->texture = texture;
        frames = spriteStripRects(texture, numberOfFrames);
        currentFrame = 0;
    }

    /// Advances the animation and returns the current frame.
    /// \param deltaTime the time since the last frame.
    /// \return the rect for the current frame.
    const SDL_Rect* advance(double deltaTime) {
        if (timer.tick(deltaTime))
            currentFrame = ++currentFrame % frames.size();
        return frame();
    }

    void reset(int framesPerSecond) {
        timer.reset(1000.0 / framesPerSecond);
        currentFrame = 0;
    }

    const SDL_Rect* frame() {
        return &frames[currentFrame];
    }

    int frameWidth() {
        return frames.at(0).w;
    }

    int frameHeight() {
        return frames.at(0).h;
    }
};

#endif //DUCKHUNT_ANIMATION_HPP
