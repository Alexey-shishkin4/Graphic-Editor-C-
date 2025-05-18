#pragma once
#include <vector>
#include <string>
#include "types.h"

struct Layer {
    std::vector<Rect> rects;
    std::vector<BrushStroke> strokes;
    std::vector<Drawable*> objects;
    int canvasWidth = 0;
    int canvasHeight = 0;
    bool visible = true;
    std::string name;

    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;
    bool surfFlag = false;

    Layer() = default;
    Layer(const Layer&) = default;
    Layer& operator=(const Layer&) = default;

    ~Layer() {
        if (surface) SDL_DestroySurface(surface);
        if (texture) SDL_DestroyTexture(texture);
    }
};
