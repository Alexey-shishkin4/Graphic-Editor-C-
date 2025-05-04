#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include "Drawable.h"


enum class Tool {
    None,
    Select,
    Move,
    Erase,
    Brush
};

class Rect : public Drawable {
    public:
        SDL_Rect rect;        // логические координаты (целые)
        SDL_Color color;
    
        Rect(SDL_Rect r, SDL_Color c) : rect(r), color(c) {}
    
        void draw(SDL_Renderer* renderer, float scale, int offsetX, int offsetY) const override {
            SDL_FRect scaledRect = {
                rect.x * scale + offsetX,
                rect.y * scale + offsetY,
                rect.w * scale,
                rect.h * scale
            };
    
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &scaledRect);
        }
    
        bool contains(int x, int y, float scale, int offsetX, int offsetY) const override {
            float scaledX = rect.x * scale + offsetX;
            float scaledY = rect.y * scale + offsetY;
            float scaledW = rect.w * scale;
            float scaledH = rect.h * scale;
    
            return x >= scaledX && x < scaledX + scaledW &&
                   y >= scaledY && y < scaledY + scaledH;
        }
};

struct Circle {
    float x, y, radius;

    Circle(float x_, float y_, float r_) : x(x_), y(y_), radius(r_) {}
};

struct Stroke {
    std::vector<SDL_FPoint> points;
    SDL_Color color;
    float thickness;
};

class BrushStroke : public Drawable {
    public:
        std::vector<Circle> circles;
    
        void addCircle(float x, float y, float radius) {
            circles.emplace_back(x, y, radius);
        }
    
        void draw(SDL_Renderer* renderer, float scale, int offsetX, int offsetY) const override {
            for (const auto& c : circles) {
                int cx = static_cast<int>(c.x * scale + offsetX);
                int cy = static_cast<int>(c.y * scale + offsetY);
                int r = static_cast<int>(c.radius * scale);
                drawCircle(renderer, cx, cy, r);
            }
        }
    
    private:
        void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) const {
            const int diameter = radius * 2;
            for (int w = 0; w < diameter; ++w) {
                for (int h = 0; h < diameter; ++h) {
                    int dx = radius - w;
                    int dy = radius - h;
                    if ((dx * dx + dy * dy) <= (radius * radius)) {
                        SDL_RenderPoint(renderer, cx + dx, cy + dy);
                    }
                }
            }
        }
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