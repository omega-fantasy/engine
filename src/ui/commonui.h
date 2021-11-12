#ifndef COMMONUI_H
#define COMMONUI_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/input.h"
#include "engine/tilemap.h"
#include "ui/boxtexture.h"

class BasicBox : public Composite {
    public:
    BasicBox(const std::string& txt): Composite({0, 0}), message(txt) { 
        Size s = Engine.map()->get_size();
        size = {1.0 * s.w, 0.35 * s.h};
        Engine.map()->add_child(this, {0.0, 0.65 * s.h}); 
        m_texture = new BoxTexture(size, {0, 0, 170}, {0, 0, 32}, {200, 200, 200});
    }
            
    virtual ~BasicBox() { 
        delete m_texture;
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

class ButtonWithTooltip : public Button {
    public:
   
    ButtonWithTooltip(Size sz, const std::string& text, const std::string& tip): Button(sz, text), tooltip_txt(tip) {}

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
            tooltip = new BasicBox(tooltip_txt);
        }
    };

    void init() {
        Engine.input()->add_move_listener(this);
    }

    std::string tooltip_txt;
    BasicBox* tooltip = nullptr;
};

#endif
