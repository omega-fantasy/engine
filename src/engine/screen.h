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
         virtual void set_update(bool update) {
             update_count = update ? MAX_NO_UPDATES : 0;
             for (auto& child : children) {
                 child->set_update(true);
             }
         }
         void set_overlay(Color color, int num_frames = 1, Listener* listener = nullptr);
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
         int MAX_NO_UPDATES = 5;
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

        inline void blit(Color* __restrict texture, Size texture_size, Point start, Box canvas, bool transparent, short texture_stride=0) {
            if (!texture_stride) texture_stride = texture_size.w;
            Point texture_end(start.x + texture_size.w, start.y + texture_size.h);
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
            
            unsigned* __restrict texture_pixels = (unsigned*)(texture + texture_start.y * texture_stride + texture_start.x); 
            unsigned* __restrict screen_pixels = (unsigned*)(pixels + start.y * size.w + start.x);
            short upper_bound_x = texture_size.w - texture_start.x - texture_endcut.x;
            short upper_bound_y = texture_size.h - texture_start.y - texture_endcut.y;
            if (upper_bound_y > 0 && upper_bound_x > 0) {
                if (transparent) {
                    for (short y = 0; y < upper_bound_y; y++) {
                        for (short x = 0; x < upper_bound_x; x++) {
                            unsigned color1 = screen_pixels[y * size.w + x];
                            unsigned color2 = texture_pixels[y * texture_stride + x];
                            unsigned rb = (color1 & 0xff00ff) + (((color2 & 0xff00ff) - (color1 & 0xff00ff)) * ((color2 & 0xff000000) >> 24) >> 8);
                            unsigned g  = (color1 & 0x00ff00) + (((color2 & 0x00ff00) - (color1 & 0x00ff00)) * ((color2 & 0xff000000) >> 24) >> 8);
                            screen_pixels[y * size.w + x] = (rb & 0xff00ff) | (g & 0x00ff00);
                        }
                    }
                } else {
                    for (short y = 0; y < upper_bound_y; y++) {
                        std::memcpy(screen_pixels + y * size.w, texture_pixels + y * texture_stride, upper_bound_x * sizeof(unsigned));
                    }
                }
            }
        }

        void update() {
            long long t_now = now();
            long long t = t_now - last_update;
            update_window();
            m_fps = t > 0 ? (double) 1 / ((double)now() / (1000 * 1000)) : 0; 
            last_update = now();
        }

        int fps() { return m_fps; }

        int* pixels;

    private:
        long long last_update = now();
        int m_fps = 0;
};

#endif
