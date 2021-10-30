#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "engine/db.h"
#include "system/player.h"
#include "system/system.h"
    
class SimulateButton : public Button, public Composite::Listener {
    public:
        SimulateButton(): Button({0, 0}, "Next Turn") {}
        
        virtual void fade_completed(Composite*) {
            if (dark) {
                Engine.map()->set_zoom(1.0);
                Point current = towns.back();
                Engine.map()->move_cam_to_tile(current);
                Engine.screen()->set_overlay(0x00000000, 120, this);
                dark = false;
            } else {
                Point current = towns.back();
                towns.pop_back();
                Engine.audio()->play_sound("menu2");
                auto& town = Engine.db()->get_table<Buildings::Town>("towns")->get(current);
                for (auto& building : town.buildings) {
                    if (building.x < 0 || building.y < 0) {
                        break;
                    }
                    System.player()->change_cash(100);
                }
                if (!towns.empty()) {
                    Engine.screen()->set_overlay(0xFF000000, 120, this);
                    dark = true;
                } else {
                    Engine.input()->enable();
                }
            }
        }

        void mouse_clicked(Point) {
            Engine.audio()->play_sound("menu1");
            towns.clear();
            auto town_table = Engine.db()->get_table<Buildings::Town>("towns");
            for (auto it = town_table->begin(); it != town_table->end(); ++it) {
                towns.push_back(it.key());
            }
            if (!towns.empty()) {
                Engine.input()->disable();
                Engine.screen()->set_overlay(0xFF000000, 120, this);
                dark = true;
            }
        }

        std::vector<Point> towns;
        bool dark = false;
};

/*
class TimeWidget : public Composite, Simulation::Event {
    class ToggleButton : public Button {
        public:
            ToggleButton(): Button({0, 0}, "Pause") {}
            
            void mouse_clicked(Point) {
                is_running = !is_running;
                Engine.sim()->toggle(is_running);
                if (is_running) {
                    set_text("Pause");
                } else {
                    set_text("Run");
                }
                Engine.audio()->play_sound("menu1");
            }
            bool is_running = true;
    };

    public:
    TimeWidget(Size sz): Composite(sz) {
        m_texture = new Texture(0x00000000, sz);
        Engine.sim()->register_event("vegetation", this);
        Engine.sim()->queue_event("vegetation", 10);
    }

    void execute() {
        double destroy = 0.0005;
        double create  = 0.00025;
        Size tilemap_size = Engine.map()->tilemap_size(); 
        unsigned seed = (unsigned)(std::chrono::system_clock::now().time_since_epoch().count());
        auto generator = std::default_random_engine(seed);
        std::uniform_int_distribution<short> distribution(0, tilemap_size.w - 1);
        for (int i = 0; i < destroy * tilemap_size.w * tilemap_size.h; i++) {
            Engine.map()->unset_tile({distribution(generator) ,distribution(generator)}); 
        }
        for (int i = 0; i < create * tilemap_size.w * tilemap_size.h; i++) {
            Engine.map()->set_tile("flower", {distribution(generator) ,distribution(generator)}); 
        }
        Engine.sim()->queue_event("vegetation", 10);
    }

    void init() {
        text_time = new Text("", size.h * 0.7);
        add_child(text_time, {0.35 * size.w, 0.35 * size.h});
        Button* button_toggle = new ToggleButton();
        button_toggle->set_texture(new Texture(0xFFAA0000, {0.3 * size.w, 0.5 * size.h}));
        add_child(button_toggle, {0.0 * size.w, 0.25 * size.h});
    }

    void draw() {
        if (Engine.sim()->running() && advance_sim++ == 30) {
            advance_sim = 0;
            Engine.sim()->step(1);
            int simtime = Engine.sim()->simtime();
            int year = 100;              // years since 1900, i.e. 2000
            int mon = 0;                 // note: zero indexed
            int day = 1 + simtime;       // note: not zero indexed
            struct tm newdate;
            newdate.tm_sec = 0; newdate.tm_min = 0; newdate.tm_hour = 1;
            newdate.tm_mday = day; newdate.tm_mon = mon; newdate.tm_year = year;
            time_t date_seconds = mktime(&newdate);
            char buffer[64];
            strftime(buffer, 64, "%F", gmtime(&date_seconds));
            text_time->set_text(buffer, 0.3 * size.h);
            set_update(true);
        }
        Composite::draw();
    }

    Text* text_time = nullptr;
    int advance_sim = 0;
};
*/

#endif
