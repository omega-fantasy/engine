#ifndef BUILDUINGSMENU_H
#define BUILDUINGSMENU_H

#include "engine/engine.h"
#include "engine/screen.h"
#include "engine/tilemap.h"
#include "engine/ui.h"
#include "ui/textinputwidget.h"

class BuildingsMenu : public Composite, Tilemap::Listener, TextInputWidget::Listener {
    public:
        class BuildButton;
        BuildingsMenu(const BuildingsMenu&) = delete;
        BuildingsMenu& operator= (const BuildingsMenu&) = delete;
        BuildingsMenu(Size sz);
        ~BuildingsMenu();
        void set_mode(BuildButton* button); 
        void confirmed(TextInputWidget*);

    private:
        virtual void tile_clicked(Point p); 
        void draw();
        void init();
        BuildButton* active_button = nullptr;
        Text* text_cash = nullptr;
        Point created_town = {-1, -1};
};

#endif
