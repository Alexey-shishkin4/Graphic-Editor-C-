#include "Renderer.h"

Renderer::Renderer(SDL_Renderer* renderer) : sdlRenderer(renderer), drawing(false) {}

Renderer::~Renderer() {
    for (auto shape : shapes) {
        delete shape;
    }
}

void Renderer::handleEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
            startX = event.button.x;
            startY = event.button.y;
            drawing = true;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT && drawing) {
            int endX = event.button.x;
            int endY = event.button.y;

            // Example: add a line (later switch by selected tool)
            shapes.push_back(new Line(startX, startY, endX, endY));

            drawing = false;
        }
        break;
    }
}

void Renderer::render() {
    for (auto shape : shapes) {
        shape->draw(sdlRenderer);
    }

    // Optional: preview current drawing
    if (drawing) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(sdlRenderer, startX, startY, mouseX, mouseY);
    }
}
