#pragma once
#include "Shape.h"

class Line : public Shape {
public:
    Line(int x1, int y1, int x2, int y2);
    void draw(SDL_Renderer* renderer) override;

private:
    int x1, y1, x2, y2;
};
