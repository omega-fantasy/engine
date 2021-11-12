#ifndef INPUT_H
#define INPUT_H

#include "util.h"
#include <SDL2/SDL.h>

class Input {
    public:
        class Listener {
            public:
            virtual void mouse_clicked(Point) {};
            virtual void mouse_moved(Point) {};
            virtual void key_pressed(const std::string& /*key*/) {};
            //virtual void keyReleased(const std::string& /*key*/) {};
        };

        void enable() { enabled = true; }
        void disable() { enabled = false;}
        void clear_temp_listeners() { clear_temps = true; }
        void remove_mouse_listener(Input::Listener* l) { remove_list.push_back(l); }

        void add_key_listeners(Input::Listener* l, const std::vector<std::string>& keys, bool hold = false, bool temp = false) {
            for (auto& key : keys) {
                if (hold) {
                    addKeyHoldListener(l, key);
                } else {
                    addKeyPressListener(l, key, temp);
                }
            }
        }
        
        void add_move_listener(Input::Listener* l) { mouse_moves.insert(l); }
        void remove_move_listener(Input::Listener* l) { mouse_moves.erase(mouse_moves.find(l)); };

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
                            Point p = mouse_pos();
                            for (auto& click : temp_clicks) {
                                auto& box = click.second;
                                if (p.x >= box.a.x && p.y >= box.a.y && p.x <= box.b.x && p.y <= box.b.y) {
                                    if (!clear_temps) {
                                        click.first->mouse_clicked(p);
                                    }
                                }
                            }
                            if (enabled) {
                                for (auto& click : clicks) {
                                    auto& box = click.second;
                                    if (p.x >= box.a.x && p.y >= box.a.y && p.x <= box.b.x && p.y <= box.b.y) {
                                        click.first->mouse_clicked(p);
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

            Point current_mouse_pos = mouse_pos();
            if (enabled && current_mouse_pos != last_mouse_pos) {
                for (auto& l : mouse_moves) {
                    l->mouse_moved(current_mouse_pos);
                }
                last_mouse_pos = current_mouse_pos; 
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
        std::set<Input::Listener*> mouse_moves;
        std::map<SDL_Keycode, Input::Listener*> temp_presses;
        std::map<SDL_Keycode, Input::Listener*> presses;
        std::map<SDL_Scancode, Input::Listener*> holds;
        std::map<Input::Listener*, Box> temp_clicks;
        std::map<Input::Listener*, Box> clicks;
        std::vector<Input::Listener*> remove_list;
        bool enabled = true;
        bool clear_temps = false;
        Point last_mouse_pos;

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
