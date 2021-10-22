#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "system/system.h"

class GameWidget : public Composite {
    public:
    GameWidget(Size sz): Composite(sz) {}
    
    private:
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
    void init() {
        Button* buttons[3] = {new NewButton(), new SaveButton(), new LoadButton()};
        Texture* texture_button = new Texture(0xFF555555, {0.8 * size.w, 0.3 * size.h});
        for (int i = 0; i < 3; i++) {
            add_child(buttons[i], {0.1 * size.w, 0.33 * i * size.h});
            buttons[i]->set_texture(texture_button);
        }

    }
};

#endif
