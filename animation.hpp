#ifndef DUCKHUNT_ANIMATION_HPP
#define DUCKHUNT_ANIMATION_HPP

#include <algorithm>
#include <vector>
#include "SDL2/SDL.h"
#include "timer.hpp"
#include "textures.hpp"

class Animation {
private:
    int currentFrame;
    Timer timer;
    std::vector<SDL_Rect> frames;
public:
    Animation(SDL_Texture* texture, unsigned int numberOfFrames, int framesPerSecond) : timer(1000.0 / framesPerSecond) {
        auto framesArray = spriteStripRects<numberOfFrames>(texture);
        frames.reserve(numberOfFrames);
        std::copy_n(std::make_move_iterator(frames.begin()), numberOfFrames, framesArray.begin());
        currentFrame = 0;
    }

    /// Advances the animation and returns the current frame.
    /// \param deltaTime the time since the last frame.
    /// \return the rect for the current frame.
    const SDL_Rect* advance(double deltaTime) {
        if (timer.tick(deltaTime))
            currentFrame = ++currentFrame % frames.size();
        return &frames[currentFrame];
    }
};

#endif //DUCKHUNT_ANIMATION_HPP
