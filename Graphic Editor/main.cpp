#define SDL_MAIN_HANDLED
//#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <vector>

struct Rect {
    float x, y, w, h;
};


struct Layer {
    std::vector<Rect> rects;
    bool visible = true;
    std::string name;
};

enum class Tool {
    None,
    Select,
    Move,
    Erase
};

Tool current_tool = Tool::None;
const char* tool_names[] = { "Erase", "Select", "Move" };
const int tool_count = 3;

enum class ActionType {
    AddRect,
    RemoveRect,
    ToggleVisibility,
    ChangeActiveLayer,
    MoveLayerUp,
    MoveLayerDown
};

struct Action {
    ActionType type;
    int layer_index;
    Rect rect; // используем при Add/Remove
    bool previous_visibility;
    int previous_active_layer;
};

std::vector<Action> undo_stack;
std::vector<Action> redo_stack;


Rect* selected_rect = nullptr;


//std::vector<Rect> user_rects;
std::vector<Layer> layers;
int active_layer = 0;  // индекс активного слоя

bool button1_pressed = false;
bool ctrl_pressed = false;
bool hovering_button1 = false;
bool tool_selected = false;

bool isDragging = false;
SDL_FRect dragRect = {0}; // временный прямоугольник


static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

bool point_in_rect(float x, float y, const Rect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

void undo() {
    if (undo_stack.empty()) return;

    Action action = undo_stack.back();
    undo_stack.pop_back();

    switch (action.type) {
        case ActionType::AddRect:
            if (!layers[action.layer_index].rects.empty()) {
                layers[action.layer_index].rects.pop_back();
                redo_stack.push_back(action);
            }
            break;
        case ActionType::ToggleVisibility:
            layers[action.layer_index].visible = action.previous_visibility;
            redo_stack.push_back(action);
            break;
        case ActionType::ChangeActiveLayer:
            std::swap(active_layer, action.previous_active_layer);
            redo_stack.push_back(action);
            break;
        case ActionType::MoveLayerUp:
        case ActionType::MoveLayerDown:
            //инвертируем перемещение
            std::swap(layers[action.layer_index], layers[action.layer_index + (action.type == ActionType::MoveLayerUp ? -1 : 1)]);
            active_layer = action.layer_index;
            redo_stack.push_back(action);
            break;
        default: break;
    }
}


void redo() {
    if (redo_stack.empty()) return;

    Action action = redo_stack.back();
    redo_stack.pop_back();

    switch (action.type) {
        case ActionType::AddRect:
            layers[action.layer_index].rects.push_back(action.rect);
            undo_stack.push_back(action);
            break;
        case ActionType::ToggleVisibility:
            layers[action.layer_index].visible = !action.previous_visibility;
            undo_stack.push_back(action);
            break;
        case ActionType::ChangeActiveLayer:
            std::swap(active_layer, action.previous_active_layer);
            undo_stack.push_back(action);
            break;
        case ActionType::MoveLayerUp:
        case ActionType::MoveLayerDown:
            std::swap(layers[action.layer_index], layers[action.layer_index + (action.type == ActionType::MoveLayerUp ? -1 : 1)]);
            active_layer = action.layer_index + (action.type == ActionType::MoveLayerUp ? -1 : 1);
            undo_stack.push_back(action);
            break;
        default: break;
    }
}

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
        if (ctrl_pressed && event->key.scancode == SDL_SCANCODE_Z) {
            undo();
        }
        if (ctrl_pressed && event->key.scancode == SDL_SCANCODE_Y) {
            redo();
        }
        if (event->key.scancode == SDL_SCANCODE_S) {
            if (current_tool == Tool::Select) {
                current_tool = Tool::None;
            } else {
                current_tool = Tool::Select;
                printf("Select tool.\n");
            }
        }
        if (event->key.scancode ==  SDL_SCANCODE_M) {
            if (current_tool == Tool::Move) {
                current_tool = Tool::None;
            } else {
                current_tool = Tool::Move;
                printf("Move tool.\n");
            }
        }
        if (event->key.scancode == SDL_SCANCODE_E) {
            if (current_tool == Tool::Erase) {
                current_tool = Tool::None;
            } else {
                current_tool = Tool::Erase;
                printf("Erase tool.\n");
            }
        }
    }
    if (event->type == SDL_EVENT_KEY_UP) {
        if (event->key.mod & SDL_KMOD_CTRL) {
            ctrl_pressed = false;
        }
    }
    if (button1_pressed) {
        switch (event->type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    isDragging = true;
                    dragRect.x = event->button.x;
                    dragRect.y = event->button.y;
                    dragRect.w = 0;
                    dragRect.h = 0;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (isDragging) {
                    float x1 = dragRect.x;
                    float y1 = dragRect.y;
                    float x2 = event->motion.x;
                    float y2 = event->motion.y;

                    dragRect.x = fminf(x1, x2);
                    dragRect.y = fminf(y1, y2);
                    dragRect.w = fabsf(x2 - x1);
                    dragRect.h = fabsf(y2 - y1);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event->button.button == SDL_BUTTON_LEFT && isDragging) {
                    isDragging = false;
                    if (dragRect.w > 0 && dragRect.h > 0) {
                        Rect new_rect = Rect{
                            dragRect.x,
                            dragRect.y,
                            dragRect.w,
                            dragRect.h
                        };
            
                        // Добавляем прямоугольник в активный слой
                        layers[active_layer].rects.push_back(new_rect);
            
                        // Добавляем в undo стек
                        undo_stack.push_back(Action{
                            ActionType::AddRect,
                            active_layer,
                            new_rect
                        });
            
                        // Очищаем redo стек
                        redo_stack.clear();
                    }
                }
                break;
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
                //Rect new_rect = Rect{ mx - 50.0f, my - 30.0f, 100.0f, 60.0f };
                //layers[active_layer].rects.push_back(new_rect);
                //undo_stack.push_back(Action{ ActionType::AddRect, active_layer, new_rect });
                //redo_stack.clear();  // сбрасываем redo при новом действии
            }
        }
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
        SDL_FRect select_tool_button = { 10.0f, 400.0f, 80.0f, 30.0f };
        SDL_FRect move_tool_button   = { 10.0f, 440.0f, 80.0f, 30.0f };
        SDL_FRect erase_tool_button  = { 10.0f, 480.0f, 80.0f, 30.0f };

        if (mx >= select_tool_button.x && mx <= select_tool_button.x + select_tool_button.w &&
            my >= select_tool_button.y && my <= select_tool_button.y + select_tool_button.h) {
            if (current_tool == Tool::Select) {
                current_tool = Tool::None;
            } else {
                current_tool = Tool::Select;
                printf("Select tool.\n");
            }
        }

        else if (mx >= move_tool_button.x && mx <= move_tool_button.x + move_tool_button.w &&
                my >= move_tool_button.y && my <= move_tool_button.y + move_tool_button.h) {
            if (current_tool == Tool::Move) {
                current_tool = Tool::None;
            } else {
                current_tool = Tool::Move;
                printf("Move tool.\n");
            }
        }

        else if (mx >= erase_tool_button.x && mx <= erase_tool_button.x + erase_tool_button.w &&
                my >= erase_tool_button.y && my <= erase_tool_button.y + erase_tool_button.h) {
            if (current_tool == Tool::Erase) {
                current_tool = Tool::None;
            } else {
                current_tool = Tool::Erase;
                printf("Erase tool.\n");
            }
        }
        
    }

    static bool dragging = false;
    static float drag_offset_x = 0, drag_offset_y = 0;
    float mx = static_cast<float>(event->button.x);
    float my = static_cast<float>(event->button.y);

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (current_tool == Tool::Move) {
            for (auto& rect : layers[active_layer].rects) {
                if (point_in_rect(mx, my, rect)) {
                    selected_rect = &rect;
                    drag_offset_x = mx - rect.x;
                    drag_offset_y = my - rect.y;
                    dragging = true;
                    break;
                }
            }
        } 
        if (current_tool == Tool::Select) {
            selected_rect = nullptr;
            for (auto& rect : layers[active_layer].rects) {
                if (point_in_rect(mx, my, rect)) {
                    selected_rect = &rect;
                }
            }
        }
        if (current_tool == Tool::Erase) {
            auto& rects = layers[active_layer].rects;
            rects.erase(std::remove_if(rects.begin(), rects.end(), [mx, my](const Rect& r) {
                return point_in_rect(mx, my, r);
            }), rects.end());
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && dragging) {
        dragging = false;
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        float mx = static_cast<float>(event->motion.x);
        float my = static_cast<float>(event->motion.y);

        if (dragging && current_tool == Tool::Move && selected_rect) {
            selected_rect->x = mx - drag_offset_x;
            selected_rect->y = my - drag_offset_y;
        }
    
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
            for (int i = 1; i < tool_count + 1; ++i) {
                SDL_FRect button = { 10.0f, 360.0f + i * 40.0f, 80.0f, 30.0f };
        
                SDL_SetRenderDrawColor(renderer,
                    (int)current_tool == i ? 180 : 100,
                    (int)current_tool == i ? 180 : 100,
                    (int)current_tool == i ? 180 : 100,
                    255);
                SDL_RenderFillRect(renderer, &button);
        
                // Optionally draw text labels
                //DrawText(renderer, font, tool_names[i], button.x + 5, button.y + 5);
            }
        }
    }

    for (const Layer& layer : layers) {
        if (!layer.visible) continue;
        for (const Rect& r : layer.rects) {
            SDL_FRect rect = { r.x, r.y, r.w, r.h };

            SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
            SDL_RenderFillRect(renderer, &rect);

            if (&r == selected_rect) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderRect(renderer, &rect);
            }
        }
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