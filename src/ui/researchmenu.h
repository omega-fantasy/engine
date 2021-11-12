#ifndef RESEARCHMENU_H
#define RESEARCHMENU_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/audio.h"
#include "system/system.h"
#include "system/player.h"
#include "system/research.h"
#include "ui/commonui.h"

class ResearchMenu : public Composite {
    public:
    ResearchMenu(Size sz): Composite(sz) {}
    ~ResearchMenu() {
        for (auto child : children) {
            delete child;
        }
    }
    
    private:
    
    class ResearchWidget : public Composite {
        class Button : public ButtonWithTooltip {
            public:
                Button(ResearchWidget* p, const std::string& n, const std::string& d): ButtonWithTooltip({0, 0}, n, d), parent(p) {}
                void mouse_clicked(Point) {
                    Engine.audio()->play_sound("menu1");
                    int progress = System.research()->progress(txt);
                    parent->set_progress(progress);
                }
                ResearchWidget* parent;
        };
        
        public:
        ResearchWidget(Size sz, const std::string& name, const std::string& desc, int prog): Composite(sz) {
            button = new Button(this, name, desc);
            bar = new ProgressBar({1.0 * sz.w, 0.4 * sz.h}, Color(255, 255, 255), Color(0, 0, 0));
            pr = prog;
        }

        ~ResearchWidget() { delete button; delete bar; }

        void set_progress(int p) { 
            bar->set_progress(p); 
            if (p == 100) {
                button->set_enabled(false);
            }
        }
    
        void init() {
            button->set_texture(new Texture(Color(100, 100, 100), Size(1.0 * size.w, 0.4 * size.h)));
            add_child(button, {0, 0});
            add_child(bar, {0.0, 0.5 * size.h});
            set_progress(pr);
        }

        int pr;
        Button* button = nullptr;
        ProgressBar* bar = nullptr;
    };


    void init() {
        int i = 1;
        for (auto& item : System.research()->itemlist()) {
            auto button = new ResearchWidget({0.8 * size.w, 0.15 * size.h}, std::get<0>(item), std::get<1>(item), std::get<2>(item));
            add_child(button, {0.1 * size.w, 0.2 * i * size.h});
            i++;
        }
    }
};

#endif
