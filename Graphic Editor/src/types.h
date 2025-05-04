#pragma once
#include "screen_utils.h"
#include <vector>
#include <SDL3/SDL.h>
#include "Drawable.h"
#include <math.h>


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
        std::vector<Rect> rects;
    
        void addCircle(float x, float y, float radius) {
            circles.emplace_back(x, y, radius);
        }

        BrushStroke() = default;
        BrushStroke(const BrushStroke& other) = default;  // Copy constructor
        BrushStroke& operator=(const BrushStroke& other) = default;  // Copy assignment
    
        void draw(SDL_Renderer* renderer, float scale, int offsetX, int offsetY) const override {
            for (const auto& c : circles) {
                SDL_FPoint worldPos = screenToWorld(c.x, c.y, scale, offsetX, offsetY);
    
                SDL_FRect r = {
                    static_cast<float>(worldPos.x - c.radius),
                    static_cast<float>(worldPos.y - c.radius),
                    static_cast<float>(2 * c.radius / scale),
                    static_cast<float>(2 * c.radius / scale)
                };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    
    private:
        void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius) const {
            for (float y = -radius; y <= radius; ++y) {
                float x_max = sqrtf(radius * radius - y * y);  // Максимальное значение x при данной y
                SDL_FRect rect = {
                    cx - x_max,
                    cy + y,
                    2 * x_max,
                    1.0f
                };
                SDL_RenderFillRect(renderer, &rect);
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
    DrawBrushStroke,
};

class DrawableImageBackground : public Drawable {
    public:
        SDL_Texture* texture;
        int width, height;
    
        DrawableImageBackground(SDL_Texture* tex, int w, int h)
            : texture(tex), width(w), height(h) {}
    
        void draw(SDL_Renderer* renderer, float scale, int offsetX, int offsetY) const override {
            SDL_FRect dstRect = {
                offsetX, offsetY,
                width * scale,
                height * scale
            };
            SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
        }
    };