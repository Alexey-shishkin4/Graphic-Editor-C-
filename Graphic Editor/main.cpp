#define SDL_MAIN_HANDLED
//#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#include <vector>

struct Rect {
    float x, y, w, h;
};

std::vector<Rect> user_rects;
bool button1_pressed = false;
bool ctrl_pressed = false;
bool hovering_button1 = false;


static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Graphic Editor", 1280, 720, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        printf("EXIT SUKA!");
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        if (event->key.scancode == SDL_SCANCODE_1) {
            if (button1_pressed) {
                button1_pressed = false;
            } else {
                button1_pressed = true;
            }
        }
        if (event->key.mod & SDL_KMOD_CTRL) {
            ctrl_pressed = true;
        }
        if (ctrl_pressed && event->key.scancode == SDL_SCANCODE_Z) {
            if (!user_rects.empty()) {
                user_rects.pop_back();
            }
        }
    }
    if (event->type == SDL_EVENT_KEY_UP) {
        if (event->key.mod & SDL_KMOD_CTRL) {
            ctrl_pressed = false;
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
        float mx = static_cast<float>(event->button.x);
        float my = static_cast<float>(event->button.y);
    
        SDL_FRect button1 = { 10.0f, 20.0f, 80.0f, 40.0f };
        bool in_button1 = (mx >= button1.x && mx <= button1.x + button1.w &&
                           my >= button1.y && my <= button1.y + button1.h);
    
        if (in_button1) {
            button1_pressed = !button1_pressed;
        } else if (button1_pressed) {
            user_rects.push_back(Rect{ mx - 50.0f, my - 30.0f, 100.0f, 60.0f });
        }
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        float mx = static_cast<float>(event->motion.x);
        float my = static_cast<float>(event->motion.y);
    
        SDL_FRect button1 = { 10.0f, 20.0f, 80.0f, 40.0f };
        hovering_button1 = (mx >= button1.x && mx <= button1.x + button1.w &&
                            my >= button1.y && my <= button1.y + button1.h);
    }

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void *appstate)
{
    static Uint32 start_time = 0;
    static bool background_done = false;
    static float sidebar_progress = 0.0f; // от 0 до 1
    static Uint32 sidebar_start_time = 0;

    if (start_time == 0) {
        start_time = SDL_GetTicks();
    }

    Uint32 now_ms = SDL_GetTicks();
    float elapsed = (now_ms - start_time) / 1000.0f;

    float transition_duration = 2.0f;
    float t = elapsed / transition_duration;

    if (t >= 1.0f && !background_done) {
        background_done = true;
        sidebar_start_time = SDL_GetTicks();
    }

    if (t > 1.0f) t = 1.0f;

    SDL_SetRenderDrawColorFloat(renderer, t, t, t, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);

    float sidebar_max_width = 100.0f;
    float sidebar_current_width = 0.0f;

    if (background_done) {
        float sidebar_elapsed = (now_ms - sidebar_start_time) / 1000.0f;
        float sidebar_duration = 1.0f; // 1 секунда
        sidebar_progress = sidebar_elapsed / sidebar_duration;
        if (sidebar_progress > 1.0f) sidebar_progress = 1.0f;
        sidebar_current_width = sidebar_max_width * sidebar_progress;

        SDL_FRect sidebar = { 0.0f, 0.0f, sidebar_current_width, 720.0f };
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &sidebar);

        if (sidebar_progress == 1.0f) {
            SDL_FRect button1 = { 10.0f, 20.0f, 80.0f, 40.0f };
            if (button1_pressed) {
                SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);  // нажатая
            } else if (hovering_button1) {
                SDL_SetRenderDrawColor(renderer, 180, 180, 255, 255);  // при наведении
            } else {
                SDL_SetRenderDrawColor(renderer, 150, 150, 255, 255);  // обычная
            }
            SDL_RenderFillRect(renderer, &button1);

            //if (button1_pressed) {
            //    SDL_FRect center_rect = { 540.0f, 310.0f, 200.0f, 100.0f };
            //    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);  // красный
            //    SDL_RenderFillRect(renderer, &center_rect);
            //}
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);  // красные прямоугольники
    for (const Rect& r : user_rects) {
        SDL_FRect rect = { r.x, r.y, r.w, r.h };
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}


/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}


int main(int argc, char *argv[])
{
    SDL_AppResult result;
    void *appstate = nullptr;

    result = SDL_AppInit(&appstate, argc, argv);
    if (result != SDL_APP_CONTINUE) {
        std::cerr << "Failed to initialize app!" << std::endl;
        return 1;
    }

    SDL_Event event;
    bool exit_flag = false;
    while (result == SDL_APP_CONTINUE) {
        while (SDL_PollEvent(&event)) {
            result = SDL_AppEvent(appstate, &event);  // Handle events
            if (result == SDL_APP_SUCCESS) {
                exit_flag = true;
                break;
            }
        }
        if (exit_flag)
            break;
        result = SDL_AppIterate(appstate);  // Iterate each frame
    }

    // Clean up
    SDL_AppQuit(appstate, result);
    SDL_Quit();
    return 0;
}