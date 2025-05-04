#include "screen_utils.h"

SDL_FPoint screenToWorld(float x, float y, float scale, float offsetX, float offsetY) {
    return SDL_FPoint{
        (x - offsetX) / scale,
        (y - offsetY) / scale
    };
}
