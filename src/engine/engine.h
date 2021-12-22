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
class ConfigParser;
class ScriptParser;
class ScenePlayer;

#include "util.h"

class GameEngine {
    public:
        GameEngine();
        void init(Size screen_size);
        void run();

        void save_state(const char* filename);
        void load_state(const char* filename);

        Database* db() { return m_db; }
        Tilemap* map() { return m_map; }
        TextureManager* textures() { return m_textures; }
        Input* input() { return m_input; }
        Screen* screen() { return m_screen; }
        AudioPlayer* audio() { return m_audio; }
        Simulation* sim() { return m_sim; }
        ConfigParser* config() { return m_config; }
        ScriptParser* script() { return m_script; }
        ScenePlayer* scenes() { return m_scenes; }

    private:
        Input* m_input = nullptr;
        Database* m_db = nullptr;
        Tilemap* m_map = nullptr;
        Screen* m_screen = nullptr;
        TextureManager* m_textures = nullptr;
        AudioPlayer* m_audio = nullptr;
        Simulation* m_sim = nullptr;
        ConfigParser* m_config = nullptr;
        ScriptParser* m_script = nullptr;
        ScenePlayer* m_scenes = nullptr;
};

extern GameEngine Engine;

#endif
