#pragma once
#include <vector>
#include "types.h"
#include "layer.h"

struct Action {
    ActionType type;
    int layerIndex;
    Rect rect;
    Stroke stroke;
    bool previous_visibility;
    int previous_active_layer;
};

class UndoManager {
    std::vector<Action> history;
    int index = -1;

public:
    void add_action(const Action& action);
    void undo(std::vector<Layer>& layers, int& active_layer);
    void redo(std::vector<Layer>& layers, int& active_layer);
};
