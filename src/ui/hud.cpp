#include "ui/hud.h"
#include "ui/buildingsmenu.h"
#include "ui/townsmenu.h"
#include "ui/timewidget.h"
#include "ui/minimap.h"

class BackButton : public Button {
    public:
        BackButton(HUD* p): Button({0, 0}, "<- Back"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            parent->change_layout(parent->create_standard_layout(), false);
        }
        HUD* parent;
};

class BuildingsButton : public Button {
    public:
        BuildingsButton(HUD* p): Button({0, 0}, "Build"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Size s = parent->get_size();
            BuildingsMenu* menu = new BuildingsMenu({s.w * 1.0, s.h * 0.5});
            parent->change_layout({{menu, {0,0}}});
        }
        HUD* parent;
};

class TownsButton : public Button {
    public:
        TownsButton(HUD* p): Button({0, 0}, "Towns"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Size s = parent->get_size();
            TownsMenu* menu = new TownsMenu({s.w * 1.0, s.h * 0.5});
            parent->change_layout({{menu, {0,0}}});
        }
        HUD* parent;
};

/*
class NewButton : public Button {
    public:
        NewButton(): Button({0, 0}, "New Game") {}
        void mouse_clicked(Point) {
            Engine.map()->randomize_map();
            Engine.screen()->set_update(true);
            Engine.sim()->reset();
            Engine.audio()->play_sound("menu2");
            System.init();
        }
};
*/

class SaveButton : public Button {
    public:
        SaveButton(): Button({0, 0}, "Save Game") {}
        void mouse_clicked(Point) { 
            Engine.save_state("state.sav");
            Engine.audio()->play_sound("menu2");
        }
};

class LoadButton : public Button {
    public:
        LoadButton(): Button({0, 0}, "Load Game") {}
        void mouse_clicked(Point) { 
            std::ifstream ifile("state.sav");
            if (ifile) {
                Engine.load_state("state.sav");
                Engine.audio()->play_sound("menu2");
            }
        }
};

HUD::HUD(Size s): Composite(s) {
    m_texture = new Texture(0x00000080, s);
}

void HUD::change_layout(const std::vector<std::pair<Composite*, Point>>& new_layout, bool back) {
    for (auto child : children) {
        if (child != mini_map) {
            delete child;
        }
    }
    children.clear();
    Point start = {0, 0};
    if (back) {
        Texture* texture_button = new Texture(0xFF555555, {0.8 * size.w, 0.08 * size.h});
        auto back_button = new BackButton(this);
        back_button->set_texture(texture_button);
        add_child(back_button, {0.1 * size.w, 0.0 * size.h});
        start.y += 0.15 * size.h;
    }
    for (auto child : new_layout) {
        Point p = child.second + start;
        add_child(child.first, p);
    }
    add_child(mini_map, {(double)(size.w - mini_map->get_size().w) / 2, (double)(size.h - mini_map->get_size().h)});
}

std::vector<std::pair<Composite*, Point>> HUD::create_standard_layout() {
    std::vector<std::pair<Composite*, Point>> ret;
    std::vector<Button*> buttons = {new BuildingsButton(this), new TownsButton(this), new SimulateButton(), new SaveButton(), new LoadButton()};
    for (int i = 0; i < (int)buttons.size(); i++) {
        buttons[i]->set_texture(new Texture(0xFF555555, {0.8 * size.w, 0.08 * size.h}));
        ret.push_back({buttons[i], {0.1 * size.w, (i+1) * 0.1 * size.h}});
    }
    return ret;
}

void HUD::init() {
    Size mini_map_size = {256, 256};
    mini_map = new MiniMap(mini_map_size);
    change_layout(create_standard_layout(), false);    
}
