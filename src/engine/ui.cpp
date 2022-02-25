#include "ui.h"
#include "screen.h"
#include "tilemap.h"
#include "ui/commonui.h"
#include "ui/hud.h"

class ScriptingButton : public BasicButton {
    public:
        ScriptingButton(Size s, const std::string& n, ScriptCallback* cb): BasicButton(s, n), callback(cb) {}
        void mouse_clicked(Point) { callback->run(); }
        ScriptCallback* callback = nullptr;
};

void Screen::init_script_api() {
    Engine.register_script_function({"MAP_randomize", {}, [&](const std::vector<ScriptParam>&) {
        Engine.map()->randomize_map(); return 0;
    }});
    Engine.register_script_function({"UI_new_tilemap", {ScriptType::NUMBER, ScriptType::NUMBER}, [&](const std::vector<ScriptParam>& params) {
        Engine.map()->create_map({params[0].d(), params[1].d()});
        return Engine.map();
    }});
    Engine.register_script_function({"UI_new_hud", {ScriptType::NUMBER, ScriptType::NUMBER}, [&](const std::vector<ScriptParam>& params) {
        return new HUD({params[0].d(), params[1].d()});
    }});

    Engine.register_script_function({"UI_screen_container", {}, [&](const std::vector<ScriptParam>&) {
        return Engine.screen();
    }});
    Engine.register_script_function({"UI_screen_size", {}, [&](const std::vector<ScriptParam>&) {
        std::map<ScriptParam, ScriptParam> ret;
        Size s = Engine.screen()->get_size();
        ret["width"] = (int)s.w; ret["height"] = (int)s.h;
        return ret;
    }});
    Engine.register_script_function({"UI_screen_clear", {}, [&](const std::vector<ScriptParam>&) {
        Engine.screen()->clear(); return 0;
    }});
    Engine.register_script_function({"UI_new_container", {ScriptType::NUMBER, ScriptType::NUMBER}, [&](const std::vector<ScriptParam>& params) {
        return new Composite({params[0].d(), params[1].d()});
    }});
    Engine.register_script_function({"UI_container_size", {ScriptType::HANDLE}, [&](const std::vector<ScriptParam>& params) {
        std::map<ScriptParam, ScriptParam> ret;
        Size s = params[0].p<Composite>()->get_size();
        ret["width"] = (int)s.w; ret["height"] = (int)s.h;
        return ret;
    }});
    Engine.register_script_function({"UI_add_child", {ScriptType::HANDLE, ScriptType::HANDLE, ScriptType::NUMBER, ScriptType::NUMBER}, [&](const std::vector<ScriptParam>& params) {
        params[0].p<Composite>()->add_child(params[1].p<Composite>(), {params[2].d(), params[3].d()}); return 0;
    }});
    Engine.register_script_function({"UI_new_button", {ScriptType::NUMBER, ScriptType::NUMBER, ScriptType::STRING, ScriptType::CALLBACK}, [&](const std::vector<ScriptParam>& params) {
        return new ScriptingButton({params[0].d(), params[1].d()}, params[2].s(), params[3].cb());
    }});
    Engine.register_script_function({"UI_container_dispose", {ScriptType::HANDLE}, [&](const std::vector<ScriptParam>& params) {
        delete params[0].p<Composite>(); return 0;
    }});
    Engine.register_script_function({"UI_play_sound", {ScriptType::STRING}, [&](const std::vector<ScriptParam>& params) {
        Engine.audio()->play_sound(params[0].s()); return 0;
    }});
}