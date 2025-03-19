#include "Application.h"
#include <iostream>

Application::Application() : window(nullptr), sdlRenderer(nullptr), renderer(nullptr), isRunning(false) {}

Application::~Application() {
    cleanup();
}

bool Application::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer) {
        std::cerr << "CreateRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = new Renderer(sdlRenderer);
    isRunning = true;
    return true;
}

void Application::run() {
    while (isRunning) {
        handleEvents();
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderClear(sdlRenderer);

        renderer->render();

        SDL_RenderPresent(sdlRenderer);
        SDL_Delay(16); // ~60 FPS
    }
}

void Application::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        renderer->handleEvent(event);
    }
}

void Application::cleanup() {
    delete renderer;
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
