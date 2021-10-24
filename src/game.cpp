#define SDL_MAIN_HANDLED
#include "engine/engine.h"
#include "engine/screen.h"
#include "engine/audio.h"
#include "ui/mainmenu.h"

int main() {
    Engine.config()->add_folder("./config");
    Size resolution(std::stoi(Engine.config()->get("settings")["resolution"]["width"]), std::stoi(Engine.config()->get("settings")["resolution"]["height"]));

    Engine.init(resolution);
    Engine.textures()->add_folder("./res/textures");
    Engine.audio()->add_sound_folder("./res/sounds");
    //Engine.audio()->add_music_folder("./res/music");
    //Engine.audio()->play_music("music", 20);
    System.init();
    MainMenu* mm = new MainMenu({0.5 * resolution.w, 0.5 * resolution.h});
    Engine.screen()->add_child(mm, {0.25 * resolution.w, 0.25 * resolution.h});
    Engine.run();
    return 0;
}
