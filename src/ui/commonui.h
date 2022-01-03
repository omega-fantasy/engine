#ifndef COMMONUI_H
#define COMMONUI_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/config.h"
#include "engine/input.h"
#include "engine/tilemap.h"
#include "ui/boxtexture.h"

class BasicBox : public Composite {
    public:
    BasicBox(Size s): Composite(s) {
        auto& cfg = Engine.config()->get("settings")["colors"];
        m_texture = new BoxTexture(size, cfg["box_topleft"].c(), cfg["box_bottomright"].c(), cfg["box_border"].c());
    }
    virtual ~BasicBox() {}
};

class BasicTextBox : public BasicBox {
    public:
    BasicTextBox(const std::string& txt): BasicBox({1.0 * Engine.map()->get_size().w, 0.35 * Engine.map()->get_size().h}), message(txt) { 
        Engine.map()->add_child(this, {0.0, 0.65 * Engine.map()->get_size().h});
    }
            
    virtual ~BasicTextBox() { 
        delete text;
        Engine.map()->remove_child(this); 
    }

    void init() {
        text = new Text(message, 0.1 * size.h, {0.95 * size.w, 0.9 * size.h});
        add_child(text, {0.025 * size.w, 0.1 * size.h});
    }

    std::string message;
    Text* text = nullptr;
};

class BasicButton : public Button {
    public:
    BasicButton(Size sz, const std::string& text): Button(sz, text) {}
    virtual ~BasicButton() {
        delete texture_default;
        delete texture_selected;
        m_texture = nullptr;
    }
    virtual void init() {
        auto& cfg = Engine.config()->get("settings")["colors"];
        texture_default = new BoxTexture(size, cfg["button_topleft"].c(), cfg["button_bottomright"].c(), cfg["button_border"].c());
        texture_selected = new BoxTexture(size, cfg["button_selected"].c(), cfg["button_selected"].c(), cfg["button_border"].c());
        set_selected(false);
    }
    void set_selected(bool selected) { set_texture(selected ? texture_selected : texture_default);}
    Texture* texture_default = nullptr;
    Texture* texture_selected = nullptr;
};

class ButtonWithTooltip : public BasicButton {
    public:
    ButtonWithTooltip(Size sz, const std::string& text, const std::string& tip): BasicButton(sz, text), tooltip_txt(tip) {}

    virtual ~ButtonWithTooltip() { 
        delete tooltip; 
        Engine.input()->remove_move_listener(this);
    }
            
    virtual void mouse_moved(Point p) {
        Box extent(pos, size);
        if (tooltip && !extent.inside(p)) {
            delete tooltip;
            tooltip = nullptr;
        } else if (!tooltip && extent.inside(p)) {
            tooltip = new BasicTextBox(tooltip_txt);
        }
    };

    virtual void init() {
        BasicButton::init();
        Engine.input()->add_move_listener(this);
    }

    std::string tooltip_txt;
    BasicTextBox* tooltip = nullptr;
};

#endif
