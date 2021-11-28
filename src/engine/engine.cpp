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
#include "scene.h"

GameEngine Engine;

GameEngine::GameEngine() {
    m_config = new ConfigParser();
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
        Color new_color;
        for (int j = 0; j < 4; j++) {
            new_color[j] = old_color[j];
            new_color[j] += ((double)i / num_frames) * (target_color[j] - old_color[j]);
        }
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

void Screen::blit(Color* texture, Size texture_size, Point start, Box canvas, bool) {
    Point texture_end(start.x + texture_size.w, start.y + texture_size.h);
    if (texture_end.x < canvas.a.x || texture_end.y < canvas.a.y || 
        start.x > canvas.b.x || start.y > canvas.b.y) {
        return; 
    }

    Point texture_start(0, 0);
    Point texture_endcut(0, 0);
    if (start.x < canvas.a.x) {
        texture_start.x = (canvas.a.x - start.x);
        start.x = canvas.a.x;
    } else if (texture_end.x > canvas.b.x) {
        texture_endcut.x = texture_end.x - canvas.b.x;
    }
    if (start.y < canvas.a.y) {
        texture_start.y = (canvas.a.y - start.y);
        start.y = canvas.a.y;
    } else if (texture_end.y > canvas.b.y) {
        texture_endcut.y = texture_end.y - canvas.b.y;
    }
    
    unsigned* texture_pixels = (unsigned*)(texture + texture_start.y * texture_size.w + texture_start.x); 
    unsigned* screen_pixels = (unsigned*)(pixels + start.y * size.w + start.x);
    for (int y = 0; y < texture_size.h - texture_start.y - texture_endcut.y; y++) {
        for (int x = 0; x < texture_size.w - texture_start.x - texture_endcut.x; x++) {
            unsigned color1 = screen_pixels[y * size.w + x];
            unsigned color2 = texture_pixels[y*texture_size.w+x];
            unsigned rb = (color1 & 0xff00ff) + (((color2 & 0xff00ff) - (color1 & 0xff00ff)) * ((color2 & 0xff000000) >> 24) >> 8);
            unsigned g  = (color1 & 0x00ff00) + (((color2 & 0x00ff00) - (color1 & 0x00ff00)) * ((color2 & 0xff000000) >> 24) >> 8);
            screen_pixels[y * size.w + x] = (rb & 0xff00ff) | (g & 0x00ff00);
        }
    }
}