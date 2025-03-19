#pragma once
#include <SDL2/SDL.h>
#include "Renderer.h"

class Application {
public:
    Application();
    ~Application();

    bool init(const char* title, int width, int height);
    void run();
    void cleanup();

private:
    SDL_Window* window;
    SDL_Renderer* sdlRenderer;
    Renderer* renderer;
    bool isRunning;

    void handleEvents();
};
