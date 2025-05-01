#define SDL_MAIN_HANDLED
//#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "editor.h"

int main(int argc, char* argv[]) {
    Editor editor;
    editor.run();
    return 0;
}