#ifndef DUCKHUNT_MESSAGE_HPP
#define DUCKHUNT_MESSAGE_HPP

#include <SDL2/SDL_system.h>
#include "drawing.hpp"

class Message {
protected:
    int x;
    int y;
    SDL_Texture* texture;
    Timer timer;
    bool shouldRender;

public:
    Message(int x, int y, double duration, SDL_Texture* texture) : timer(duration) {
        this->x = x;
        this->y = y;
        this->texture = texture;
        shouldRender = true;
    }

    /// Renders the pop up message to the screen.
    /// \param drawer The drawer to render with.
    /// \param deltaTime The time since the last frame.
    /// \return true if the message time is expired, false otherwise
    virtual void render(Drawer* drawer, double deltaTime) {
        if (shouldRender) {
            drawer->renderTexture(texture, x, y);
            if (timer.tick(deltaTime))
                shouldRender = false;
        }
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

    void render(Drawer* drawer, double deltaTime) override {
        Message::render(drawer, deltaTime);

        if (shouldRender)
            for (int i = 0; i < score.size(); ++i)
                drawer->renderCharacter(numbersTex, score[i], x + i * 8, y + 20);
    }
};

class RoundMessage : public Message {
private:
    std::string round;
    SDL_Texture* numbersTex;
public:
    RoundMessage(int x, int y, double duration, SDL_Texture* texture, int round, SDL_Texture* numbersTex)
        : Message(x, y, duration, texture) {
        this->round = std::to_string(round);
        this->numbersTex = numbersTex;
    }

    void render(Drawer* drawer, double deltaTime) override {
        Message::render(drawer, deltaTime);

        int x_offset = 21 - 4 * static_cast<int>(round.size() - 1);
        if (shouldRender)
            for (int i = 0; i < round.size(); ++i)
                drawer->renderCharacter(numbersTex, round[i], x + x_offset + i * 8, y + 21);
    }
};

#endif //DUCKHUNT_MESSAGE_HPP
