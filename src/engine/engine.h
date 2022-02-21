#ifndef ENGINE_H
#define ENGINE_H

class Input;
class Database;
class Tilemap;
class TextureManager;
class Input;
class Screen;
class AudioPlayer;
class Simulation;
class ScenePlayer;

#include "util.h"

class GameEngine {
    public:
        GameEngine();
        void init();
        void run();

        void save_state(const char* filename);
        void load_state(const char* filename);

        void register_script_function(const ScriptFunction& function);
        void execute_script(const std::string& filepath);

        Database* db() { return m_db; }
        Tilemap* map() { return m_map; }
        TextureManager* textures() { return m_textures; }
        Input* input() { return m_input; }
        Screen* screen() { return m_screen; }
        AudioPlayer* audio() { return m_audio; }
        Simulation* sim() { return m_sim; }
        ScenePlayer* scenes() { return m_scenes; }
        ScriptParam& config(const std::string& name) { return m_configs[name]; }

    private:
        Input* m_input = nullptr;
        Database* m_db = nullptr;
        Tilemap* m_map = nullptr;
        Screen* m_screen = nullptr;
        TextureManager* m_textures = nullptr;
        AudioPlayer* m_audio = nullptr;
        Simulation* m_sim = nullptr;
        ScenePlayer* m_scenes = nullptr;
        std::map<std::string, ScriptParam> m_configs;
};

extern GameEngine Engine;

#endif
