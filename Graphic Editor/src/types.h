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

class Rect {
    public:
        float x, y, w, h;
    
        // Конструктор, принимающий значения x, y, w, h
        Rect(float x, float y, float w, float h)
            : x(x), y(y), w(w), h(h) {}
    
        // Можно добавить другие методы, если нужно
};

struct Circle {
    float x, y, radius;
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