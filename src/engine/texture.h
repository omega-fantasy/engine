#ifndef TEXTURE_H
#define TEXTURE_H

#include "geometry.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <filesystem>
#include <iostream>

class Texture {
    public:
        using ID = short;
        Texture(ID id, const std::string& path, const std::string name): m_id(id), m_name(name) {
            SDL_Surface* img = SDL_LoadBMP(path.c_str());
            load((int*)img->pixels, {img->w, img->h});
            SDL_FreeSurface(img);
        }
        Texture(SDL_Surface* img): m_id(0), m_name("") {
            load((int*)img->pixels, {img->w, img->h});
        }
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        
        Texture(Color color, Size s) {
            m_id = 0;
            pixels_og = new int[s.w * s.h];
            for (int i = 0; i < s.w * s.h; i++) {
                pixels_og[i] = color;         
            }
            width = s.w;
            height = s.h;
        }

        int* pixels(float zoom = 1) {
            if (zoom < 1) {         
                int idx = 0;
                if (zoom == 0.125) idx = 3;
                else if (zoom == 0.25) idx = 2;
                else if (zoom == 0.5) idx = 1;
                if ((int)pixels_zoomout.size() - 1 >= idx) {
                    auto ret = pixels_zoomout[idx];
                    if (ret) return ret;
                }
            } else if (std::abs(zoom - 1.0) < 0.01) {
                return pixels_og;
            } else if (zoom > 1 && (int)pixels_zoomin.size()-1 >= zoom) {
                auto ret = pixels_zoomin[static_cast<int>(zoom)];
                if (ret) return ret;
            }
            return load_scaled(zoom);
        }

        Size size(float zoom = 1) {
            return Size(width * zoom, height * zoom);
        }

        std::string name() { return m_name; }

        bool transparent() {
            return hasTransparency;
        }

        void set_transparent(bool b) {
            hasTransparency = b;
        }   

        ID id() { return m_id; }
        void set_id(ID i) { m_id = i; }

    protected:
        ID m_id;
        std::string m_name;
        std::vector<int*> pixels_zoomin;
        std::vector<int*> pixels_zoomout;
        int* pixels_og = nullptr;
        bool hasTransparency = false;
        int width = 0;
        int height = 0;
        
        int fastlog2(float v) {
            if (v == 0.125) return -3;
            if (v == 0.25) return -2;
            if (v == 0.5) return -1;
            if (v == 0.0625) return -4;
            return 0; 
        }

        void load(int* src, Size s) {
            width = s.w;
            height = s.h; 
            pixels_og = new int[width * height];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int idx =  y * width + x;
                    pixels_og[idx] = src[idx];
                    bool transparent = *((unsigned char*)(pixels_og + idx) + 3) < 128;
                    if (transparent) {
                        hasTransparency = true;
                    }
                }
            }
            if (hasTransparency) {
                for (int i = 0; i < height * width; i++) {
                    unsigned char* pixel =  (unsigned char*)&pixels_og[i];
                    float a_pixel = (float)pixel[3] / 255;
                    pixel[0] *= a_pixel;
                    pixel[1] *= a_pixel;
                    pixel[2] *= a_pixel;
                }
            }
        }

        int* load_scaled(float zoom) {
            int fact = (double)1 / zoom;
            int izoom = (int)zoom;
            int* src = pixels_og;
            int* pixels = new int[static_cast<int>(width * zoom * height * zoom)];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int srcidx =  y * width + x;
                    if (zoom < 1) {
                        if (x % fact == 0 && y % fact == 0) {
                            pixels[y/fact * width/fact + x/fact] = src[srcidx];
                        }
                    } else {
                        for (int i = 0; i < izoom; i++) {
                            for (int j = 0; j < izoom; j++) {
                                pixels[(i+y*izoom) * width*izoom + x*izoom + j] = src[srcidx];
                            }
                        }
                    }
                }
            }
            if (zoom > 1) {
                if (pixels_zoomin.size() < zoom+1) {
                    pixels_zoomin.resize(zoom+1);
                }
                pixels_zoomin[zoom] = pixels;
            } else if (zoom < 1){
                int idx = -fastlog2(zoom);
                if ((int)pixels_zoomout.size() < idx+1) {
                    pixels_zoomout.resize(idx+1);
                }
                pixels_zoomout[idx] = pixels;
            }
            return pixels;
        }
};

class TextureManager {
    public:
        const std::string GENERATED_TOKEN = "__";

        TextureManager() {
            letter_to_texture.resize(1024);
        }

        void add_folder(const std::string& folder) {
            for (auto& fil : std::filesystem::recursive_directory_iterator(folder)) {
                std::string filename = fil.path().filename().string();
                std::string filepath = fil.path().string();
                if (filename.find(".bmp") != std::string::npos) {
                    std::string name = filename.substr(0, filename.find("."));
                    Texture* t  = new Texture(currentID, filepath, name);
                    id_to_texture[currentID] = t;
                    name_to_texture[name] = t;
                    currentID++;
                }
            }
        }

        Texture* get(Texture::ID id) { return id_to_texture[id]; }
        
        Texture* get(const std::string& name) {
            if (name_to_texture.find(name) != name_to_texture.end()) {
                return name_to_texture[name]; 
            } else if (name.find(GENERATED_TOKEN) != std::string::npos) {
                return get_generated(name);
            }
            return nullptr;
        }

        Texture* get(char letter, int size) {
            if (letter_to_texture[size].empty()) {
                init_letters(size);
            }
            return letter_to_texture[size][letter]; 
        }

    private:
        Texture* id_to_texture[15000] = {0};
        std::unordered_map<std::string, Texture*> name_to_texture;
        std::vector<std::vector<Texture*>> letter_to_texture;
        Texture::ID currentID = 1;
        
        int darken_pixel(int pixel) {
            constexpr unsigned char d = 80;
            int out = 0;
            unsigned char* px_in =  (unsigned char*)&pixel;
            unsigned char* px_out =  (unsigned char*)&out;
            px_out[0] = px_in[0] > d ? px_in[0] - d : 0;
            px_out[1] = px_in[1] > d ? px_in[1] - d : 0;
            px_out[2] = px_in[2] > d ? px_in[2] - d : 0;
            px_out[3] = px_in[3] > d ? px_in[3] - d : 0;
            return out;
        }

        Texture* get_generated(const std::string& name) {
            std::string basename = name.substr(0, name.find(GENERATED_TOKEN)); 
            std::string postfix = name.substr(name.find(GENERATED_TOKEN), name.size() - name.find(GENERATED_TOKEN)); 
            if (name_to_texture.find(basename) == name_to_texture.end()) {
                return nullptr;
            }
            Texture* t = name_to_texture[basename];
            Size s = t->size();
            Texture* gen_texture = new Texture(0x0, s);
            //gen_texture->set_transparent(true);
            std::memcpy(gen_texture->pixels(), t->pixels(), s.w * s.h * sizeof(int));
            int* pixels = gen_texture->pixels();
            if (postfix.find("top") != std::string::npos) {
                for (int x = 0; x < s.w; x++) {
                    pixels[x] = darken_pixel(pixels[x]);
                    pixels[x + s.w] = darken_pixel(pixels[x + s.w]);
                }
            }
            if (postfix.find("bottom") != std::string::npos) {
                for (int x = 0; x < s.w; x++) {
                    pixels[x + (s.w-1)*s.h] = darken_pixel(pixels[x + (s.w-1)*s.h]);
                    pixels[x + (s.w-2)*s.h] = darken_pixel(pixels[x + (s.w-2)*s.h]);
                }
            }
            if (postfix.find("left") != std::string::npos) {
                for (int x = 0; x < s.h; x++) {
                    pixels[x * s.w] = darken_pixel(pixels[x * s.w]);
                    pixels[x * s.w + 1] = darken_pixel(pixels[x * s.w + 1]);
                }
            }
            if (postfix.find("right") != std::string::npos) {
                for (int x = 0; x < s.h; x++) {
                    pixels[x * s.w + s.w - 1] = darken_pixel(pixels[x * s.w + s.w - 1]);
                    pixels[x * s.w + s.w - 2] = darken_pixel(pixels[x * s.w + s.w - 2]);
                }
            }
            gen_texture->set_id(currentID);
            id_to_texture[currentID] = gen_texture;
            name_to_texture[name] = gen_texture;
            currentID++;
            return gen_texture;
        }

        void init_letters(int size) {
            TTF_Font * font = TTF_OpenFont("res/mono.ttf", size);
            SDL_Color fgcolor = {255, 255, 255, 255};
            letter_to_texture[size].resize(128);
            union LU {
                char c[2] = {0, 0};
                unsigned short us;
            };
            LU letter;
            for (char c = 32; c < 127; c++) {
                letter.c[0] = c;
                SDL_Surface* surface = TTF_RenderGlyph_Blended(font, letter.us, fgcolor);
                letter_to_texture[size][c] = new Texture(surface);
                SDL_FreeSurface(surface);
            }
        }
};

#endif
