#include "buildingsmenu.h"
#include "engine/config.h"
#include "engine/db.h"
#include "engine/audio.h"

BuildingsMenu::BuildButton::BuildButton(BuildingsMenu* p, const std::string& n, Size sz, int pr, bool ground): Button(sz, n), parent(p), name(n), price(pr), is_ground(ground) {}

void BuildingsMenu::BuildButton::init() { if (price >= 0) { set_text(name + " (" + std::to_string(price) + ")"); }}

void BuildingsMenu::BuildButton::mouse_clicked(Point) { parent->set_mode(this); }

BuildingsMenu::BuildingsMenu(Size sz) : Composite(sz) {}

void BuildingsMenu::init() {
    Engine.map()->add_listener(this);
    text_cash = new Text(std::to_string(Engine.db()->get_table<int>("cash")->get(0)) + " Gil", 0.05 * size.h);
    add_child(text_cash, {0.3 * size.w, 0.0});

    int i = 1;
    for (auto& button : Engine.config()->get("buildings")["buildings"]) {
        Button* b = new BuildButton(this, button["name"], {0, 0}, std::stoi(button["price"]), false);
        b->set_texture(texture_button_default);
        add_child(b, Point(0.1 * size.w, i * 0.08 * size.h));
        i++;
    }
    i = children.size();
    for (auto& button : Engine.config()->get("buildings")["grounds"]) {
        Button* b = new BuildButton(this, button["name"], {0, 0}, std::stoi(button["price"]), true);
        b->set_texture(texture_button_default);
        add_child(b, Point(0.1 * size.w, i * 0.08 * size.h));
        i++;
    }
    Button* destroy_button = new BuildButton(this, "destroy", {0, 0}, -1, true);
    add_child(destroy_button, Point(0.1 * size.w, i * 0.08 * size.h));
    destroy_button->set_texture(texture_button_default);
}

bool BuildingsMenu::change_cash(int amount) {
    auto& current = Engine.db()->get_table<int>("cash")->get(0);
    if (current + amount < 0) {
        return false;
    }
    current += amount;
    text_cash->set_text(std::to_string(Engine.db()->get_table<int>("cash")->get(0)) + " Gil", 0.05 * size.h);
    return true;
}

void BuildingsMenu::tile_clicked(Point p) {
    if (active_button) {
        if (active_button->name == "destroy") {
            Engine.map()->unset_tile(p);
            Engine.audio()->play_sound("destroy");
        } else {
            if (change_cash(-active_button->price)) {
                if (active_button->is_ground ? Engine.map()->set_ground(active_button->name, p, true) : Engine.map()->set_tile(active_button->name, p)) {
                    Engine.audio()->play_sound("build");
                    return;
                } else {
                    change_cash(active_button->price); // refund
                }
            }
            Engine.audio()->play_sound("error");
        }
    }
}

void BuildingsMenu::draw() {
    static int update_cash = 0;
    if (update_cash++ > 50) {
        update_cash = 0;
        change_cash(0);
    }
    Composite::draw();
}

void BuildingsMenu::set_mode(BuildButton* button) {
    if (active_button) {
        active_button->set_texture(texture_button_default);
    }
    Engine.audio()->play_sound("menu1");
    if (active_button != button) {
        active_button = button;
        active_button->set_texture(texture_button_active);
        Engine.map()->set_cursor_texture(active_button->name);
    } else {
        active_button = nullptr;
        Engine.map()->set_cursor_texture("");
    }
} 
