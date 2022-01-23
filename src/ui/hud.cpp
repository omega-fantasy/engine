#include "ui/hud.h"
#include "ui/buildingsmenu.h"
#include "ui/townsmenu.h"
#include "ui/researchmenu.h"
#include "ui/timewidget.h"
#include "ui/minimap.h"
#include "ui/commonui.h"
#include "engine/scripting.h"

class BackButton : public BasicButton {
    public:
        BackButton(HUD* p, Size s): BasicButton(s, "<- Back"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            parent->change_layout(parent->create_standard_layout(), false);
        }
        HUD* parent;
};

class BuildingsButton : public BasicButton {
    public:
        BuildingsButton(HUD* p, Size s): BasicButton(s, "Build"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Size s = parent->get_size();
            BuildingsMenu* menu = new BuildingsMenu({s.w * 1.0, s.h * 0.5});
            parent->change_layout({{menu, {0,0}}});
        }
        HUD* parent;
};

class ResearchButton : public BasicButton {
    public:
        ResearchButton(HUD* p, Size s): BasicButton(s, "Research"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Size s = parent->get_size();
            ResearchMenu* menu = new ResearchMenu({s.w * 1.0, s.h * 0.5});
            parent->change_layout({{menu, {0,0}}});
        }
        HUD* parent;
};

class EventButton : public BasicButton {
    public:
        EventButton(HUD* p, Size s): BasicButton(s, "Events"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
        }
        HUD* parent;
};

class TownsButton : public BasicButton {
    public:
        TownsButton(HUD* p, Size s): BasicButton(s, "Towns"), parent(p) {}
        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            Size s = parent->get_size();
            TownsMenu* menu = new TownsMenu({s.w * 1.0, s.h * 0.5});
            parent->change_layout({{menu, {0,0}}});
        }
        HUD* parent;
};

class FPSButton : public BasicButton {
    public:
        FPSButton(Size s): BasicButton(s, "0") {}
        void draw() {
            if (listener_registered) {
                int current_fps = Engine.screen()->fps();
                if (current_fps != last_fps) {
                    current_fps = current_fps > 1000 ? 1000: current_fps;
                    set_text("FPS: " + std::to_string(current_fps));
                    last_fps = current_fps;
                }
            }
            BasicButton::draw();
        }
        int last_fps = -1;
};

class BenchmarkButton : public BasicButton {
    public:
        BenchmarkButton(Size s): BasicButton(s, "Benchmark") {}
        void mouse_clicked(Point) {
            Engine.map()->set_zoom(0.125);
            start = true;
            t_start = now();
            cur_frames = 0;
        }
        void draw() {
            if (start) {
                cur_frames++;
                if (cur_frames == max_frames) {
                    int avg_fps = 1.0 / ((double)(now()-t_start) / (1000*1000*max_frames));
                    std::string txt = "Result: " + std::to_string(avg_fps) + "fps";
                    set_text(txt);
                    print(txt);
                } else if (cur_frames < max_frames) {
                    Engine.map()->move_cam({10, 10});
                } else {
                    start = false;
                }
            }
            BasicButton::draw();
        }
        bool start = false;
        long long t_start = 0;
        int max_frames = 500;
        int cur_frames = 0;
};

class TestScriptButton : public BasicButton, public MessageBox::Listener {
    public:
        TestScriptButton(Size s): BasicButton(s, "Test Script") {}
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

class NewButton : public BasicButton {
    public:
        NewButton(Size s): BasicButton(s, "New Map") {}
        void mouse_clicked(Point) {
            Engine.map()->randomize_map();
            Engine.screen()->set_update(true);
            Engine.audio()->play_sound("menu2");
            System.init();
        }
};

class SaveButton : public BasicButton {
    public:
        SaveButton(Size s): BasicButton(s, "Save Game") {}
        void mouse_clicked(Point) { 
            Engine.save_state("state.sav");
            Engine.audio()->play_sound("menu2");
        }
};

class LoadButton : public BasicButton {
    public:
        LoadButton(Size s): BasicButton(s, "Load Game") {}
        void mouse_clicked(Point) { 
            if (file_exists("state.sav")) {
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
        add_child(new BackButton(this, {0.8 * size.w, 0.06 * size.h}), {0.1 * size.w, 0.01 * size.h});
        start.y += 0.1 * size.h;
    }
    for (auto child : new_layout) {
        Point p = child.second + start;
        add_child(child.first, p);
    }
    add_child(mini_map, {(double)(size.w - mini_map->get_size().w) / 2, (double)(size.h - 0.05 * size.h - mini_map->get_size().h)});
}

std::vector<std::pair<Composite*, Point>> HUD::create_standard_layout() {
    std::vector<std::pair<Composite*, Point>> ret;
    Size s(0.8 * size.w, 0.06 * size.h);
    std::vector<Button*> buttons = {new BuildingsButton(this, s), new ResearchButton(this, s), new EventButton(this, s), new TownsButton(this, s), new SimulateButton(s), new NewButton(s), new SaveButton(s), new LoadButton(s), new TestScriptButton(s), new BenchmarkButton(s)};
    for (int i = 0; i < (int)buttons.size(); i++) {
        ret.push_back({buttons[i], {0.1 * size.w, (i+0.1) * 0.06 * size.h}});
    }
    return ret;
}

void HUD::init() {
    Size mini_map_size = {256, 256};
    mini_map = new MiniMap(mini_map_size);
    change_layout(create_standard_layout(), false);    
}
