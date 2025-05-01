#include "tools.h"

void handle_mouse_event(SDL_Event& e, ToolType tool, BrushToolState& brushState,
                        std::vector<Layer>& layers, UndoManager& undoManager,
                        int activeLayerIndex, SDL_Color currentColor, float thickness) {
    if (tool == ToolType::Brush) {
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
            brushState.drawing = true;
            brushState.currentStroke.points.clear();
            brushState.currentStroke.color = currentColor;
            brushState.currentStroke.thickness = thickness;
        } else if (e.type == SDL_EVENT_MOUSE_MOTION && brushState.drawing) {
            float x = e.motion.x;
            float y = e.motion.y;
            brushState.currentStroke.points.push_back({x, y});
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
            if (!brushState.currentStroke.points.empty()) {
                layers[activeLayerIndex].strokes.push_back(brushState.currentStroke);
                undoManager.add_action(Action{.type = ActionType::StrokeDrawn, .layerIndex = activeLayerIndex, .stroke = brushState.currentStroke});
            }
            brushState.drawing = false;
        }
    }
}

void draw(SDL_Renderer* renderer, const std::vector<Layer>& layers,
          ToolType currentTool, const Stroke* currentStroke) {
    for (const auto& layer : layers) {
        if (!layer.visible) continue;

        for (const auto& obj : layer.objects) {
            SDL_SetRenderDrawColor(renderer, obj.color.r, obj.color.g, obj.color.b, obj.color.a);
            SDL_RenderFillRect(renderer, &obj.rect);
        }

        for (const auto& stroke : layer.strokes) {
            if (stroke.points.size() < 2) continue;
            SDL_SetRenderDrawColor(renderer, stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
            for (size_t i = 1; i < stroke.points.size(); ++i) {
                SDL_RenderLine(renderer, stroke.points[i - 1].x, stroke.points[i - 1].y,
                                            stroke.points[i].x, stroke.points[i].y);
            }
        }
    }

    if (currentTool == ToolType::Brush && currentStroke && currentStroke->points.size() > 1) {
        SDL_SetRenderDrawColor(renderer, currentStroke->color.r, currentStroke->color.g, currentStroke->color.b, currentStroke->color.a);
        for (size_t i = 1; i < currentStroke->points.size(); ++i) {
            SDL_RenderLine(renderer, currentStroke->points[i - 1].x, currentStroke->points[i - 1].y,
                                        currentStroke->points[i].x, currentStroke->points[i].y);
        }
    }
}