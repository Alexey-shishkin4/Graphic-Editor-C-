#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "Shape.h"
#include "Line.h"
#include "Rectangle.h"
#include "Circle.h"

class Renderer {
public:
    Renderer(SDL_Renderer* renderer);
    ~Renderer();

    void handleEvent(const SDL_Event& event);
    void render();

private:
    SDL_Renderer* sdlRenderer;
    std::vector<Shape*> shapes;
    int startX, startY;
    bool drawing;
};
