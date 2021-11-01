#ifndef TOWNSMENU_H
#define TOWNSMENU_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/tilemap.h"
#include "system/system.h"
#include "system/player.h"
#include "system/buildings.h"

class TownsMenu : public Composite {
    public:
    TownsMenu(Size sz): Composite(sz) {}
    ~TownsMenu() {
        for (auto child : children) {
            delete child;
        }
    }
    
    private:
    class TownButton : public Button {
        public:
            TownButton(const std::string& name, Point p): Button({0, 0}, name), pos(p) {}
            void mouse_clicked(Point) {
                Engine.map()->move_cam_to_tile(pos);
            }
            Point pos;
    };

    class WorldName : public Text {
        public:
            WorldName(const std::string& name, Size s): Text("World: " + name, 0.8 * s.h, s) {}
            void init() { m_texture = new Texture(0xFF555555, {size.w, size.h}); }
    };

    void init() {
        unsigned button_color = 0xFF555555;
        Size button_size = {0.8 * size.w, 0.08 * size.h};
        auto worldtext = new WorldName(System.player()->worldname(), button_size);
        add_child(worldtext, {0.1 * size.w, 0.1 * size.h});
        int i = 2;
        for (auto& town : System.buildings()->townlist()) {
            auto button = new TownButton(town.first, town.second);
            add_child(button, {0.1 * size.w, 0.1 * i * size.h});
            button->set_texture(new Texture(button_color, button_size));
            i++;
        }
    }
};

#endif
