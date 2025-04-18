#define SDL_MAIN_HANDLED
//#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#include <vector>

struct Rect {
    float x, y, w, h;
};


struct Layer {
    std::vector<Rect> rects;
    bool visible = true;
    std::string name;
};


//std::vector<Rect> user_rects;
std::vector<Layer> layers;
int active_layer = 0;  // индекс активного слоя

bool button1_pressed = false;
bool ctrl_pressed = false;
bool hovering_button1 = false;


static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Graphic Editor", 1280, 720, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    layers.push_back(Layer{});
    layers[0].name = "Layer 1";

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
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
        if (event->key.scancode == SDL_SCANCODE_N) {
            Layer new_layer;
            new_layer.name = "Layer " + std::to_string(layers.size() + 1);
            layers.push_back(new_layer);
            active_layer = layers.size() - 1;
            printf("New layer added. Total: %zu\n", layers.size());
        }
        if (event->key.scancode == SDL_SCANCODE_TAB) {
            active_layer = (active_layer + 1) % layers.size();  // следующий слой
            printf("Active layer: %d (%s)\n", active_layer, layers[active_layer].name.c_str());
        }
        if (event->key.scancode == SDL_SCANCODE_DELETE) {
            if (active_layer != 0 && active_layer < (int)layers.size()) {
                layers.erase(layers.begin() + active_layer);
                active_layer = 0;
            }
        }
        if (event->key.scancode == SDL_SCANCODE_UP) {
            if (active_layer > 0) {
                std::swap(layers[active_layer], layers[active_layer - 1]);
                active_layer--;
            }
        }
        if (event->key.scancode == SDL_SCANCODE_DOWN) {
            if (active_layer != 0 && active_layer < (int)layers.size() - 1) {
                std::swap(layers[active_layer], layers[active_layer + 1]);
                active_layer++;
            }
        }
        //if (ctrl_pressed && event->key.scancode == SDL_SCANCODE_Z) {
        //    if (!user_rects.empty()) {
        //        user_rects.pop_back();
        //    }
        //}
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
        SDL_FRect sidebar_rect = { 0.0f, 0.0f, 100.0f, 600.0f }; // Область GUI-сайдбара
        bool in_button1 = (mx >= button1.x && mx <= button1.x + button1.w &&
                           my >= button1.y && my <= button1.y + button1.h);
        bool in_sidebar = (mx >= sidebar_rect.x && mx <= sidebar_rect.x + sidebar_rect.w &&
                           my >= sidebar_rect.y && my <= sidebar_rect.y + sidebar_rect.h);
    
        if (in_button1) {
            button1_pressed = !button1_pressed;
        } else if (button1_pressed && !in_sidebar) {
            if (!layers.empty() && active_layer >= 0 && active_layer < layers.size()) {
                layers[active_layer].rects.push_back(Rect{ mx - 50.0f, my - 30.0f, 100.0f, 60.0f });
            }
        }
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
            float mx = static_cast<float>(event->button.x);
            float my = static_cast<float>(event->button.y);
        
            for (int i = 0; i < (int)layers.size(); ++i) {
                SDL_FRect layer_button = { 10.0f, 80.0f + i * 40.0f, 80.0f, 30.0f };
                SDL_FRect eye_icon = { layer_button.x + 60.0f, layer_button.y + 5.0f, 15.0f, 15.0f };
        
                if (mx >= eye_icon.x && mx <= eye_icon.x + eye_icon.w &&
                    my >= eye_icon.y && my <= eye_icon.y + eye_icon.h) {
                    layers[i].visible = !layers[i].visible; // Переключаем видимость
                } else if (mx >= layer_button.x && mx <= layer_button.x + layer_button.w &&
                           my >= layer_button.y && my <= layer_button.y + layer_button.h) {
                    active_layer = i; // Выбор слоя
                }
            }
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


SDL_AppResult SDL_AppIterate(void *appstate) {
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

    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    for (const Layer& layer : layers) {
        if (!layer.visible) continue;
        for (const Rect& r : layer.rects) {
            SDL_FRect rect = { r.x, r.y, r.w, r.h };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // Рисуем список слоёв
    int y = 80;
    for (int i = 0; i < (int)layers.size(); ++i) {
        SDL_FRect layer_button = { 10.0f, (float)y, 80.0f, 30.0f };
        if (i == active_layer) {
            SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        }
        SDL_RenderFillRect(renderer, &layer_button);

        // Нарисовать индикатор видимости
        if (layers[i].visible) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // зелёный
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // красный
        }
        SDL_FRect eye = { layer_button.x + 60.0f, layer_button.y + 5.0f, 15.0f, 15.0f };
        SDL_RenderFillRect(renderer, &eye);

        y += 40;
    }

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}


/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    /* SDL will clean up the window/renderer for us. */
}


int main(int argc, char *argv[]) {
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