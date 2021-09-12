#include "texture.h"
#include "screen.h"
#include "tilemap.h"
#include "db.h"
#include "input.h"
#include "ui.h"
#include "engine.h"
#include "audio.h"
#include "sim.h"
#include "config.h"

GameEngine Engine;

GameEngine::GameEngine() {
    m_config = new ConfigParser();
}

void GameEngine::init(Size screen_size) {
    TTF_Init();
    m_db = new Database("database");
    m_input = new Input();
    m_audio = new AudioPlayer();
    m_screen = new Screen(screen_size);
    m_textures = new TextureManager();
    m_map = new Tilemap({0, 0});
    m_sim = new Simulation();
}
        
void GameEngine::save_state(const char* filename) {
    m_db->write(filename);
}
 
void GameEngine::load_state(const char* filename) {
    m_db->read(filename);
    m_map->create_map(m_map->get_size());
}

void GameEngine::run() {
    while(1) {
        m_input->handleInputs();
        m_screen->draw();
        m_screen->update();
    }
}
