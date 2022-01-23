#ifndef MAINMENU_H
#define MAINMENU_H

#include "ui/hud.h"
#include "ui/textinputwidget.h"
#include "ui/messagebox.h"
#include "ui/commonui.h"
#include "engine/engine.h"
#include "engine/tilemap.h"
#include "engine/input.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "system/system.h"
#include "system/player.h"

class MapScreen : public Composite, public Composite::Listener, public MessageBox::Listener {
    class MapNavigate : public Input::Listener {
        public:
            MapNavigate() {
                accel = std::stoi(Engine.config()->get("settings")["movespeed"]);
                key_up = Engine.config()->get("settings")["keys"]["moveup"];
                key_down = Engine.config()->get("settings")["keys"]["movedown"];
                key_left = Engine.config()->get("settings")["keys"]["moveleft"];
                key_right = Engine.config()->get("settings")["keys"]["moveright"];
                key_zoomin = Engine.config()->get("settings")["keys"]["zoomin"];
                key_zoomout = Engine.config()->get("settings")["keys"]["zoomout"];
                key_quit = Engine.config()->get("settings")["keys"]["quit"];
                Engine.input()->add_key_listeners(this, {key_up, key_down, key_left, key_right});
                Engine.input()->add_key_listeners(this, {key_zoomin, key_zoomout, key_quit});
            }

            virtual void key_pressed(const std::string& key) {
                if (key == key_up) {
                    Engine.map()->move_cam({0, -accel});
                } else if (key == key_down) {
                    Engine.map()->move_cam({0, accel});
                } else if (key == key_left) {
                    Engine.map()->move_cam({-accel, 0});
                } else if (key == key_right) {
                    Engine.map()->move_cam({accel, 0});
                } else if (key == key_zoomin) {
                    Engine.map()->zoom_cam(2);
                } else if (key == key_zoomout) {
                    Engine.map()->zoom_cam(-2);
                } else if (key == key_quit) {
                    exit(0);
                }
            }

            int accel = 1;
            std::string key_up, key_down, key_left, key_right, key_zoomin, key_zoomout, key_quit;
    };
    public:

    MapScreen(): Composite({0, 0}) {
        Size resolution = Engine.screen()->get_size();
        double hud_width_per = 0.25;
        HUD* hud = new HUD({(double)(resolution.w * hud_width_per), (double)resolution.h});
        Engine.map()->create_map({(double)(resolution.w * (1-hud_width_per)), (double)resolution.h});
        new MapNavigate();
        Engine.screen()->add_child(Engine.map(), {(int)hud->get_size().w, 0});
        Engine.screen()->add_child(hud, {0, 0});
        Engine.screen()->set_update(true);
    }

    virtual void confirmed(MessageBox* messagebox) {
        Engine.map()->remove_child(messagebox);
        delete messagebox;
    }
    
    virtual void fade_completed(Composite*) {
        Size s = Engine.map()->get_size();
        std::string text = "Welcome to the world of " + System.player()->worldname() + "! This is just an example text to ensure that text boxes are working correctly... Press Enter to continue with the game!";
        auto messagebox = new MessageBox({1.0 * s.w, 0.35 * s.h}, text, this);
        Engine.map()->add_child(messagebox, {0.0 * s.w, 0.65 * s.h});
    }
};

class MainMenu : public Composite, TextInputWidget::Listener, Composite::Listener {
  public:
    MainMenu(Size sz): Composite(sz) {}
    
    virtual void fade_completed(Composite*) {
        Engine.screen()->clear();
        System.init();
        System.player()->set_worldname("Gaia");
        /*
        System.player()->set_worldname(m_widget->current_text());
        delete m_widget;
        Engine.input()->disable();
        */
        auto map_screen = new MapScreen();
        Engine.screen()->add_child(map_screen, {0, 0});
        Engine.map()->randomize_map();
        Engine.screen()->set_overlay(0x00000000, 10, map_screen);
        delete this;
    }   

    virtual void confirmed(TextInputWidget* widget) {
        m_widget = widget;
        Engine.screen()->set_overlay(0xFF000000, 10, this);
        Engine.input()->disable();
    };
    
  private:
    TextInputWidget* m_widget = nullptr;

    class NewButton : public BasicButton {
        public:
            NewButton(MainMenu* p, Size s): BasicButton(s, "New Game"), parent(p) {}
            void mouse_clicked(Point) {
                Engine.screen()->set_update(true);
                Engine.audio()->play_sound("menu2");
                parent->unregister_buttons();
                Engine.screen()->clear();
                /*
                Size s = Engine.screen()->get_size();
                Engine.screen()->add_child(new TextInputWidget({s.w * 0.75, s.h * 0.5}, "Enter the world's name:", parent), {s.w * 0.125, s.h * 0.25});
                */
                Engine.screen()->set_overlay(0xFF000000, 10, parent);
                Engine.input()->disable();
            }
            MainMenu* parent;
    };

    class LoadButton : public BasicButton {
        public:
            LoadButton(MainMenu* p, Size s): BasicButton(s, "Load Game"), parent(p) {}
            void mouse_clicked(Point) { 
                if (file_exists("state.sav")) {
                    Engine.load_state("state.sav");
                    Engine.audio()->play_sound("menu2");
                    parent->unregister_buttons();
                    Engine.screen()->clear();
                    Engine.screen()->add_child(new MapScreen(), {0, 0});
                }
            }
            MainMenu* parent;
    };
    
    class OptionsButton : public BasicButton {
        public:
            OptionsButton(Size s): BasicButton(s, "Options") {}
            void mouse_clicked(Point) { }
    };
    
    class ExitButton : public BasicButton {
        public:
            ExitButton(Size s): BasicButton(s, "Quit") {}
            void mouse_clicked(Point) { exit(0); }
    };
                
    void unregister_buttons() {
        for (Button* b : buttons) {
            Engine.input()->remove_mouse_listener(b);
        }
    }

    void init() {
        Size s(1.0 * size.w, 0.3 * size.h);
        buttons = {new NewButton(this, s), new LoadButton(this, s), new OptionsButton(s), new ExitButton(s)};
        for (int i = 0; i < 4; i++) {
            add_child(buttons[i], {0.0, 0.3 * i * size.h});
        }
    }
        
    std::vector<Button*> buttons = {nullptr};
};

#endif
