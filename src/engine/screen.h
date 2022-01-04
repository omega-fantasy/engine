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
         int MAX_NO_UPDATES = 1;
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

        void blit(Color* texture, Size texture_size, Point start, Box canvas, bool transparent);

        void update() {
            long long t = now() - last_update;
            long long delta = 16660 - t;
            if (delta > 0) { wait(delta); }
            update_window();
            m_fps = t > 0 ? (double) 1 / ((double)t / (1000 * 1000)) : 0; 
            last_update = now();
        }

        int fps() { return m_fps; }

    private:
        int* pixels;
        long long last_update = now();
        int m_fps = 0;
};

#endif
