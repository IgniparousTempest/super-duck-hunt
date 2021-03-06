#ifndef DUCKHUNT_ERRORS_HPP
#define DUCKHUNT_ERRORS_HPP

#include <iostream>
#include "SDL2/SDL.h"

/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
    os << msg << " error: " << SDL_GetError() << std::endl;
}

/// This exception indicates that the game was quit.
struct QuitTrigger : public std::exception
{
    const char* what () const throw () override {
        return "A quit event was triggered";
    }
};

#endif //DUCKHUNT_ERRORS_HPP
