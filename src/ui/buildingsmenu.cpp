#include "buildingsmenu.h"
#include "ui/commonui.h"
#include "engine/audio.h"
#include "engine/tilemap.h"
#include "engine/engine.h"
#include "system/buildings.h"
#include "system/system.h"
        
class BuildingsMenu::BuildButton : public BasicButton {
    public:
        BuildButton(BuildingsMenu* p, const std::string& n, Size sz, int pr): BasicButton(sz, n), parent(p), name(n), price(pr) {}

        virtual void init() { 
            BasicButton::init();
            if (price >= 0) { 
                set_text(name + " (" + std::to_string(price) + ")"); 
            }
        }

        virtual void mouse_clicked(Point) { parent->set_mode(this); }
       
        BuildingsMenu* parent;
        std::string name;
        int price;
};

BuildingsMenu::BuildingsMenu(Size sz) : Composite(sz) {}
BuildingsMenu::~BuildingsMenu() {
    set_mode(nullptr);
    for (auto child : children) {
        delete child;
    }
    Engine.map()->remove_listener(this);
}
        
void BuildingsMenu::confirmed(TextInputWidget* widget) {
    System.buildings()->set_townname(created_town, widget->current_text()); 
    Engine.map()->remove_child(widget);
    delete widget;
    created_town = {-1, -1};
}

void BuildingsMenu::init() {
    Size s = {0.8 * size.w, 0.1 * size.h};
    Engine.map()->add_listener(this);
    text_cash = new Text(std::to_string(System.player()->current_cash()) + " Gil", 0.05 * size.h, {0.6 * size.w, 0.1 * size.h});
    add_child(text_cash, {0.3 * size.w, 0.0});
    add_child(new BuildButton(this, "new town", s, -1), Point(0.1 * size.w, children.size() * 0.1 * size.h));
    add_child(new BuildButton(this, "destroy", s, -1), Point(0.1 * size.w, children.size() * 0.1 * size.h));
    int i = children.size();
    for (auto& button : System.buildings()->types()) {
        add_child(new BuildButton(this, button.name, s, button.price), Point(0.1 * size.w, i * 0.1 * size.h));
        i++;
    }
}

void BuildingsMenu::tile_clicked(Point p) {
    if (active_button) {
        std::string sound;
        if (active_button->name == "destroy") {
            System.buildings()->destroy(p);
            sound = "destroy";
        } else if (active_button->name == "new town") {
            bool success = System.buildings()->create_town(p) && Engine.map()->set_tile("town", p);
            sound = success ? "build" : "error";
            if (success) {
                Size s = {Engine.map()->get_size().w * 0.75, Engine.map()->get_size().h * 0.25};
                TextInputWidget* name_input = new TextInputWidget(s, "Enter the town's name:", this);
                Engine.map()->add_child(name_input, {Engine.map()->get_size().w * 0.125, Engine.map()->get_size().h * 0.375});
                created_town = p;
            }
        } else {
            sound = System.buildings()->create(active_button->name, p) ? "build" : "error";
        }
        if (sound == "build") {
            set_mode(nullptr);
        } else {
            Engine.audio()->play_sound(sound);
        }
    }
}

void BuildingsMenu::draw() {
    static int update_cash = 0;
    if (update_cash++ > 50) {
        update_cash = 0;
        text_cash->set_text(std::to_string(System.player()->current_cash()) + " Gil", 0.05 * size.h);
    }
    Composite::draw();
}

void BuildingsMenu::set_mode(BuildButton* button) {
    if (active_button) {
        active_button->set_selected(false);
    }
    Engine.audio()->play_sound("menu1");
    if (button && active_button != button) {
        active_button = button;
        active_button->set_selected(true);
        if (active_button->name == "new town") {
            Engine.map()->set_cursor_texture("town");
        } else {
            Engine.map()->set_cursor_texture(active_button->name);
        }
    } else {
        active_button = nullptr;
        Engine.map()->set_cursor_texture("");
    }
} 
