#define SDL_MAIN_HANDLED
#include "engine/engine.h"
#include "engine/screen.h"
#include "engine/tilemap.h"
#include "engine/input.h"
#include "engine/audio.h"
#include "engine/sim.h"
#include "ui/hud.h"
#include "system/system.h"

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
        Engine.input()->add_key_listeners(this, {key_up, key_down, key_left, key_right}, true);
        Engine.input()->add_key_listeners(this, {key_zoomin, key_zoomout, key_quit}, false);
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

int main() {
    Engine.config()->add_folder("./config");
    
    Size resolution(std::stoi(Engine.config()->get("settings")["resolution"]["width"]), std::stoi(Engine.config()->get("settings")["resolution"]["height"]));

    Engine.init(resolution);
    Engine.textures()->add_folder("./res/textures");
    Engine.audio()->add_sound_folder("./res/sounds");
    //Engine.audio()->add_music_folder("./res/music");
    //Engine.audio()->play_music("music", 20);
    System.init();

    double hud_width_per = 0.20;
    HUD* hud = new HUD({(double)(resolution.w * hud_width_per), (double)resolution.h});
    Engine.map()->create_map({(double)(resolution.w * (1-hud_width_per)), (double)resolution.h});
    Engine.map()->randomize_map();

    new MapNavigate();
    Engine.screen()->add_child(Engine.map(), {(int)hud->get_size().w, 0});
    Engine.screen()->add_child(hud, {0, 0});

    Engine.run();
    return 0;
}
