#include "undo.h"
#include <string>
#include "editor.h"

void UndoManager::add_action(const Action& action) {
    if (index + 1 < (int)history.size())
        history.erase(history.begin() + index + 1, history.end());
    history.push_back(action);
    ++index;
}

void UndoManager::undo(Editor& editor, std::vector<Layer>& layers, int& active_layer) {
    if (index < 0) return;

    const Action& action = history[index];

    int tmp = active_layer;
    int tmp1 = action.previous_active_layer;

    switch (action.type) {
        case ActionType::AddRect:
            if (!layers[action.layerIndex].rects.empty()) {
                layers[action.layerIndex].rects.pop_back();
            }
            break;
        case ActionType::ToggleVisibility:
            layers[action.layerIndex].visible = action.previous_visibility;
            break;
        case ActionType::ChangeActiveLayer:
            std::swap(tmp, tmp1);
            break;
        case ActionType::MoveLayerUp:
        case ActionType::MoveLayerDown:
            std::swap(
                layers[action.layerIndex],
                layers[action.layerIndex + (action.type == ActionType::MoveLayerUp ? -1 : 1)]
            );
            active_layer = action.layerIndex;
            break;
        case ActionType::DrawBrushStroke: // Обработка кисти
            // Remove the last brush stroke (undo the drawing)
            if (!editor.brushStrokes.empty()) {
                editor.brushStrokes.pop_back();
            }
            if (!layers[action.layerIndex].strokes.empty()) {
                layers[action.layerIndex].strokes.pop_back();
            }
            break;
        default:
            break;
    }

    --index;
}

void UndoManager::redo(Editor& editor, std::vector<Layer>& layers, int& active_layer) {
    if (index + 1 >= (int)history.size()) return;

    ++index;
    const Action& action = history[index];

    int tmp = active_layer;
    int tmp1 = action.previous_active_layer;

    switch (action.type) {
        case ActionType::AddRect:
            layers[action.layerIndex].rects.push_back(action.rect);
            break;
        case ActionType::ToggleVisibility:
            layers[action.layerIndex].visible = !action.previous_visibility;
            break;
        case ActionType::ChangeActiveLayer:
            std::swap(tmp, tmp1);
            break;
        case ActionType::MoveLayerUp:
        case ActionType::MoveLayerDown:
            std::swap(
                layers[action.layerIndex],
                layers[action.layerIndex + (action.type == ActionType::MoveLayerUp ? -1 : 1)]
            );
            active_layer = action.layerIndex + (action.type == ActionType::MoveLayerUp ? -1 : 1);
            break;
        case ActionType::DrawBrushStroke: // Обработка повторного действия кисти
            editor.brushStrokes.push_back(action.brushStroke);
            break;
        default:
            break;
    }
}
