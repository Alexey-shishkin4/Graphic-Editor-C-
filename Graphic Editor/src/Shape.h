#pragma once
#include <SDL2/SDL.h>

class Shape {
public:
    virtual ~Shape() {}
    virtual void draw(SDL_Renderer* renderer) = 0;
};
