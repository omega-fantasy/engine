#ifndef UI_H
#define UI_H

#include "engine.h"
#include "screen.h"
#include "texture.h"
#include "input.h"

class ProgressBar : public Composite {
    public:
        ProgressBar(Size sz, Color fg, Color bg): Composite(sz), color_fg(fg), color_bg(bg) {
            m_texture = new Texture(bg, sz);   
        }
        ~ProgressBar() { delete m_texture; }

        void set_progress(int progress) {
            short cutoff = ((double)progress / 100) * size.w;
            Color* pixels = (Color*)m_texture->pixels();
            for (short y = 0; y < size.h; y++) {
                for (short x = 0; x < size.w; x++) {
                    pixels[y * size.w + x] = x <= cutoff ? color_fg : color_bg;
                }
            }
        }

        Color color_fg;
        Color color_bg;
};

class Text : public Composite {
    public:
        Text(const std::string& txt, short txt_height, Size sz): Composite(sz) {
            set_text(txt, txt_height);
        }

        void set_text(const std::string& txt, short txt_height) {
            lines.clear();
            lines.resize(1);
            std::vector<Texture*> word;
            int current_line = 0;
            short word_length = 0;
            short line_length = 0;
            short line_height = 0;
            short total_height = 0;

            for (int i = 0; i < (int)txt.size(); i++) {
                bool newline = i < (int)txt.size()-1 ? txt[i] == 10 : false;
                if (newline) {
                    total_height += line_height;
                    lines[current_line].insert(lines[current_line].end(), word.begin(), word.end());
                    lines.push_back(std::vector<Texture*>());
                    line_length = 0;
                    line_height = 0;
                    current_line++;
                    word.clear();
                    word_length = 0;
                    continue;
                }
                Texture* t = Engine.textures()->get(txt[i], txt_height);
                if (t) {
                    word.push_back(t);
                    word_length += t->size().w;
                    line_height = t->size().h > line_height ? t->size().h : line_height;
                }
                if (txt[i] == ' ' || i == (int)(txt.size()-1)) {
                    if (line_length + word_length < size.w) {
                        lines[current_line].insert(lines[current_line].end(), word.begin(), word.end());
                        line_length += word_length;
                    } else {
                        total_height += line_height;
                        if (total_height + line_height > size.h) { 
                            //std::cout << "Text too long!" << std::endl;
                            // add excess text handling here
                            break;
                        }
                        lines.push_back(word);
                        line_length = word_length;
                        line_height = 0;
                        current_line++;
                    }
                    word.clear();
                    word_length = 0;
                }
            }
        }

        void draw() {
            if (!initialized) {
                init();
                initialized = true;
            }
            if (needs_update()) {
                if (m_texture) {
                    Engine.screen()->blit(m_texture->pixels(1.0), m_texture->size(1.0), pos, Box(pos, size), false);
                }
                Point p = pos;
                for (auto& line : lines) {
                    short line_height = 0;
                    for (auto& letter : line) {
                        Engine.screen()->blit(letter->pixels(1.0), letter->size(1.0), p, Box(pos, size), true);
                        p.x += letter->size().w;
                        line_height = letter->size().h > line_height ? letter->size().h : line_height;
                    }
                    p.x = pos.x;
                    p.y += line_height;
                }
                set_update(false);
            }
            for (auto& child : children) {
                child->draw();
            }
        }

        std::vector<std::vector<Texture*>> lines;
};

class TextInput : public Text, public Input::Listener {
    public:
        TextInput(int max_length, Size sz): Text("_", 0.8 * sz.h, sz), max(max_length) { size = sz; }

        virtual void init() {
            std::vector<std::string> letters = {"Backspace", "Space"};
            for (char c = 0; c < 127; c++) { letters.push_back(std::string(1, c)); }
            Engine.input()->add_key_listeners(this, letters, true);
        }
        
        ~TextInput() { Engine.input()->clear_temp_listeners(); }

        virtual void key_pressed(const std::string& key) {
            Size old_size = size;
            if (key == "Backspace" && current_text.size() > 0) {
                current_text.pop_back();
                set_text(current_text + "_", 0.8 * size.h);
            } else if ((int)current_text.size() <= max) {
                if (key.size() == 1) {
                    char c = Engine.input()->shift_held() ? key[0] : (char)std::tolower(key[0]);
                    set_text(current_text + c + "_", 0.8 * size.h);
                    current_text += c;
                } else if (key == "Space") {
                    set_text(current_text + " _", 0.8 * size.h);
                    current_text += " ";
                }
            }
            size = old_size;
        }
    
        int max;
        std::string current_text;
};

class Button : public Composite, public Input::Listener {
    public:
        Button(Size sz, const std::string& text): Composite(sz), txt(text) { }
        ~Button() { Engine.input()->remove_mouse_listener(this); }

        void set_texture(Texture* t) {
            m_texture = t; 
            size = t->size();
        }

        void set_text(const std::string& s) { text_composite->set_text(s, size.h * 0.5); }
        
        void set_enabled(bool e) {
            if (enabled == e) {
                return;
            }
            if (e) {
                set_overlay(Color(0, 0, 0, 0));
                Engine.input()->add_mouse_listener(this, {pos, size});
            } else {
                set_overlay(Color(10, 10, 10, 200));
                Engine.input()->remove_mouse_listener(this);
            }
            enabled = e;
        }

        void draw() {
            if (!listener_registered) {
                if (!txt.empty()) {
                    text_composite = new Text(txt, size.h * 0.5, size);
                    add_child(text_composite, Point(10, 10));
                }
                Engine.input()->add_mouse_listener(this, {pos, size});
                listener_registered = true;
            }
            Composite::draw();
        }

        std::string txt;
        Text* text_composite = nullptr;
        bool listener_registered = false;
        bool enabled = true;
};

#endif
