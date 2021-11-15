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
            pixels_og = (int*)pixels;
        }
        
        Texture(Color color, Size s): width(s.w), height(s.h) {
            pixels_og = new int[s.w * s.h];
            std::fill(pixels_og, pixels_og + s.w * s.h, int(color));
        }

        virtual ~Texture() {
            delete[] pixels_og;
            for (int* t : pixels_zoomin) {
                delete[] t;
            }
            for (int* t : pixels_zoomout) {
                delete[] t;
            }
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
        int* pixels_og = nullptr;
        ID m_id = 0;
        std::vector<int*> pixels_zoomin;
        std::vector<int*> pixels_zoomout;
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
            }
            return generate_texture(name);
        }

        Texture* get(char letter, int size) {
            if (letter_to_texture[size].empty()) {
                init_letters(size);
            }
            return letter_to_texture[size][letter]; 
        }

        constexpr static char DELIMITER = '$';
        std::string generate_name(const std::string& command, const std::vector<std::string>& params) {
            std::string ret = command; 
            for (auto& p : params) {
                ret += DELIMITER + p;
            }
            return ret;
        }

    private:
        Texture* id_to_texture[15000] = {0};
        std::map<std::string, Texture*> name_to_texture;
        std::vector<std::vector<Texture*>> letter_to_texture;
        Texture::ID currentID = 1;

        Texture* generate_texture(const std::string& name) {
            std::vector<std::string> params = split(name, DELIMITER);
            if (params.empty()) return nullptr;
            Texture* t = nullptr;
            if (params[0] == "blend") {
                t = get_blended(params[1], params[2], params[3], params[4], params[5]);
                register_texture(name, t);
            } else if (params[0] == "border_alpha") {
                t = get_alpha_bordered(params[1], params[2]); 
                register_texture(name, t);
            }
            return t;
        }

        Color randomize(Color c, double variance) {
            double gauss = random_gauss(0, variance);
            if (gauss < 0) {
                return c - (unsigned char)(255 * -gauss);
            } else {
                return c + (unsigned char)(255 * gauss);
            }
            return 0;
        }

        Texture* get_blended(const std::string& base, const std::string& top, const std::string& right, const std::string& bottom, const std::string& left) {
            if (name_to_texture.find(base) == name_to_texture.end()) {
                return nullptr;
            }
            Texture* t1 = name_to_texture[base];
            Size s = t1->size();
            Texture* gen_texture = new Texture(0x0, s);
            std::memcpy(gen_texture->pixels(), t1->pixels(), s.w * s.h * sizeof(int));
            
            const double variance = 0.03;
            int depth = 2;
            int max_depth = 3;
            Color* target = (Color*)gen_texture->pixels();
            if (!left.empty()) {
                Color* src = (Color*)name_to_texture[left]->pixels();
                for (int y = 0; y < s.h; y++) {
                    for (int x = 0; x < depth; x++) {
                        target[y * s.w + x] = randomize(src[(s.h-1) * (s.w-1) / 4], variance);
                    }
                    depth += random_uniform(-1, 2);
                    if (depth < 1) depth = 1;
                    if (depth > max_depth) depth = max_depth;
                }
            }
            if (!right.empty()) {
                Color* src = (Color*)name_to_texture[right]->pixels();
                for (int y = 0; y < s.h; y++) {
                    for (int x = s.w-1-depth; x < s.w; x++) {
                        target[y * s.w + x] = randomize(src[(s.h-1) * (s.w-1) / 4], variance);
                    }
                    depth += random_uniform(-1, 2);
                    if (depth < 1) depth = 1;
                    if (depth > max_depth) depth = max_depth;
                }
            }
            if (!top.empty()) {
                Color* src = (Color*)name_to_texture[top]->pixels();
                for (int x = 0; x < s.w; x++) {
                    for (int y = 0; y < depth; y++) {
                        target[y * s.w + x] = randomize(src[(s.h-1) * (s.w-1) / 4], variance);
                    }
                    depth += random_uniform(-1, 2);
                    if (depth < 1) depth = 1;
                    if (depth > max_depth) depth = max_depth;
                }
            }
            if (!bottom.empty()) {
                Color* src = (Color*)name_to_texture[bottom]->pixels();
                for (int x = 0; x < s.w; x++) {
                    for (int y = s.h-1-depth; y < s.h; y++) {
                        target[y * s.w + x] = randomize(src[(s.h-1) * (s.w-1) / 4], variance);
                    }
                    depth += random_uniform(-1, 2);
                    if (depth < 1) depth = 1;
                    if (depth > max_depth) depth = max_depth;
                }
            }
            return gen_texture;
        }

        Texture* get_alpha_bordered(const std::string& basename, const std::string& postfix) {
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
            return gen_texture;
        }

        Texture* generate(Size s, Color c, double variance) {
            Color* img = new Color[s.w * s.h];
            std::for_each(img, img+s.w*s.h, [&](Color& p){ p = randomize(c, variance); });
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
