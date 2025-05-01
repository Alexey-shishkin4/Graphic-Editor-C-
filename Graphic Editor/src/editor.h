#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include "layer.h"
#include "types.h"
#include "undo.h"
#include "tools.h"

class Editor {
public:
    Editor();
    ~Editor();
    void run();

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    bool running = true;
    bool button1_pressed = false;
    bool hovering_button1 = false;
    int tool_count = 4;

    std::vector<Layer> layers;
    int active_layer = 0;
    Rect* selected_rect = nullptr;

    float drag_offset_x = 0, drag_offset_y = 0;
    bool dragging = false;
    bool isDragging = false;
    SDL_FRect dragRect = {0}; // временный прямоугольник

    Tool current_tool = Tool::None;
    //BrushState brushState;
    UndoManager undoManager;

    SDL_Color currentColor = {0, 0, 0, 255};
    int currentThickness = 2;

    void handle_event(SDL_Event &e);
    void handle_mouse_button_down(SDL_MouseButtonEvent& button_event);
    void handle_mouse_motion(SDL_MouseMotionEvent& motion_event);
    void handle_mouse_button_up(SDL_MouseButtonEvent& button_event);
    void toggle_tool(Tool tool);
    void render();
};
