#include "ui/hud.h"
#include "ui/buildingsmenu.h"
#include "ui/timewidget.h"
#include "ui/gamewidget.h"
#include "ui/minimap.h"

HUD::HUD(Size s): Composite(s) {
    m_texture = new Texture(0x00000080, s);
}

void HUD::init() {
    Size mini_map_size = {128, 128};
    MiniMap* mini_map = new MiniMap(mini_map_size);
    add_child(mini_map, {(double)(size.w - mini_map_size.w) / 2, (double)(size.h - mini_map_size.h)});
    GameWidget* state_menu = new GameWidget({size.w * 1.0, size.h * 0.15}); 
    add_child(state_menu, {0.0, 0.85 * size.h - mini_map_size.h});
    BuildingsMenu* menu = new BuildingsMenu({size.w * 1.0, size.h * 0.5});
    add_child(menu, {0, 0});
//    TimeWidget* time_widget = new TimeWidget({size.w * 0.8, size.h * 0.08});
//    add_child(time_widget, {0.1 * size.w, 0.92 * size.h - mini_map_size.h - state_menu->get_size().h});
    SimulateButton* sim_widget = new SimulateButton();
    sim_widget->set_texture(new Texture(0xFFAA0000, {size.w * 0.8, size.h * 0.05}));
    add_child(sim_widget, {0.1 * size.w, 0.92 * size.h - mini_map_size.h - state_menu->get_size().h});
}
