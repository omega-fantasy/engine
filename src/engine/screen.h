#ifndef SCREEN_H
#define SCREEN_H

#include "engine.h"
#include "texture.h"
#include "config.h"
#include <cstring>
#include <vector>
#include <chrono>
#include <algorithm>
#include <SDL2/SDL.h>

class Composite {
    public:
         Composite(Size sz): size(sz) {}
         virtual ~Composite() {}

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
             if (update) {
                 update_count = MAX_NO_UPDATES;
             } else {
                 update_count = 0;
             }
             for (auto& child : children) {
                 child->set_update(true);
             }
         }

    protected:
         bool initialized = false;
         Point pos = {0, 0};
         Size size;
         Texture* m_texture = nullptr;
         std::vector<Composite*> children;
         int update_count = 0;
         bool needs_update() {
             if (update_count < MAX_NO_UPDATES) {
                update_count++;
                return false;
             }
             set_update(true);
             update_count = 0;
             return true;
         }
};

class Screen : public Composite {
    public:
        Screen(Size sz): Composite(sz) {
            SDL_Init(SDL_INIT_VIDEO);
            window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.w, size.h, 0);
            if (std::stoi(Engine.config()->get("settings")["resolution"]["fullscreen"])) {
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            }
            pixels = static_cast<int*>(SDL_GetWindowSurface(window)->pixels);
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
            
            int*  texture_pixels = texture->pixels(zoom) + texture_start.y * texture_size.w + texture_start.x; 
            int*  screen_pixels = pixels + start.y * size.w + start.x;
            int linesize = (texture_size.w - texture_start.x - texture_endcut.x) * sizeof(int);

            if (texture->transparent()) {
                for (int y = 0; y < texture_size.h - texture_start.y - texture_endcut.y; y++) {
                    for (int x = 0; x < texture_size.w - texture_start.x - texture_endcut.x; x++) {
                        unsigned char* pixel = (unsigned char*)&texture_pixels[y * texture_size.w + x];
                        float a_pixel = alpha_lookup[pixel[3]];
                        if (pixel[3] == 255) {
                            screen_pixels[y * size.w + x] = texture_pixels[y * texture_size.w + x];
                        } else if (a_pixel > 0) {
                            float b_pixel = alpha_lookup[255-pixel[3]];
                            unsigned char* screen =  (unsigned char*)&screen_pixels[y * size.w + x];
                            screen[0] = pixel[0] + screen[0] * b_pixel; // B
                            screen[1] = pixel[1] + screen[1] * b_pixel; // G
                            screen[2] = pixel[2] + screen[2] * b_pixel; // R
                            screen[3] = 255 * (a_pixel + alpha_lookup[screen[3]] * b_pixel); // A
                        }
                    }
                }
            } else {
                for (int y = 0; y < texture_size.h - texture_start.y - texture_endcut.y; y++) {
                    std::memcpy(screen_pixels + y * size.w, texture_pixels + y * texture_size.w, linesize);
                }
            }
        }

        void update() {
            long long t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - lastUpdate).count();
            long long delta = 16660 - t;
            if (delta > 0) { SDL_Delay(delta / 1000); }
            SDL_UpdateWindowSurface(window);
            //auto diff = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - lastUpdate).count();
            //std::cout << "FPS: " << (double)1 / diff << std::endl;
            lastUpdate = std::chrono::high_resolution_clock::now();
        }

    private:
        SDL_Window* window;
        int* pixels;
        std::chrono::high_resolution_clock::time_point lastUpdate = std::chrono::high_resolution_clock::now();
        static constexpr float alpha_lookup[256] = {
            0.0000, 0.0039, 0.0078, 0.0118, 0.0157, 0.0196, 0.0235, 0.0275, 0.0314,
            0.0353, 0.0392, 0.0431, 0.0471, 0.0510, 0.0549, 0.0588, 0.0627, 0.0667,
            0.0706, 0.0745, 0.0784, 0.0824, 0.0863, 0.0902, 0.0941, 0.0980, 0.1020,
            0.1059, 0.1098, 0.1137, 0.1176, 0.1216, 0.1255, 0.1294, 0.1333, 0.1373,
            0.1412, 0.1451, 0.1490, 0.1529, 0.1569, 0.1608, 0.1647, 0.1686, 0.1725,
            0.1765, 0.1804, 0.1843, 0.1882, 0.1922, 0.1961, 0.2000, 0.2039, 0.2078,
            0.2118, 0.2157, 0.2196, 0.2235, 0.2275, 0.2314, 0.2353, 0.2392, 0.2431,
            0.2471, 0.2510, 0.2549, 0.2588, 0.2627, 0.2667, 0.2706, 0.2745, 0.2784,
            0.2824, 0.2863, 0.2902, 0.2941, 0.2980, 0.3020, 0.3059, 0.3098, 0.3137,
            0.3176, 0.3216, 0.3255, 0.3294, 0.3333, 0.3373, 0.3412, 0.3451, 0.3490,
            0.3529, 0.3569, 0.3608, 0.3647, 0.3686, 0.3725, 0.3765, 0.3804, 0.3843,
            0.3882, 0.3922, 0.3961, 0.4000, 0.4039, 0.4078, 0.4118, 0.4157, 0.4196,
            0.4235, 0.4275, 0.4314, 0.4353, 0.4392, 0.4431, 0.4471, 0.4510, 0.4549,
            0.4588, 0.4627, 0.4667, 0.4706, 0.4745, 0.4784, 0.4824, 0.4863, 0.4902,
            0.4941, 0.4980, 0.5020, 0.5059, 0.5098, 0.5137, 0.5176, 0.5216, 0.5255,
            0.5294, 0.5333, 0.5373, 0.5412, 0.5451, 0.5490, 0.5529, 0.5569, 0.5608,
            0.5647, 0.5686, 0.5725, 0.5765, 0.5804, 0.5843, 0.5882, 0.5922, 0.5961,
            0.6000, 0.6039, 0.6078, 0.6118, 0.6157, 0.6196, 0.6235, 0.6275, 0.6314,
            0.6353, 0.6392, 0.6431, 0.6471, 0.6510, 0.6549, 0.6588, 0.6627, 0.6667,
            0.6706, 0.6745, 0.6784, 0.6824, 0.6863, 0.6902, 0.6941, 0.6980, 0.7020,
            0.7059, 0.7098, 0.7137, 0.7176, 0.7216, 0.7255, 0.7294, 0.7333, 0.7373,
            0.7412, 0.7451, 0.7490, 0.7529, 0.7569, 0.7608, 0.7647, 0.7686, 0.7725,
            0.7765, 0.7804, 0.7843, 0.7882, 0.7922, 0.7961, 0.8000, 0.8039, 0.8078,
            0.8118, 0.8157, 0.8196, 0.8235, 0.8275, 0.8314, 0.8353, 0.8392, 0.8431,
            0.8471, 0.8510, 0.8549, 0.8588, 0.8627, 0.8667, 0.8706, 0.8745, 0.8784,
            0.8824, 0.8863, 0.8902, 0.8941, 0.8980, 0.9020, 0.9059, 0.9098, 0.9137,
            0.9176, 0.9216, 0.9255, 0.9294, 0.9333, 0.9373, 0.9412, 0.9451, 0.9490,
            0.9529, 0.9569, 0.9608, 0.9647, 0.9686, 0.9725, 0.9765, 0.9804, 0.9843,
            0.9882, 0.9922, 0.9961, 1.0000
        };
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
}

#endif
