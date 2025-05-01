#pragma once
#include <vector>
#include <SDL3/SDL.h>

enum class Tool {
    None,
    Select,
    Move,
    Erase,
    Brush
};

struct Rect {
    float x, y, w, h;
};

struct Stroke {
    std::vector<SDL_FPoint> points;
    SDL_Color color;
    float thickness;
};

enum class ActionType {
    AddRect,
    RemoveRect,
    ToggleVisibility,
    ChangeActiveLayer,
    MoveLayerUp,
    MoveLayerDown,
    StrokeDrawn
};