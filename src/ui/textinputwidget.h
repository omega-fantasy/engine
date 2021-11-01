#ifndef TEXTINPUTWIDGET_H
#define TEXTINPUTWIDGET_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/input.h"

class TextInputWidget : public Composite {
    public:
    class Listener {
        public:
            virtual void confirmed(TextInputWidget*) {};
    };
    
    class ConfirmButton : public Button {
        public:
            ConfirmButton(TextInputWidget* p): Button({0, 0}, "Confirm"), parent(p) {}
            virtual ~ConfirmButton() {}
            void mouse_clicked(Point) {
                if (!parent) {
                    delete this;
                } else {
                    Engine.audio()->play_sound("menu2");
                    if (parent->listener) {
                        parent->listener->confirmed(parent);
                    }
                }
            }
            void init() {
                Engine.input()->add_mouse_listener(this, {pos, size}, true);
            }

            TextInputWidget* parent = nullptr;
    };
   
   TextInputWidget(Size sz, const std::string& text, TextInputWidget::Listener* l = nullptr): Composite(sz), title(text), listener(l) {
        m_texture = new Texture(0xFF000000, size); 
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
        title_text = new Text(title, 0.15 * size.h, {1.0 * size.w, 0.2 * size.h});
        add_child(title_text, {0.0 * size.w, 0.0 * size.h});
        input = new TextInput(12, {1.0 * size.w, 0.2 * size.h});
        add_child(input, {0.0 * size.w, 0.33 * size.h});

        button = new ConfirmButton(this);
        button->set_texture(new Texture(0xFF555555, {1.0 * size.w, 0.2 * size.h}));
        add_child(button, {0.0 * size.w, 0.8 * size.h});
    }

    std::string title;
    Text* title_text = nullptr;
    TextInput* input = nullptr;
    ConfirmButton* button = nullptr;
    Listener* listener = nullptr;
};

#endif
