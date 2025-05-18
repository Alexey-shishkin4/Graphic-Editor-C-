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
    Brush,
    Pen
};

class Rect : public Drawable {
    public:
        SDL_Rect rect;        // логические координаты (целые)
        SDL_Color color;
    
        Rect(SDL_Rect r, SDL_Color c) : rect(r), color(c) {}
    
        void draw(SDL_Renderer* renderer, float scale, float offsetX, float offsetY) const override {
            SDL_FRect scaledRect = {
                rect.x * scale + offsetX,
                rect.y * scale + offsetY,
                rect.w * scale,
                rect.h * scale
            };
    
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &scaledRect);
        }
    
        bool contains(int x, int y, float scale, float offsetX, float offsetY) const override {
            float scaledX = rect.x * scale + offsetX;
            float scaledY = rect.y * scale + offsetY;
            float scaledW = rect.w * scale;
            float scaledH = rect.h * scale;
    
            return x >= scaledX && x < scaledX + scaledW &&
                   y >= scaledY && y < scaledY + scaledH;
        }

        //void drawToSurface(SDL_Surface* surface) const override {
        //    // 1) получаем enum-формат пикселя
        //    Uint32 pf = surface->format->format;
        //    // 2) свёртка RGBA в одно число
        //    Uint32 c = SDL_MapRGBA(pf, color.r, color.g, color.b, color.a);
        //    // 3) прямоугольник в логике слоя (целые)
        //    SDL_Rect dst{ rect.x, rect.y, rect.w, rect.h };
        //    // 4) заливаем
        //    SDL_FillSurfaceRect(surface, &dst, c);
        //}
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
    
        void draw(SDL_Renderer* renderer, float scale, float offsetX, float offsetY) const override {
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

        //void drawToSurface(SDL_Surface* surface) const override {
        //    Uint32 c = SDL_MapRGBA(surface->format,
        //                        color.r, color.g, color.b, color.a);
        //    auto plot = [&](int px, int py){
        //        if (px<0||py<0||px>=surface->w||py>=surface->h) return;
        //        reinterpret_cast<Uint32*>(surface->pixels)[py*surface->w + px] = c;
        //    };
        //    for (auto& cir : circles) {
        //        int cx = int(cir.x), cy = int(cir.y), r = int(cir.radius);
        //        // брезенхем
        //        int x = r, y = 0, err = 0;
        //        while (x >= y) {
        //            plot(cx + x, cy + y); plot(cx + y, cy + x);
        //            plot(cx - y, cy + x); plot(cx - x, cy + y);
        //            plot(cx - x, cy - y); plot(cx - y, cy - x);
        //            plot(cx + y, cy - x); plot(cx + x, cy - y);
        //            if (err <= 0) { y++; err += 2*y + 1; }
        //            if (err >  0) { x--; err -= 2*x + 1; }
        //        }
        //    }
        //}
    
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
    
        void draw(SDL_Renderer* renderer, float scale, float offsetX, float offsetY) const override {
            SDL_FRect dstRect = {
                offsetX, offsetY,
                width * scale,
                height * scale
            };
            SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
        }

        //void drawToSurface(SDL_Surface* surface, SDL_Renderer* renderer) const override {
        //    // захватим текстуру в пиксели через рендерер
        //    std::vector<Uint32> buf(width*height);
        //    // переключиться на рендеринг в эту текстуру
        //    SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
        //    SDL_SetRenderTarget(renderer, texture);
        //    // считать прямоугольник 0,0,width,height
        //    SDL_RenderReadPixels(renderer, nullptr,
        //                        surface.format->format,
        //                        buf.data(), width * sizeof(Uint32));
        //    // вернуть рендер-таргет
        //    SDL_SetRenderTarget(renderer, oldTarget);
        //    // скопировать в surface
        //    std::memcpy(surface->pixels, buf.data(), buf.size()*sizeof(Uint32));
        //}
};


class PenTool {
    public:
    std::vector<SDL_FPoint> points;
    bool isClosed = false;

    void addPoint(SDL_FPoint pt) {
        if (!points.empty()) {
            SDL_FPoint first = points[0];
            float dx = pt.x - first.x;
            float dy = pt.y - first.y;
            if (points.size() >= 3 && dx * dx + dy * dy < 25.0f) { // ближе 5 пикселей
                isClosed = true;
                return;
            }
        }
        points.push_back(pt);
    }

    void reset() {
        points.clear();
        isClosed = false;
    }

    void render(SDL_Renderer* renderer) const {
        if (points.empty()) return;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            SDL_RenderLine(renderer, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
        }
        if (isClosed && points.size() > 2) {
            SDL_RenderLine(renderer, points.back().x, points.back().y, points[0].x, points[0].y);
        }
    }
};