#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include "layer.h"
#include "types.h"
#include "tools.h"

class UndoManager;

class Editor {
public:
    std::vector<BrushStroke> brushStrokes;
    
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
    int canvasWidth = 800;
    int canvasHeight = 600;
    SDL_Rect canvasRect = {0, 0, 800, 600};
    Rect* selected_rect = nullptr;

    float drag_offset_x = 0, drag_offset_y = 0;
    bool dragging = false;
    bool isDragging = false;
    SDL_FRect dragRect = {0}; // временный прямоугольник

    bool isBrushing = false;
    float lastBrushX = -1, lastBrushY = -1;
    float brushSize = 4.0f;

    Tool current_tool = Tool::None;
    //BrushState brushState;
    UndoManager undoManager;

    float scale = 1.0f; // Коэффициент масштабирования
    int offsetX = 0, offsetY = 0; // Сдвиг холста

    void handle_event(SDL_Event &e);
    void handle_mouse_button_down(SDL_MouseButtonEvent& button_event);
    void handle_mouse_motion(SDL_MouseMotionEvent& motion_event);
    void handle_mouse_button_up(SDL_MouseButtonEvent& button_event);
    void toggle_tool(Tool tool);
    void render();
    void importImage(const std::string& path);
};
