#include "ui/hud.h"
#include "ui/buildingsmenu.h"
#include "ui/townsmenu.h"
#include "ui/researchmenu.h"
#include "ui/timewidget.h"
#include "ui/minimap.h"
#include "ui/boxtexture.h"
#include "engine/scripting.h"

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

class ResearchButton : public Button {
    public:
        ResearchButton(HUD* p): Button({0, 0}, "Research"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Size s = parent->get_size();
            ResearchMenu* menu = new ResearchMenu({s.w * 1.0, s.h * 0.5});
            parent->change_layout({{menu, {0,0}}});
        }
        HUD* parent;
};

class EventButton : public Button {
    public:
        EventButton(HUD* p): Button({0, 0}, "Events"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
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

class FPSButton : public Button {
    public:
        FPSButton(): Button({0, 0}, "0") {}
        void draw() {
            if (listener_registered) {
                set_text("FPS: " + std::to_string(Engine.screen()->fps()));
            }
            Button::draw();
        }
};

class TestScriptButton : public Button, public MessageBox::Listener {
    public:
        TestScriptButton(): Button({0, 0}, "Test Script") {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Engine.script()->execute("./scripts/test.script");
            Size s = Engine.map()->get_size();
            std::string text = "Test Script Output:\n \n" + Engine.script()->script_output();
            auto messagebox = new MessageBox({1.0 * s.w, 1.0 * s.h}, text, this, 0.025);
            Engine.map()->add_child(messagebox, {0.0 * s.w, 0.0 * s.h});
        }

        virtual void confirmed(MessageBox* messagebox) {
            Engine.map()->remove_child(messagebox);
            delete messagebox;
        }
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
    m_texture = new BoxTexture(s, {0, 0, 170}, {0, 0, 32}, {200, 200, 200});
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
    add_child(mini_map, {(double)(size.w - mini_map->get_size().w) / 2, (double)(size.h - 0.05 * size.h - mini_map->get_size().h)});
}

std::vector<std::pair<Composite*, Point>> HUD::create_standard_layout() {
    std::vector<std::pair<Composite*, Point>> ret;
    std::vector<Button*> buttons = {new BuildingsButton(this), new ResearchButton(this), new EventButton(this), new TownsButton(this), new SimulateButton(), new SaveButton(), new LoadButton(), new TestScriptButton()};
    for (int i = 0; i < (int)buttons.size(); i++) {
        buttons[i]->set_texture(new Texture(0xFF555555, {0.8 * size.w, 0.06 * size.h}));
        ret.push_back({buttons[i], {0.1 * size.w, (i+1) * 0.07 * size.h}});
    }
    return ret;
}

void HUD::init() {
    Size mini_map_size = {256, 256};
    mini_map = new MiniMap(mini_map_size);
    change_layout(create_standard_layout(), false);    
}
