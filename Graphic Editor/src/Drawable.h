#pragma once
#include <SDL3/SDL.h>


class Drawable {
public:
    virtual ~Drawable() = default;

    // Отрисовка с учетом масштаба и смещения
    virtual void draw(SDL_Renderer* renderer, float scale, float offsetX, float offsetY) const = 0;

    // (Необязательно) Проверка попадания в объект — например, для выбора
    virtual bool contains(int x, int y, float scale, float offsetX, float offsetY) const { return false; }
};
