#pragma once
#include <SDL3/SDL.h>
#include "types.h"
#include "layer.h"
#include "undo.h"

struct BrushToolState {
    bool drawing = false;
    Stroke currentStroke;
};

void handle_mouse_event(SDL_Event& e, Tool tool, BrushToolState& brushState,
                        std::vector<Layer>& layers, UndoManager& undoManager,
                        int activeLayerIndex, SDL_Color currentColor, float thickness);

void draw(SDL_Renderer* renderer, const std::vector<Layer>& layers,
          Tool currentTool, const Stroke* currentStroke = nullptr);
