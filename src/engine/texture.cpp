#include "texture.h"
#include "engine.h"
#include "config.h"
#include "db.h"

Texture::Texture(Size s, Color* pixels): m_size(s), pixels_og(pixels) {
    hasTransparency = std::any_of(pixels, pixels+s.w*s.h, [](Color p){return p.alpha < 128;});
}
        
Texture::Texture(Color color, Size s):  m_size(s), pixels_og(new Color[s.w * s.h]) {
    std::fill(pixels_og, pixels_og + s.w * s.h, int(color));
}

Texture::~Texture() {
    delete[] pixels_og;
    for (Color* t : pixels_zoomin) {
        delete[] t;
    }
    for (Color* t : pixels_zoomout) {
        delete[] t;
    }
}

Color* Texture::load_scaled(float zoom) {
    int fact = (double)1 / zoom;
    int izoom = (int)zoom;
    Color* src = pixels_og;
    Color* pixels = new Color[static_cast<int>(m_size.w * zoom * m_size.h * zoom)];
    for (int y = 0; y < m_size.h; y++) {
        for (int x = 0; x < m_size.w; x++) {
            int srcidx =  y * m_size.w + x;
            if (zoom < 1) {
                if (x % fact == 0 && y % fact == 0) {
                    pixels[y/fact * m_size.w/fact + x/fact] = src[srcidx];
                }
            } else {
                for (int i = 0; i < izoom; i++) {
                    for (int j = 0; j < izoom; j++) {
                        pixels[(i+y*izoom) * m_size.w*izoom + x*izoom + j] = src[srcidx];
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
        if ((int)pixels_zoomout.size() < idx+1) {
            pixels_zoomout.resize(idx+1);
        }
        pixels_zoomout[idx] = pixels;
    }
    return pixels;
}




// Texture generation magic

static Color randomize(Color c, double variance) {
    double gauss = random_gauss(0, variance);
    if (gauss < 0) {
        return c - (unsigned char)(255 * -gauss);
    } else {
        return c + (unsigned char)(255 * gauss);
    }
    return 0;
}

static Texture* generate_noise(Size s, Color c, double variance) {
    Color* img = new Color[s.w * s.h];
    std::for_each(img, img+s.w*s.h, [&](Color& p){ p = randomize(c, variance); });
    return new Texture(s, img);
}

static Texture* generate_tiled(Size s, Color c, double variance, int tiles_horizontal, int tiles_vertical, double offset) {
    Color* img = new Color[s.w * s.h];
    Color color_sep = c - (unsigned char)30;
    Size tile_size(s.w / tiles_horizontal, s.h / tiles_vertical);
    int offset_x = 0;
    for (int y = 0; y < s.h; y++) {
        if (y % tile_size.h == 0) {
            offset_x = (int)(offset_x + offset * tile_size.w) % tile_size.w;
        }
        for (int x = 0; x < s.w; x++) {
            bool sep = y % tile_size.h == 0 || (x + offset_x) % tile_size.w == 0;
            img[y * s.w + x] = sep ? randomize(color_sep, 0.01) : randomize(c, variance);
        }
    }
    return new Texture(s, img);
}

static Texture* generate_building(Size tile_size, Size building_size, short depth, const std::string& facade, const std::string& roof, const std::vector<std::pair<std::string, Point>>& objects) {
    constexpr int shadow_width = 1;
    Size s((building_size.w + shadow_width) * tile_size.w, (building_size.h + depth) * tile_size.h);
    Color* img = new Color[s.w * s.h];
    Color* pixels_facade = Engine.textures()->get(facade)->pixels();
    Color* pixels_roof = Engine.textures()->get(roof)->pixels();
    for (int y = 0; y < s.h; y++) {
        Color* src = y < depth * tile_size.h ? pixels_roof : pixels_facade;
        for (int x = 0; x < s.w; x++) {
            if (x < building_size.w * tile_size.w) {
                img[y * s.w + x] = src[(y % tile_size.h) * tile_size.w + (x % tile_size.w)];
            } else if (y / tile_size.h > building_size.h - 1 &&
                       x % tile_size.w < tile_size.w / 2 && 
                       (y < s.h - tile_size.h || x % tile_size.w < tile_size.h - y % tile_size.h)) {
                img[y * s.w + x] = Color(0, 0, 0, 100);
            }
        }
    }
    for (auto& object : objects) {
        Texture* t = Engine.textures()->get(object.first);
        Color* src = t->pixels();
        Point pos = object.second;
        Size object_size = t->size();
        int offset = pos.x * tile_size.w + pos.y  * tile_size.h * s.w;
        for (int y = 0; y < object_size.h; y++) {
            for (int x = 0; x < object_size.w; x++) {
                img[offset + y * s.w + x] = src[y * object_size.w + x];
            }
        }
    }
    return new Texture(s, img);
}

static Texture* generate_plant(Size s, Color color_crown, Color color_trunk, double variance, double crown_width, double crown_height, double trunk_width, double trunk_height) {
    Color* img = new Color[s.w * s.h];
    std::fill(img, img + s.w * s.h, int(Color(0,0,0,0)));
    Box trunk(Point(s.w * (1 - trunk_width) / 2 - 1, s.h * (1 - trunk_height) - 1), Point(s.w * trunk_width + s.w * (1 - trunk_width) / 2 - 1, 1.0 * s.h - 1));
    Box crown(Point(s.w * (1 - crown_width) / 2 - 1, s.h * (1 - crown_height) - trunk.size().h - 1), Point(s.w * crown_width  + s.w * (1 - crown_height) / 2 - 1, 1.0 * s.h - 1 - trunk.size().h));

    for (int y = 0; y < s.h; y++) {
        if (y >= crown.a.y && y <= crown.b.y) {
            double d = 1 - ((double)std::abs(y - crown.center().y) / (crown.size().h /2));
            double base = y > crown.center().y ? 0.6 : 0.35;
            if (trunk.size().h == 0) {
                base += 0.1;
            }
            short x_range = base * crown.size().w + (1 - base) * crown.size().w * d + (int)random_uniform(0, 1 + 0.1 * s.w);
            for (int x = s.w / 2 - x_range / 2; x < s.w / 2 + x_range / 2; x++) {
                unsigned char diff = 150 * ((double)crown.center().distance({x, y}) / (crown.size().w / 2 + crown.size().h / 2));
                img[y * s.w + x] = randomize(color_crown - diff, variance);
            }
        } else if (y >= trunk.a.y && y <= trunk.b.y) {
            double d = 1 - ((double)std::abs(y - trunk.center().y) / (trunk.size().h /2));
            double base = y > trunk.center().y ? 0.7 : 0.35;
            short x_range = base * trunk.size().w + (1 - base) * trunk.size().w * d + (int)random_uniform(0, 1 + 0.1 * s.w);
            for (int x = s.w / 2 - x_range / 2; x < s.w / 2 + x_range / 2; x++) {
                unsigned char diff = 160 * ((double)trunk.center().distance({x, y}) / (trunk.size().w / 2 + trunk.size().h / 2));
                img[y * s.w + x] = randomize(color_trunk - diff, variance);
            }           
        
        }
    }

    return new Texture(s, img);
}

static Texture* generate_debris(Size s, Color, Color, Color, Size ) {
    Color* img = new Color[s.w * s.h];


    return new Texture(s, img);
}





TextureManager::TextureManager() {
    letter_to_texture.resize(1024);
}

TextureManager::~TextureManager() {
    for (auto& pair : name_to_texture) {
        delete pair.second;
    }
    for (auto& sizes : letter_to_texture) {
        for (auto t : sizes) {
            delete t;
        }
    }
}

void TextureManager::reinit() {
    auto table = Engine.db()->get_table<String<256>>("textures");
    for (auto it = table->begin(); it != table->end(); ++it) {
        Texture::ID id = it.key();
        std::string name = (*it).toStdString();
        Texture* t = nullptr;
        if (name_to_texture.find(name) == name_to_texture.end()) {
            t = generate_texture(name);
            name_to_texture[name] = t;
        } else {
            t = name_to_texture[name];
        }
        t->m_id = id;
        id_to_texture[id] = t;
    }
}

void TextureManager::register_texture(const std::string& name, Texture* t) {
    t->m_id = currentID;
    id_to_texture[currentID] = t;
    name_to_texture[name] = t;
    Engine.db()->get_table<String<256>>("textures")->add(currentID, name);
    currentID++;
    for (float z : {0.125, 0.25, 0.5, 1.0, 2.0, 4.0}) {
        t->load_scaled(z);
    }
}

void TextureManager::add_folder(const std::string& folder) {
    auto& config = Engine.config()->get("textures");
    for (auto& g : config["generate"]) {
        Size s(std::stoi(Engine.config()->get("settings")["tilesize"]["width"]), std::stoi(Engine.config()->get("settings")["tilesize"]["height"]));
        std::string type = g["type"];
        Texture* t = nullptr;
        if (type == "building") {
            std::vector<std::pair<std::string, Point>> objects;
            for (auto& object : g["objects"]) {
                objects.emplace_back(object["name"], Point(std::stoi(object["x"]), std::stoi(object["y"])));
            }
            t = generate_building(s, Size(std::stoi(g["width"]), std::stoi(g["height"])), std::stoi(g["depth"]), g["facade"], g["roof"], objects);
        } else if (type == "plant") {
            Size size(s.w * std::stoi(g["width"]), s.h * std::stoi(g["height"]));
            Color color_crown = (unsigned)std::stoul(g["color_crown"], nullptr, 0);
            Color color_trunk = (unsigned)std::stoul(g["color_trunk"], nullptr, 0);
            t = generate_plant(size, color_crown, color_trunk, std::stod(g["variance"]), std::stod(g["width_crown"]), std::stod(g["height_crown"]), std::stod(g["width_trunk"]), std::stod(g["height_trunk"])); 
        } else {
            Color c(std::stoi(g["red"]), std::stoi(g["green"]), std::stoi(g["blue"]));
            if (type == "noise") {
                t = generate_noise(s, c, std::stod(g["variance"]));
            } else if (type == "tiled") {
                t = generate_tiled(s, c, std::stod(g["variance"]), std::stoi(g["tiles_horizontal"]), std::stoi(g["tiles_vertical"]), std::stod(g["offset"]));
            } 
        }
        if (t) {
            register_texture(g["name"], t);
        }
    }
    for (auto& filepath : filelist(folder, ".bmp")) {
        auto bmp = load_bmp(filepath);
        if (name_to_texture.find(filename(filepath)) == name_to_texture.end()) {
            register_texture(filename(filepath), new Texture(bmp.second, bmp.first));
        }
    }
}

Texture* TextureManager::get(Texture::ID id) {
    if (!id_to_texture[id]) {
        return nullptr;
    }
    return id_to_texture[id]; 
}
        
Texture* TextureManager::get(const std::string& name) {
    if (name_to_texture.find(name) != name_to_texture.end()) {
        return name_to_texture[name]; 
    }
    return generate_texture(name);
}

Texture* TextureManager::get(char letter, int size) {
    if (letter_to_texture[size].empty()) {
        init_letters(size);
    }
    return letter_to_texture[size][letter]; 
}

std::string TextureManager::generate_name(const std::string& command, const std::vector<std::string>& params) {
    std::string ret = command; 
    for (auto& p : params) {
        ret += DELIMITER + p;
    }
    return ret;
}

Texture* TextureManager::generate_texture(const std::string& name) {
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

Texture* TextureManager::get_blended(const std::string& base, const std::string& top, const std::string& right, const std::string& bottom, const std::string& left) {
    if (name_to_texture.find(base) == name_to_texture.end()) {
        return nullptr;
    }
    Texture* t1 = name_to_texture[base];
    Size s = t1->size();
    Texture* gen_texture = new Texture(0x0, s);
    std::memcpy(gen_texture->pixels(), t1->pixels(), s.w * s.h * sizeof(int));
    
    const double variance = 0.03;
    int depth = 0.125 * s.w;
    int max_depth = 0.2 * s.w;
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

Texture* TextureManager::get_alpha_bordered(const std::string& basename, const std::string& postfix) {
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

void TextureManager::init_letters(int height, Color color) {
    char start = 32;
    char end = 127;
    letter_to_texture[height].resize(end + 1);
    auto letters = load_letters(fontpath, height, color, start, end);
    for (auto& letter : letters) {
        letter_to_texture[height][start++] = new Texture(letter.second, letter.first);
    }
}