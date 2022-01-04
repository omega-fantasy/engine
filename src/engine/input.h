#ifndef INPUT_H
#define INPUT_H

#include "util.h"

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
        void add_key_listeners(Input::Listener* l, const std::vector<std::string>& keys, bool temp = false) {
            for (auto& key : keys) {
                if (temp) {
                    temp_presses[key] = l;
                } else {
                    presses[key] = l;
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
        bool shift_held() { return shift_active; }

        void handleInputs() {
            std::vector<std::string> pressed;
            std::vector<std::string> released;
            pressed_keys(pressed, released);
            for (auto& hold : held) {
                pressed.push_back(hold);
            }
            for (auto& key : pressed) {
                if (temp_presses.find(key) != temp_presses.end() && !clear_temps) {
                    temp_presses[key]->key_pressed(key);
                } else if (enabled && presses.find(key) != presses.end()) {
                    presses[key]->key_pressed(key);
                    if (key != "WheelUp" && key != "WheelDown") {
                        held.insert(key);
                    }
                } else if (key == "MouseLeft") {
                    Point p = mouse_pos();
                    std::map<Input::Listener*, Box>& active_clicks = enabled ? clicks : temp_clicks;
                    for (auto& click : active_clicks) {
                        auto& box = click.second;
                        if (p.x >= box.a.x && p.y >= box.a.y && p.x <= box.b.x && p.y <= box.b.y) {
                            if (enabled || !clear_temps) {
                                click.first->mouse_clicked(p);
                            }
                        }
                    }
                }
                if (key == "Left Shift") {
                    shift_active = true;
                }
            }
            for (auto& key : released) {
                if (key == "Left Shift") {
                    shift_active = false;
                }
                held.erase(key);
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
        std::set<std::string> held;
        std::set<Input::Listener*> mouse_moves;
        std::map<std::string, Input::Listener*> temp_presses;
        std::map<std::string, Input::Listener*> presses;
        std::map<Input::Listener*, Box> temp_clicks;
        std::map<Input::Listener*, Box> clicks;
        std::vector<Input::Listener*> remove_list;
        bool enabled = true;
        bool clear_temps = false;
        bool shift_active = false;
        Point last_mouse_pos;
};

#endif
