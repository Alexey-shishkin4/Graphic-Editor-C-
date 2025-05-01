#pragma once
#include <vector>
#include <string>
#include "types.h"

struct Layer {
    std::vector<Rect> rects;
    bool visible = true;
    std::string name;

    Layer() = default;
    Layer(const Layer&) = default;
    Layer& operator=(const Layer&) = default;
};