#ifndef UI_H
#define UI_H

#include "engine.h"
#include "screen.h"
#include "texture.h"
#include "input.h"

class Text : public Composite {
    public:
        Text(const std::string& txt, short txt_height): Composite({0, 0}) {
            set_text(txt, txt_height);
        }

        void set_text(const std::string& txt, short txt_size) {
            letters.clear();
            size = {0, 0};
            for (auto& letter : txt) {
                Texture* t = Engine.textures()->get(letter, txt_size);
                letters.push_back(t);
                size.w += t->size().w;
                if (t->size().h > size.h) { 
                    size.h = t->size().h;
                }
            }
        }

        void draw() {
            if (needs_update()) {
                Point p = pos; 
                for (auto& letter : letters) {
                    Engine.screen()->blit(letter, p, Box(p, letter->size()));
                    p.x += letter->size().w;
                }
                set_update(false);
            }
            Composite::draw();
        }

        std::vector<Texture*> letters;
};

class Button : public Composite, Input::Listener {
    public:
        Button(Size sz, const std::string& text): Composite(sz), txt(text) { }
        ~Button() { Engine.input()->remove_mouse_listener(this); }

        void set_texture(Texture* t) { 
            m_texture = t; 
            size = t->size();
        }

        void set_text(const std::string& s) { text_composite->set_text(s, size.h * 0.5); }
        
        void draw() {
            if (!listener_registered) {
                if (!txt.empty()) {
                    text_composite = new Text(txt, size.h * 0.5);
                    add_child(text_composite, Point(10, 10));
                }
                Engine.input()->addMouseClickListener(this, {pos, size});
                listener_registered = true;
            }
            Composite::draw();
        }

        std::string txt;
        Text* text_composite = nullptr;
        bool listener_registered = false;
};

#endif
