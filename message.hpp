#ifndef DUCKHUNT_MESSAGE_HPP
#define DUCKHUNT_MESSAGE_HPP

#include <SDL2/SDL_system.h>
#include "drawing.hpp"

class Message {
protected:
    int x;
    int y;
    SDL_Texture* texture;
    Timer timer = Timer(0);

public:
    Message(int x, int y, double duration, SDL_Texture* texture) {
        this->x = x;
        this->y = y;
        this->texture = texture;
        timer.reset(duration);
    }

    /// Renders the pop up message to the screen.
    /// \param drawer The drawer to render with.
    /// \param deltaTime The time since the last frame.
    /// \return true if the message time is expired, false otherwise
    virtual bool render(Drawer* drawer, double deltaTime) {
        drawer->renderTexture(texture, x, y);
        return timer.tick(deltaTime);
    }
};

class PerfectMessage : public Message {
private:
    std::string score;
    SDL_Texture* numbersTex;
public:
    PerfectMessage(int x, int y, double duration, SDL_Texture* texture, int score, SDL_Texture* numbersTex)
        : Message(x, y, duration, texture) {
        this->score = std::to_string(score);
        std::reverse(this->score.begin(), this->score.end());
        this->numbersTex = numbersTex;
    }

    bool render(Drawer* drawer, double deltaTime) override {
        bool finished = Message::render(drawer, deltaTime);

        for (int i = 0; i < score.size(); ++i)
            drawer->renderCharacter(numbersTex, score[i], x + i * 8, y + 20);

        return finished;
    }
};

#endif //DUCKHUNT_MESSAGE_HPP
