#include "editor.h"
#include <iostream>
#include <algorithm>
#include <math.h>
#include <vector>
#include "undo.h"

bool point_in_rect(float x, float y, const Rect& r) {
    return x >= r.rect.x && x <= r.rect.x + r.rect.w && y >= r.rect.y && y <= r.rect.y + r.rect.h;
}
bool point_in_rect(float x, float y, const SDL_FRect& r) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}


SDL_FPoint screenToWorld(float screenX, float screenY, float scale, float offsetX, float offsetY) {
    SDL_FPoint world;
    world.x = (screenX - offsetX) / scale;
    world.y = (screenY - offsetY) / scale;
    return world;
}

SDL_FPoint worldToScreen(float worldX, float worldY, float scale, float offsetX, float offsetY) {
    SDL_FPoint screen;
    screen.x = worldX * scale + offsetX;
    screen.y = worldY * scale + offsetY;
    return screen;
}


void drawCircle(SDL_Renderer* renderer, float centerX, float centerY, float radius) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        SDL_RenderPoint(renderer, centerX + x, centerY + y);
        SDL_RenderPoint(renderer, centerX + y, centerY + x);
        SDL_RenderPoint(renderer, centerX - y, centerY + x);
        SDL_RenderPoint(renderer, centerX - x, centerY + y);
        SDL_RenderPoint(renderer, centerX - x, centerY - y);
        SDL_RenderPoint(renderer, centerX - y, centerY - x);
        SDL_RenderPoint(renderer, centerX + y, centerY - x);
        SDL_RenderPoint(renderer, centerX + x, centerY - y);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}



Editor::Editor() {
    SDL_SetAppMetadata("Graphic Editor", "1.0", "renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return;
    }

    if (!SDL_CreateWindowAndRenderer("Graphic Editor", 1280, 720, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return;
    }

    layers.push_back(Layer{});
    layers[0].name = "Layer 1";
}

Editor::~Editor() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Editor::run() {
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            handle_event(e);
        }
        render();
        SDL_Delay(16);
    }
}

void Editor::handle_event(SDL_Event& e) {
    // Обработка событий клавиш
    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.scancode == SDL_SCANCODE_ESCAPE) {
            running = false;
        } else if (e.key.scancode == SDL_SCANCODE_1) {
            if (button1_pressed) {
                button1_pressed = false;
            } else {
                button1_pressed = true;
            }
        }
         else if (e.key.scancode == SDL_SCANCODE_N) {
            Layer new_layer;
            new_layer.name = "Layer " + std::to_string(layers.size() + 1);
            layers.push_back(new_layer);
            active_layer = layers.size() - 1;
            printf("New layer added. Total: %zu\n", layers.size());
        } else if (e.key.scancode == SDL_SCANCODE_TAB) {
            active_layer = (active_layer + 1) % layers.size();
            printf("Active layer: %d (%s)\n", active_layer, layers[active_layer].name.c_str());
        } else if (e.key.scancode == SDL_SCANCODE_DELETE) {
            if (active_layer != 0 && active_layer < layers.size()) {
                layers.erase(layers.begin() + active_layer);
                active_layer = 0;
            }
        } else if (e.key.scancode == SDL_SCANCODE_UP) {
            if (active_layer > 0) {
                std::swap(layers[active_layer], layers[active_layer - 1]);
                active_layer--;
            }
        } else if (e.key.scancode == SDL_SCANCODE_DOWN) {
            if (active_layer < layers.size() - 1) {
                std::swap(layers[active_layer], layers[active_layer + 1]);
                active_layer++;
            }
        } else if (e.key.scancode == SDL_SCANCODE_Z && (e.key.mod & SDL_KMOD_CTRL)) {
            undoManager.undo(*this, layers, active_layer);
        } else if (e.key.scancode == SDL_SCANCODE_Y && (e.key.mod & SDL_KMOD_CTRL)) {
            undoManager.redo(*this, layers, active_layer);
        } else if (e.key.scancode == SDL_SCANCODE_S) {
            toggle_tool(Tool::Select);
        } else if (e.key.scancode == SDL_SCANCODE_M) {
            toggle_tool(Tool::Move);
        } else if (e.key.scancode == SDL_SCANCODE_E) {
            toggle_tool(Tool::Erase);
        } else if (e.key.scancode == SDL_SCANCODE_B) {
            toggle_tool(Tool::Brush);
        }
    }

    if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        const Uint16 mod = SDL_GetModState();
        const bool ctrlHeld = (mod & SDL_KMOD_CTRL);
    
        if (ctrlHeld) {
            brushSize += e.wheel.y;  // e.wheel.y > 0 вверх, < 0 вниз
            if (brushSize < 1.0f) brushSize = 1.0f;
            if (brushSize > 50.0f) brushSize = 50.0f;
            printf("Brush size: %.1f\n", brushSize);
        } else {
            // Масштаб камеры
            float mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY); // получаем координаты мыши

            // До масштабирования — позиция в мировых координатах
            float worldX = (mouseX - offsetX) / scale;
            float worldY = (mouseY - offsetY) / scale;

            // Сохраняем старый масштаб
            float oldScale = scale;

            // Меняем масштаб
            if (e.wheel.y > 0) {
                scale += 0.1f;
                if (scale > 5.0f) scale = 5.0f;
            } else if (e.wheel.y < 0) {
                scale -= 0.1f;
                if (scale < 0.1f) scale = 0.1f;
            }

            // После изменения масштаба: пересчитываем offset так, чтобы мышь "смотрела" на ту же мировую точку
            offsetX = mouseX - worldX * scale;
            offsetY = mouseY - worldY * scale;

            printf("scale = %.2f, offsetX = %.2f, offsetY = %.2f\n", scale, offsetX, offsetY);
            printf("%f %f %f\n", scale, offsetX, offsetY);
        }
    }
    // Обработка событий мыши
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        handle_mouse_button_down(e.button);
    }
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        handle_mouse_motion(e.motion);
    }
    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        handle_mouse_button_up(e.button);
    }
    if (e.type == SDL_EVENT_MOUSE_MOTION && e.key.scancode == SDL_BUTTON_LEFT && (e.key.mod & SDL_KMOD_CTRL)) {
        offsetX += e.motion.xrel;
        offsetY += e.motion.yrel;
    }
}

void Editor::handle_mouse_button_down(SDL_MouseButtonEvent& button_event) {
    float mx = static_cast<float>(button_event.x);
    float my = static_cast<float>(button_event.y);

    SDL_FRect button1 = { 10.0f, 20.0f, 80.0f, 40.0f };
    SDL_FRect sidebar_rect = { 0.0f, 0.0f, 100.0f, 600.0f };

    bool in_button1 = (mx >= button1.x && mx <= button1.x + button1.w &&
                       my >= button1.y && my <= button1.y + button1.h);
    bool in_sidebar = (mx >= sidebar_rect.x && mx <= sidebar_rect.x + sidebar_rect.w &&
                       my >= sidebar_rect.y && my <= sidebar_rect.y + sidebar_rect.h);

    if (button_event.button == SDL_BUTTON_LEFT) {
        if (in_button1) {
            button1_pressed = !button1_pressed;
            return;
        }

        // Обработка переключения видимости слоёв и выбора слоя
        for (int i = 0; i < static_cast<int>(layers.size()); ++i) {
            SDL_FRect layer_button = { 10.0f, 80.0f + i * 40.0f, 80.0f, 30.0f };
            SDL_FRect eye_icon = { layer_button.x + 60.0f, layer_button.y + 5.0f, 15.0f, 15.0f };

            if (mx >= eye_icon.x && mx <= eye_icon.x + eye_icon.w &&
                my >= eye_icon.y && my <= eye_icon.y + eye_icon.h) {
                layers[i].visible = !layers[i].visible;
                return;
            } else if (mx >= layer_button.x && mx <= layer_button.x + layer_button.w &&
                       my >= layer_button.y && my <= layer_button.y + layer_button.h) {
                active_layer = i;
                return;
            }
        }

        // Переключение инструментов
        SDL_FRect select_tool_button = { 10.0f, 400.0f, 80.0f, 30.0f };
        SDL_FRect move_tool_button   = { 10.0f, 440.0f, 80.0f, 30.0f };
        SDL_FRect erase_tool_button  = { 10.0f, 480.0f, 80.0f, 30.0f };
        SDL_FRect brush_tool_button  = { 10.0f, 520.0f, 80.0f, 30.0f };

        auto toggle_tool = [&](Tool tool) {
            current_tool = (current_tool == tool ? Tool::None : tool);
        };

        if (point_in_rect(mx, my, select_tool_button)) {
            toggle_tool(Tool::Select);
            return;
        } else if (point_in_rect(mx, my, move_tool_button)) {
            toggle_tool(Tool::Move);
            return;
        } else if (point_in_rect(mx, my, erase_tool_button)) {
            toggle_tool(Tool::Erase);
            return;
        } else if (point_in_rect(mx, my, brush_tool_button)) {
            toggle_tool(Tool::Brush);
            return;
        }

        // Поведение инструментов
        if (current_tool == Tool::Move) {
            // Преобразуем координаты мыши в мировые
            float world_mx = (mx - offsetX) / scale;
            float world_my = (my - offsetY) / scale;
        
            for (auto& rect : layers[active_layer].rects) {
                if (point_in_rect(world_mx, world_my, rect)) {
                    selected_rect = &rect;
                    drag_offset_x = world_mx - rect.rect.x;
                    drag_offset_y = world_my - rect.rect.y;
                    dragging = true;
                    printf("dragging is true\n");
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
            rects.erase(std::remove_if(rects.begin(), rects.end(),
                [mx, my](const Rect& r) {
                    return point_in_rect(mx, my, r);
                }), rects.end());
        }

        if (current_tool == Tool::Brush) {
            isBrushing = true;
            //brushStrokes.clear();
        
            float mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            SDL_FPoint worldMouse = screenToWorld(mouseX, mouseY, scale, offsetX, offsetY);
        
            BrushStroke stroke;
            stroke.addCircle(worldMouse.x, worldMouse.y, 4.0f);
            brushStrokes.push_back(stroke);
        }

        // Начало выделения прямоугольной области
        if (button1_pressed) {
            isDragging = true;
            dragRect.x = mx;
            dragRect.y = my;
            dragRect.w = 0;
            dragRect.h = 0;
        }
    }
}


void Editor::handle_mouse_motion(SDL_MouseMotionEvent& motion_event) {
    float mx = static_cast<float>(motion_event.x);
    float my = static_cast<float>(motion_event.y);
    if (dragging && current_tool == Tool::Move && selected_rect) {
        // Опять преобразуем мышь в мировые координаты
        float world_mx = (mx - offsetX) / scale;
        float world_my = (my - offsetY) / scale;
    
        selected_rect->rect.x = world_mx - drag_offset_x;
        selected_rect->rect.y = world_my - drag_offset_y;
    }
    
    SDL_FRect button1 = { 10.0f, 20.0f, 80.0f, 40.0f };
    hovering_button1 = (mx >= button1.x && mx <= button1.x + button1.w &&
                        my >= button1.y && my <= button1.y + button1.h);

                        
    if (isBrushing && current_tool == Tool::Brush) {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        
        // Преобразуем экранные координаты в мировые
        SDL_FPoint worldMouse = screenToWorld(mouseX, mouseY, scale, offsetX, offsetY);
        BrushStroke& stroke = brushStrokes.back();
        
        if (lastBrushX >= 0 && lastBrushY >= 0) {
            float dx = worldMouse.x - lastBrushX;
            float dy = worldMouse.y - lastBrushY;
            float dist = std::hypot(dx, dy);
            int steps = static_cast<int>(dist / 1.5f);
        
            // Расчет промежуточных точек
            for (int i = 0; i <= steps; ++i) {
                float t = i / static_cast<float>(steps);
                float ix = lastBrushX + t * dx;
                float iy = lastBrushY + t * dy;
            
                // Преобразуем мировые координаты обратно в экранные, чтобы рисовать
                SDL_FPoint screenPos = worldToScreen(ix, iy, scale, offsetX, offsetY);
            
                // Теперь рисуем с корректным учетом масштаба и смещения
                stroke.addCircle(screenPos.x, screenPos.y, brushSize);
            }
        }
    
        lastBrushX = worldMouse.x;
        lastBrushY = worldMouse.y;
    }





    if (button1_pressed) {
        if (isDragging) {
            float x1 = dragRect.x;
            float y1 = dragRect.y;
            float x2 = motion_event.x;
            float y2 = motion_event.y;

            dragRect.x = fminf(x1, x2);
            dragRect.y = fminf(y1, y2);
            dragRect.w = fabsf(x2 - x1);
            dragRect.h = fabsf(y2 - y1);
        }
    }
}

void Editor::handle_mouse_button_up(SDL_MouseButtonEvent& button_event) {
    lastBrushX = lastBrushY = -1;
    if (dragging) {
        dragging = false;
    }

    if (current_tool == Tool::Brush && button_event.button == SDL_BUTTON_LEFT && isBrushing) {
        isBrushing = false;
        
        if (!brushStrokes.empty()) {
            BrushStroke& stroke = brushStrokes.back();
            int c = 0;
            for (const auto& circle : stroke.circles) {
                c += 1;
                if (c == 1) continue;
                SDL_FPoint worldPos = screenToWorld(circle.x, circle.y, scale, offsetX, offsetY);
    
                SDL_Rect r = {
                    static_cast<int>(worldPos.x - circle.radius),
                    static_cast<int>(worldPos.y - circle.radius),
                    static_cast<int>(2 * circle.radius),
                    static_cast<int>(2 * circle.radius)
                };
    
                Rect new_rect(r, SDL_Color{160, 160, 160, 255});
                stroke.rects.push_back(new_rect);

            }

            undoManager.add_action(Action{
                ActionType::DrawBrushStroke,
                active_layer,                  
                Rect{SDL_Rect{10, 10, 10, 10}, SDL_Color{160, 160, 160, 255}},                        
                layers[active_layer].visible,
                active_layer,                  
                stroke                         
            });
            stroke.circles.clear();
            layers[active_layer].strokes.push_back(stroke);
            lastBrushX = -1;
            lastBrushY = -1;
        }
    }
    

    if (button1_pressed) {
        if (button_event.button == SDL_BUTTON_LEFT && isDragging) {
            isDragging = false;
            if (dragRect.w > 0 && dragRect.h > 0) {
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                
                // Преобразуем координаты мыши в мировые
                SDL_FPoint worldPos = screenToWorld(mouseX, mouseY, scale, offsetX, offsetY);
                
                // Отменяем масштабирование для размеров прямоугольника
                int scaledWidth = static_cast<int>(dragRect.w / scale);
                int scaledHeight = static_cast<int>(dragRect.h / scale);
                
                // Вычисляем смещение для левого верхнего угла
                float worldStartX = screenToWorld(dragRect.x, dragRect.y, scale, offsetX, offsetY).x;
                float worldStartY = screenToWorld(dragRect.x, dragRect.y, scale, offsetX, offsetY).y;
                
                // Теперь создаем новый прямоугольник с учетом смещения и масштаба
                SDL_Rect r = { 
                    static_cast<int>(worldStartX), 
                    static_cast<int>(worldStartY), 
                    scaledWidth, 
                    scaledHeight 
                };
    
                Rect new_rect(r, SDL_Color({160, 160, 160, 255}));
    
                if (!layers.empty() && active_layer >= 0 && active_layer < static_cast<int>(layers.size())) {
                    layers[active_layer].rects.push_back(new_rect);
                    undoManager.add_action(Action{ ActionType::AddRect, active_layer, new_rect });
                }
            }
        }
    }
    
    
}



void Editor::toggle_tool(Tool tool) {
    if (current_tool == tool) {
        current_tool = Tool::None;
    } else {
        current_tool = tool;
    }
}

void Editor::render() {
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

        for (const Layer& layer : layers) {
            if (!layer.visible) continue;
        
            for (const Drawable* obj : layer.objects) {
                obj->draw(renderer, scale, offsetX, offsetY);
            }
        }
    
        for (const Rect& r : layer.rects) {
            r.draw(renderer, scale, offsetX, offsetY);
            if (&r == selected_rect) {
                SDL_FRect scaledRect = {
                    r.rect.x * scale + offsetX,
                    r.rect.y * scale + offsetY,
                    r.rect.w * scale,
                    r.rect.h * scale
                };
    
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderRect(renderer, &scaledRect);  // SDL3 поддерживает SDL_FRect*
            }
        }
        for (const auto& stroke : layer.strokes) {
            for (const Rect& r : stroke.rects) {
                r.draw(renderer, scale, offsetX, offsetY);
                if (&r == selected_rect) {
                    SDL_FRect scaledRect = {
                        r.rect.x * scale + offsetX,
                        r.rect.y * scale + offsetY,
                        r.rect.w * scale,
                        r.rect.h * scale
                    };
    
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    SDL_RenderRect(renderer, &scaledRect);  // SDL3 поддерживает SDL_FRect*
                }
            }
        }
    }

    // Показываем прямоугольник при растягивании
    if (isDragging && dragRect.w > 0 && dragRect.h > 0) {
        SDL_FRect preview = {
            dragRect.x,
            dragRect.y,
            dragRect.w,
            dragRect.h
        };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);  // полупрозрачный зелёный
        SDL_RenderFillRect(renderer, &preview);
        
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);  // рамка
        SDL_RenderFillRect(renderer, &preview);
    }

    if (isBrushing && current_tool == Tool::Brush) {
        SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255); // красный
    
        for (const auto& stroke : brushStrokes) {
            int c = 0;
            for (const auto& circle : stroke.circles) {
                c += 1;
                if (c == 1) continue;
                drawCircle(renderer, circle.x, circle.y, circle.radius);
            }
        }
    }
    


    SDL_RenderPresent(renderer);
}

void Editor::importImage(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        SDL_Log("Load error!");
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    int w = surface->w;
    int h = surface->h;
    SDL_DestroySurface(surface);

    // создаём слой
    Layer newLayer;
    newLayer.name = "Image Layer";
    newLayer.visible = true;
    newLayer.canvasWidth = w;
    newLayer.canvasHeight = h;

    // добавляем фон
    Drawable* bg = new DrawableImageBackground(texture, w, h);
    newLayer.objects.push_back(bg);

    layers.push_back(std::move(newLayer));
    active_layer = layers.size() - 1;

    // сброс масштаба и смещения
    scale = 1.0f;
    offsetX = 0;
    offsetY = 0;
}
