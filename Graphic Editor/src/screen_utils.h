#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

#include <SDL3/SDL.h>

SDL_FPoint screenToWorld(float x, float y, float scale, float offsetX, float offsetY);

#endif
