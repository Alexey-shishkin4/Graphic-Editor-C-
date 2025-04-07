#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


class Shape {
public:
    virtual ~Shape() {}
    virtual void draw(SDL_Renderer* renderer) = 0;
};
