#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/input.h"
#include "engine/config.h"
#include "ui/commonui.h"

class MessageBox : public BasicBox, public Input::Listener {
    public:
    class Listener {
        public:
            virtual void confirmed(MessageBox*) {}
    };
   
    MessageBox(Size sz, const std::string& txt, MessageBox::Listener* l = nullptr, double height_factor = 0.1): BasicBox(sz), message(txt), listener(l), text_height_factor(height_factor) {}

    virtual void mouse_clicked(Point) {
        if (listener) {
            listener->confirmed(this);
        }
    }
            
    virtual void key_pressed(const std::string& key) {
        if (key == "Return" && listener) {
            listener->confirmed(this);
        }
    }

    virtual ~MessageBox() {
        delete text;
        delete m_texture;
        Engine.input()->clear_temp_listeners();
        Engine.input()->enable();
    }

    void init() {
        Engine.input()->add_key_listeners(this, {"Return"}, true);
        Engine.input()->add_mouse_listener(this, {pos, size}, true);
        Engine.input()->disable();
        text = new Text(message, text_height_factor * size.h, {0.95 * size.w, 0.9 * size.h});
        add_child(text, {0.025 * size.w, 0.1 * size.h});
    }

    std::string message;
    Text* text = nullptr;
    Listener* listener = nullptr;
    double text_height_factor = 0.1;
};

#endif
