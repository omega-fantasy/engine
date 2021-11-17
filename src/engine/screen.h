#ifndef SCREEN_H
#define SCREEN_H

#include "engine.h"
#include "texture.h"
#include "config.h"

class Composite {
    public:
         class Listener {
              public:
              virtual void fade_completed(Composite*) = 0; 
         };

         Composite(Size sz): size(sz) {}
         virtual ~Composite() { delete m_overlay; }

         virtual std::vector<Composite*> get_children() { return children; }
         virtual void set_size(Size s) { size = s; }

         virtual void add_child(Composite* child, Point offset) {
             child->pos = pos + offset;
             children.push_back(child);
         }
         
         virtual void remove_child(Composite* child) {
            children.erase(std::remove(children.begin(), children.end(), child), children.end()); 
         }
    
         virtual void draw();
         virtual void init() {}

         Size get_size() { return size; }
         int MAX_NO_UPDATES = 10;
         
         virtual void set_update(bool update) {
             update_count = update ? MAX_NO_UPDATES : 0;
             for (auto& child : children) {
                 child->set_update(true);
             }
         }

         void set_overlay(Color color, int num_frames = 1, Listener* listener = nullptr) {
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
         
         bool needs_update() {
             if (update_count < MAX_NO_UPDATES) {
                update_count++;
                return false;
             }
             set_update(true);
             update_count = 0;
             return true;
         }

    protected:
         std::vector<Color> overlay_colors;
         Listener* overlay_listener = nullptr;
         Texture* m_overlay = nullptr;
         bool initialized = false;
         Point pos = {0, 0};
         Size size;
         Texture* m_texture = nullptr;
         std::vector<Composite*> children;
         int update_count = 0;
};

class Screen : public Composite {
    public:
        Screen(Size sz): Composite(sz) {
            bool fullscreen = std::stoi(Engine.config()->get("settings")["resolution"]["fullscreen"]);
            pixels = (int*)create_window(sz, fullscreen);
        }

        void clear() {
            children.clear();
            std::memset(pixels, 0, sizeof(int) * size.w * size.h); 
        }

        void blit(Texture* texture, Point start, Box canvas, float zoom = 1) {
            Size texture_size = texture->size(zoom);
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
            
            unsigned* texture_pixels = (unsigned*)(texture->pixels(zoom) + texture_start.y * texture_size.w + texture_start.x); 
            unsigned* screen_pixels = (unsigned*)(pixels + start.y * size.w + start.x);
            int linesize = (texture_size.w - texture_start.x - texture_endcut.x) * sizeof(int);

            if (texture->transparent()) {
                for (int y = 0; y < texture_size.h - texture_start.y - texture_endcut.y; y++) {
                    for (int x = 0; x < texture_size.w - texture_start.x - texture_endcut.x; x++) {
                        // lol
                        screen_pixels[y * size.w + x] = 
                        ((screen_pixels[y*size.w+x] & 0xff00ff) + (((texture_pixels[y*texture_size.w+x] & 0xff00ff) - (screen_pixels[y*size.w+x] & 0xff00ff)) * ((unsigned char*)&texture_pixels[y * texture_size.w + x])[3] >> 8) & 0xff00ff) |
                        ((screen_pixels[y*size.w+x] & 0x00ff00) + (((texture_pixels[y*texture_size.w+x] & 0x00ff00) - (screen_pixels[y*size.w+x] & 0x00ff00)) * ((unsigned char*)&texture_pixels[y * texture_size.w + x])[3] >> 8) & 0xff00);
                    }
                }
            } else {
                for (int y = 0; y < texture_size.h - texture_start.y - texture_endcut.y; y++) {
                    std::memcpy(screen_pixels + y * size.w, texture_pixels + y * texture_size.w, linesize);
                }
            }
        }

        void update() {
            long long t = now() - last_update;
            long long delta = 16660 - t;
            if (delta > 0) { wait(delta); }
            update_window();
            last_update = now();
        }

    private:
        int* pixels;
        long long last_update = now();
};

inline void Composite::draw() {
    if (!initialized) {
        init();
        initialized = true;
    }
    if (needs_update() && m_texture) {
        Engine.screen()->blit(m_texture, pos, Box(pos, size), 1);
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
        Engine.screen()->blit(m_overlay, pos, Box(pos, size), 1);
        set_update(true);
    }
}

#endif
