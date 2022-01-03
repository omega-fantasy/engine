#ifndef TEXTINPUTWIDGET_H
#define TEXTINPUTWIDGET_H

#include "ui/commonui.h"
#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/input.h"

class TextInputWidget : public BasicBox {
    public:
    class Listener {
        public:
            virtual void confirmed(TextInputWidget*) {};
    };
    
    class ConfirmButton : public BasicButton {
        public:
            ConfirmButton(TextInputWidget* p, Size s): BasicButton(s, "Confirm"), parent(p) {}
            virtual ~ConfirmButton() {}
            virtual void mouse_clicked(Point) {
                if (!parent) {
                    delete this;
                } else {
                    Engine.audio()->play_sound("menu2");
                    if (parent->listener) {
                        parent->listener->confirmed(parent);
                    }
                }
            }
            virtual void init() {
                BasicButton::init();
                Engine.input()->add_mouse_listener(this, {pos, size}, true);
            }
            TextInputWidget* parent = nullptr;
    };
   
   TextInputWidget(Size sz, const std::string& text, TextInputWidget::Listener* l = nullptr): BasicBox(sz), title(text), listener(l) {
        MAX_NO_UPDATES = 1;       
   }

    virtual ~TextInputWidget() {
        delete title_text;
        delete input;
        button->parent = nullptr;
        Engine.input()->enable();
    }
    
    std::string current_text() { return input->current_text; }

    void init() {
        Engine.input()->disable();
        title_text = new Text(title, 0.15 * size.h, {0.8 * size.w, 0.2 * size.h});
        add_child(title_text, {0.025 * size.w, 0.1 * size.h});
        input = new TextInput(12, {1.0 * size.w, 0.2 * size.h});
        add_child(input, {0.025 * size.w, 0.33 * size.h});
        button = new ConfirmButton(this, {1.0 * size.w, 0.2 * size.h});
        add_child(button, {0.0 * size.w, 0.8 * size.h});
    }

    std::string title;
    Text* title_text = nullptr;
    TextInput* input = nullptr;
    ConfirmButton* button = nullptr;
    Listener* listener = nullptr;
};

#endif
