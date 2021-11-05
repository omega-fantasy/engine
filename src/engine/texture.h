#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine.h"
#include "config.h"

class Texture {
    public:
        using ID = short;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        
        Texture(Size s, Color* pixels): width(s.w), height(s.h) {
            hasTransparency = std::any_of(pixels, pixels+width*height, [](Color p){return p.alpha < 128;});
            if (hasTransparency) { // pre-multiply transparency
                std::for_each(pixels, pixels+width*height, [](Color& p){
                    float a_pixel = (float)p[3] / 255;
                    p[0] *= a_pixel; p[1] *= a_pixel; p[2] *= a_pixel;
                });
            }
            pixels_og = (int*)pixels;
        }
        
        Texture(Color color, Size s): width(s.w), height(s.h) {
            pixels_og = new int[s.w * s.h];
            std::fill(pixels_og, pixels_og + s.w * s.h, int(color));
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

        Size size(float zoom = 1) { return Size(width * zoom, height * zoom); }
        bool transparent() { return hasTransparency; }
        void set_transparent(bool b) { hasTransparency = b; }   
        ID id() { return m_id; }
        void set_id(ID i) { m_id = i; }

    protected:
        ID m_id = 0;
        std::vector<int*> pixels_zoomin;
        std::vector<int*> pixels_zoomout;
        int* pixels_og = nullptr;
        bool hasTransparency = false;
        int width = 0;
        int height = 0;

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
                int idx = 0;
                if (zoom == 0.125) idx = 3;
                if (zoom == 0.25) idx = 2;
                if (zoom == 0.5) idx = 1;
                if (zoom == 0.0625) idx = 4;
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
        TextureManager() { letter_to_texture.resize(1024); }

        void register_texture(const std::string& name, Texture* t) {
            t->set_id(currentID);
            id_to_texture[currentID] = t;
            name_to_texture[name] = t;
            currentID++;
        }

        void add_folder(const std::string& folder) {
            for (auto& filepath : filelist(folder, ".bmp")) {
                auto bmp = load_bmp(filepath);
                register_texture(filename(filepath), new Texture(bmp.second, bmp.first));
            }
            auto& config = Engine.config()->get("textures");
            if (!std::stoi(config["use_generated_textures"])) {
                return;
            }
            for (auto& g : config["generate"]) {
                Color c(std::stoi(g["red"]), std::stoi(g["green"]), std::stoi(g["blue"]));
                Size s(std::stoi(Engine.config()->get("settings")["tilesize"]["width"]), std::stoi(Engine.config()->get("settings")["tilesize"]["height"]));
                register_texture(g["name"], generate(s, c, std::stod(g["variance"])));
            }
        }

        Texture* get(Texture::ID id) { 
            if (!id_to_texture[id]) {
                return nullptr;
            }
            return id_to_texture[id]; 
        }
        
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
        std::map<std::string, Texture*> name_to_texture;
        std::vector<std::vector<Texture*>> letter_to_texture;
        Texture::ID currentID = 1;

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
            Color* pixels = (Color*)gen_texture->pixels();
            unsigned char d = 60;
            if (postfix.find("top") != std::string::npos) {
                for (int x = 0; x < s.w; x++) {
                    pixels[x] = pixels[x] - d;
                    pixels[x + s.w] = pixels[x + s.w] - d;
                }
            }
            if (postfix.find("bottom") != std::string::npos) {
                for (int x = 0; x < s.w; x++) {
                    pixels[x + (s.w-1)*s.h] = pixels[x + (s.w-1)*s.h] - d;
                    pixels[x + (s.w-2)*s.h] = pixels[x + (s.w-2)*s.h] - d;
                }
            }
            if (postfix.find("left") != std::string::npos) {
                for (int x = 0; x < s.h; x++) {
                    pixels[x * s.w] = pixels[x * s.w] - d;
                    pixels[x * s.w + 1] = pixels[x * s.w + 1] - d;
                }
            }
            if (postfix.find("right") != std::string::npos) {
                for (int x = 0; x < s.h; x++) {
                    pixels[x * s.w + s.w - 1] = pixels[x * s.w + s.w - 1] - d;
                    pixels[x * s.w + s.w - 2] = pixels[x * s.w + s.w - 2] - d;
                }
            }
            register_texture(name, gen_texture);
            return gen_texture;
        }

        Texture* generate(Size s, Color c, double variance) {
            Color* img = new Color[s.w * s.h];
            std::for_each(img, img+s.w*s.h, [&](Color& p){
                double gauss = random_gauss(0, variance);
                if (gauss < 0) {
                    p = c - (unsigned char)(255 * -gauss);
                } else {
                    p = c + (unsigned char)(255 * gauss);
                }
            });
            return new Texture(s, img);
        }

        void init_letters(int height, Color color = {255, 255, 255}) {
            char start = 32;
            char end = 127;
            letter_to_texture[height].resize(end + 1);
            auto letters = load_letters("./res/mono.ttf", height, color, start, end);
            for (auto& letter : letters) {
                letter_to_texture[height][start++] = new Texture(letter.second, letter.first);
            }
        }
};

#endif
