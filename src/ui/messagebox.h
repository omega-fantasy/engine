#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/input.h"

class MessageBox : public Composite, public Input::Listener {
    public:
    class Listener {
        public:
            virtual void confirmed(MessageBox*) {}
    };
   
   MessageBox(Size sz, const std::string& text, MessageBox::Listener* l = nullptr): Composite(sz), message(text), listener(l) {
        m_texture = new Texture(0xFF000080, size); 
    }
            
    virtual void key_pressed(const std::string& key) {
        if (key == "Return") {
            if (listener) {
                listener->confirmed(this);
            }
        }
    }

    virtual ~MessageBox() {
        delete text;
        Engine.input()->clear_temp_listeners();
        Engine.input()->enable();
    }

    void init() {
        Engine.input()->add_key_listeners(this, {"Return"}, false, true);
        Engine.input()->disable();
        text = new Text(message, 0.1 * size.h, {0.95 * size.w, 0.9 * size.h});
        add_child(text, {0.025 * size.w, 0.1 * size.h});
    }

    std::string message;
    Text* text = nullptr;
    Listener* listener = nullptr;
};

#endif
