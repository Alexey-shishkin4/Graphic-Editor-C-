#pragma once
#include <vector>
#include "types.h"
#include "layer.h"

class Editor;

struct Action {
    ActionType type;
    int layerIndex;
    Rect rect;
    bool previous_visibility;
    int previous_active_layer;
    BrushStroke brushStroke;
};

class UndoManager {
    std::vector<Action> history;
    int index = -1;

public:
    void add_action(const Action& action);
    void undo(Editor& editor, std::vector<Layer>& layers, int& active_layer);
    void redo(Editor& editor, std::vector<Layer>& layers, int& active_layer);
};
