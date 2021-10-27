#ifndef INPUT_H
#define INPUT_H

#include "geometry.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <unordered_map>

class Input {
    public:
        class Listener {
            public:
            virtual void mouse_clicked(Point) {};
            virtual void key_pressed(const std::string& /*key*/) {};
            //virtual void keyReleased(const std::string& /*key*/) {};
        };

        void enable() {
            enabled = true;
        }

        void disable() {
            enabled = false;
        }

        Point mouse_pos() {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            return {mx, my};
        }

        void clear_temp_listeners() {
            clear_temps = true;
        }

        void add_key_listeners(Input::Listener* l, const std::vector<std::string>& keys, bool hold = false, bool temp = false) {
            for (auto& key : keys) {
                if (hold) {
                    addKeyHoldListener(l, key);
                } else {
                    addKeyPressListener(l, key, temp);
                }
            }
        }

        void remove_mouse_listener(Input::Listener* l) {
            remove_list.push_back(l);
        }

        void add_mouse_listener(Input::Listener* l, Box b, bool temp = false) {
            if (temp) {
                temp_clicks[l] = b;
            } else { 
                if (clicks.find(l) == clicks.end()) {
                    clicks[l] = b;
                } 
            }
        }

        void handleInputs() {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_MOUSEWHEEL: {
                        if (enabled) {
                            if (event.wheel.y > 0 && presses.find(1) != presses.end()) { 
                                presses[1]->key_pressed("WheelUp");
                            } else if (event.wheel.y < 0 && presses.find(-1) != presses.end()) { 
                                presses[-1]->key_pressed("WheelDown");
                            }
                        }
                        break;
                    }
                    case SDL_KEYDOWN: {
                        auto find = temp_presses.find(event.key.keysym.sym);
                        if (find != temp_presses.end()) {
                            if (!clear_temps) {
                                find->second->key_pressed(SDL_GetKeyName(event.key.keysym.sym));
                            }
                            break;
                        }
                        find = presses.find(event.key.keysym.sym);
                        if (enabled && find != presses.end()) {
                            find->second->key_pressed(SDL_GetKeyName(event.key.keysym.sym));
                        }
                        break;
                    }
                    case SDL_MOUSEBUTTONDOWN: {
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            int mx, my;
                            SDL_GetMouseState(&mx, &my);
                            for (auto& click : temp_clicks) {
                                auto& box = click.second;
                                if (mx >= box.a.x && my >= box.a.y && mx <= box.b.x && my <= box.b.y) {
                                    if (!clear_temps) {
                                        click.first->mouse_clicked({mx, my});
                                    }
                                }
                            }
                            if (enabled) {
                                for (auto& click : clicks) {
                                    auto& box = click.second;
                                    if (mx >= box.a.x && my >= box.a.y && mx <= box.b.x && my <= box.b.y) {
                                        click.first->mouse_clicked({mx, my});
                                    }
                                }
                            }
                        }
                        break; 
                    }
                    case SDL_WINDOWEVENT: {
                        switch (event.window.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                            exit(0);
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            if (enabled) {
                auto keystate = SDL_GetKeyboardState(nullptr);
                for (auto& hold : holds) {
                    if (keystate[hold.first]) {
                        hold.second->key_pressed(SDL_GetScancodeName(hold.first));
                    }
                }
            }
                
            for (auto l : remove_list) {
                clicks.erase(l);
            }
            remove_list.clear();
            if (clear_temps) {
                temp_presses.clear();
                temp_clicks.clear();
                clear_temps = false;
            }
        }

    private:
        std::unordered_map<SDL_Keycode, Input::Listener*> temp_presses;
        std::unordered_map<SDL_Keycode, Input::Listener*> presses;
        std::unordered_map<SDL_Scancode, Input::Listener*> holds;
        std::unordered_map<Input::Listener*, Box> temp_clicks;
        std::unordered_map<Input::Listener*, Box> clicks;
        std::vector<Input::Listener*> remove_list;
        bool enabled = true;
        bool clear_temps = false;

        void addKeyPressListener(Input::Listener* l, const std::string& key, bool temp = false) {
            SDL_Keycode k = SDL_GetKeyFromName(key.c_str());
            if (key == "WheelUp") k = 1;
            if (key == "WheelDown") k = -1;
            if (temp) {
                if (k != SDLK_UNKNOWN && temp_presses.find(k) == temp_presses.end()) {
                    temp_presses[k] = l;
                    return;
                }
            } else if (k != SDLK_UNKNOWN && presses.find(k) == presses.end()) {
                presses[k] = l;
            }
        }
        
        void addKeyHoldListener(Input::Listener* l, const std::string& key) {
            auto k = SDL_GetScancodeFromName(key.c_str());
            if (k != SDL_SCANCODE_UNKNOWN && holds.find(k) == holds.end()) {
                holds[k] = l;
            }
        }
};

#endif
