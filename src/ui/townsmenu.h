#ifndef TOWNSMENU_H
#define TOWNSMENU_H

#include "ui/commonui.h"
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
    class TownButton : public BasicButton {
        public:
            TownButton(const std::string& name, Point p, Size s): BasicButton(s, name), pos(p) {}
            void mouse_clicked(Point) {
                Engine.map()->move_cam_to_tile(pos);
            }
            Point pos;
    };

    void init() {
        Size button_size = {0.8 * size.w, 0.1 * size.h};
        add_child(new BasicButton(button_size, "World: " + System.player()->worldname()), {0.1 * size.w, 0.1 * size.h});
        int i = 2;
        for (auto& town : System.buildings()->townlist()) {
            add_child(new TownButton(town.first, town.second, button_size), {0.1 * size.w, 0.1 * i * size.h});
            i++;
        }
    }
};

#endif
