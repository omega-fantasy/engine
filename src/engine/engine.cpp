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
#include "scripting.h"
#include "scene.h"

GameEngine Engine;

GameEngine::GameEngine() {
    m_config = new ConfigParser();
    m_script = new Script();
}

void GameEngine::init(Size screen_size) {
    m_db = new Database("database");
    m_input = new Input();
    m_screen = new Screen(screen_size);
    m_audio = new AudioPlayer();
    m_textures = new TextureManager();
    m_map = new Tilemap({0, 0});
    m_sim = new Simulation();
    m_scenes = new ScenePlayer();
}
        
void GameEngine::save_state(const char* filename) {
    m_db->write(filename);
}
 
void GameEngine::load_state(const char* filename) {
    m_db->read(filename);
    m_textures->reinit();
    m_map->create_map(m_map->get_size());
}

void GameEngine::run() {
    while(1) {
        m_scenes->handle_scenes();
        m_input->handleInputs();
        m_screen->draw();
        m_screen->update();
    }
}

void Composite::set_overlay(Color color, int num_frames, Listener* listener) {
    overlay_listener = listener;
    if (!m_overlay) {
        m_overlay = new Texture((unsigned)0x0, size);
        m_overlay->set_transparent(true);
    }
    Color target_color;
    target_color = color;
    Color old_color;
    old_color = static_cast<unsigned>(m_overlay->pixels()[0]); 
    for (int i = num_frames; i > 0; i--) {
        Color new_color = old_color;
        new_color.red += ((double)i / num_frames) * (target_color.red - old_color.red);
        new_color.green += ((double)i / num_frames) * (target_color.green - old_color.green);
        new_color.blue += ((double)i / num_frames) * (target_color.blue - old_color.blue);
        new_color.alpha += ((double)i / num_frames) * (target_color.alpha - old_color.alpha);
        overlay_colors.push_back(new_color);
    }
}

void Composite::draw() {
    if (!initialized) {
        init();
        initialized = true;
    }
    if (needs_update() && m_texture) {
        Engine.screen()->blit(m_texture->pixels(), m_texture->size(), pos, Box(pos, size), m_texture->transparent());
        set_update(false);
    }
    for (auto& child : children) {
        child->draw();
    }
    if (!overlay_colors.empty()) {
        Color new_color;
        new_color = overlay_colors.back();
        new_color.alpha = overlay_colors.back().alpha; // to make static analysis shut up
        Color* pixels = m_overlay->pixels();
        std::fill(pixels, pixels + m_overlay->size().w * m_overlay->size().h, int(new_color));
        overlay_colors.pop_back();
        if (overlay_colors.empty()) {
            if (new_color.alpha == 0) {
                delete m_overlay;
                m_overlay = nullptr;
            }
            if (overlay_listener) {
                overlay_listener->fade_completed(this);
            }
        }
    }
    if (m_overlay) {
        Engine.screen()->blit(m_overlay->pixels(), m_overlay->size(), pos, Box(pos, size), true);
        set_update(true);
    }
}