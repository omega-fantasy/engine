#ifndef TILEMAP_H
#define TILEMAP_H

#include "engine.h"
#include "db.h"
#include "screen.h"
#include "input.h"
#include "config.h"
#include <random>
#include <chrono>
#include <utility>
#include <vector>
#include <algorithm>

class Tilemap : public Composite, Input::Listener {
    public:
        class Listener {
            public:
                virtual void tile_clicked(Point) {}
                virtual void map_changed() {}
        };
        
        Tilemap(Size screen_size): Composite(screen_size) {}

        void create_map(Size screen_size) {
            map_size = {std::stoi(Engine.config()->get("settings")["mapsize"]["width"]), std::stoi(Engine.config()->get("settings")["mapsize"]["height"])};
           tile_dim = {std::stoi(Engine.config()->get("settings")["tilesize"]["width"]), std::stoi(Engine.config()->get("settings")["tilesize"]["height"])};
            size = screen_size;
            tiles_ground = Engine.db()->get_matrix<Texture::ID>("tiles", map_size.w, map_size.h);
            tiles_above = Engine.db()->get_matrix<Texture::ID>("buildings", map_size.w, map_size.h);
            for (auto& listener : click_listeners) {
                listener->map_changed();
            }
        }

        bool set_ground(const std::string& texture_name, Point p, bool blocked) {
            Texture* texture = Engine.textures()->get(texture_name);
            tiles_ground->get(p.x, p.y) = blocked ? -texture->id() : texture->id();
            return true;
        }
        
        bool set_tile(const std::string& texture_name, Point p) {
            Texture* texture = Engine.textures()->get(texture_name);
            Size s = texture->size() / tile_dim;  
            return set(texture->id(), p, s);
        }

        Point texture_root(Point p) {
            Texture::ID id = tiles_above->get(p.x, p.y);
            if (id < 0) {
                Size s = Engine.textures()->get(-id)->size() / tile_dim;
                for (short y = p.y; y > p.y - s.h && y >= 0; y--) { 
                    for (short x = p.x; x > p.x - s.w && x >= 0 ; x--) {
                        Texture::ID current_id = tiles_above->get(x, y);
                        if (current_id == -id) {
                            return {x, y};
                        }
                    }
                }
            }
            return p;
        }

        void unset_tile(Point pos) {
            Point p = texture_root(pos);
            Texture::ID id = tiles_above->get(p.x, p.y);
            if (id <= 0) {
                return;
            }
            Size s = Engine.textures()->get(id)->size() / tile_dim;
            for (short y = p.y; y < p.y + s.h; y++) { 
                for (short x = p.x; x < p.x + s.w; x++) {
                    if ((x == p.x && y == p.y) || tiles_above->get(x, y) < 0) {
                        tiles_above->get(x, y) = 0;
                    }
                }
            }
        }

        Texture::ID get_ground(Point p) { 
            Texture::ID id = tiles_ground->get(p.x, p.y);
            return id < 0 ? -id : id; 
        }
        
        void move_cam(Point p) {
            camera_pos = camera_pos + p;
            if ((camera_pos.x + size.w) / (zoom * tile_dim.w) > map_size.w) {
                camera_pos.x = (map_size.w * tile_dim.w * zoom - size.w);
            }
            if ((camera_pos.y + size.h) / (zoom * tile_dim.h) > map_size.h) {
                camera_pos.y = (map_size.h * tile_dim.h * zoom - size.h);
            }
            fix_camera();
            set_update(true);
        }

        void move_cam_to_tile(Point tile_pos) {
            camera_pos = {tile_pos.x * tile_dim.w * zoom, tile_pos.y * tile_dim.h * zoom};
            Box visible = visible_tiles();
            BigPoint mid = {zoom * tile_dim.w * (visible.b.x - visible.a.x) / 2, zoom * tile_dim.h * (visible.b.y - visible.a.y) / 2};
            camera_pos = camera_pos - mid;
            fix_camera();
        }

        void zoom_cam(int factor) {
            if (factor > 0 && zoom * factor <= MAX_ZOOM) {
                move_cam({size.w/4, size.h/4});
                camera_pos.x *= factor;
                camera_pos.y *= factor;
                zoom *= factor;
            } else if (zoom * -((double)1/factor) >= MIN_ZOOM) {
                BigPoint camera_before = camera_pos;
                move_cam({-size.w/2, -size.h/2});
                camera_pos.x /= -factor;
                camera_pos.y /= -factor;
                zoom *= -((double)1/factor);
                fix_camera();
                if (camera_pos.x < 0 || camera_pos.y < 0) { // screen to small to contain map
                    zoom /= -((double)1/factor);
                    camera_pos = camera_before;
                }
            }
        }

        void set_cursor_texture(const std::string& name) { cursor_texture = name; }

        Size tilemap_size() { return map_size; }
        Size tile_size() { return tile_dim; }
        float camera_zoom() { return zoom; }
        void set_zoom(float z) { zoom = z; }
        BigPoint camera_position() { return camera_pos; }
        void add_listener(Tilemap::Listener* l) { click_listeners.push_back(l); }
        void remove_listener(Tilemap::Listener* l) { click_listeners.erase(std::find(click_listeners.begin(), click_listeners.end(), l)); }
        
        Box visible_tiles() {
            short xstart = (camera_pos.x) / (zoom * tile_dim.w);
            short xend = (camera_pos.x + size.w) / (zoom * tile_dim.w);
            short ystart = (camera_pos.y) / (zoom * tile_dim.h);
            short yend = (camera_pos.y + size.h) / (zoom * tile_dim.h);
            short pad = 1;
            xstart = xstart > pad ? xstart - pad : 0;
            ystart = ystart > pad ? ystart - pad : 0;
            xend   = xend + pad < map_size.w ? xend + pad : map_size.w - 1; 
            yend   = yend + pad < map_size.h ? yend + pad : map_size.h - 1; 
            return {Point(xstart, ystart), Point(xend, yend)};
        }
        
        class Config {
            public:
            struct Item {
                Item(const std::string& n, double p) : name(n), perc(p) {}
                std::string name;
                double perc;
                Size size() { return m_size; }
                Texture::ID id() {
                    if (m_id < 0) {
                        Texture* t = Engine.textures()->get(name);
                        m_id = t->id();
                        m_size = t->size();
                    }
                    return m_id;
                }
                private:
                    Texture::ID m_id = -1;
                    Size m_size = {0, 0};
            };
            struct Biome {
                Biome(const std::string& n, double p, bool b, const std::vector<Item>& i) : name(n), perc(p), blocking(b), items(i) {}
                std::string name;
                double perc;
                bool blocking;
                std::vector<Item> items;
                Texture::ID id() {
                    if (m_id < 0) {
                        m_id = Engine.textures()->get(name)->id();
                    }
                    return m_id;
                }
                private:
                    Texture::ID m_id = -1;
            };
            Config() {
                auto& configfile = Engine.config()->get("mapgen");
                for (auto& biome : configfile["biomes"]) {
                    std::vector<Config::Item> items;
                    for (auto& item : biome["vegetation"]) {
                        items.emplace_back(item["name"], std::stod(item["quantity"]));
                    }
                    biomes.push_back({biome["name"], std::stod(biome["quantity"]), (bool)std::stoi(biome["blocking"]), items});
                }
                num_cells = std::stoi(configfile["num_cells"]);
                sample_factor = std::stoi(configfile["sample_factor"]);
                sample_distance = std::stoi(configfile["sample_distance"]);
            }
            std::vector<Biome> biomes;
            short num_cells = 0;
            short sample_factor = 0;
            short sample_distance = 0;
        };

        void randomize_map() {
            for (auto& listener : click_listeners) {
                listener->map_changed();
            }
            unsigned seed = (unsigned)(std::chrono::system_clock::now().time_since_epoch().count());
            auto generator = std::default_random_engine(seed);
            std::uniform_int_distribution<int> distribution(0, 2147483647);
            unsigned long long r = distribution(generator);
            auto rnd = [&]() {
                r = r * 48271 % 2147483648;
                return (double)r / 2147483648;
            };

            Config config;
            Size num_cells = {config.num_cells, config.num_cells};
            Size cell_size = map_size / num_cells;
            int max_samples = cell_size.w * cell_size.h * config.sample_factor; // 3
            int sample_dist = config.sample_distance; // 2

            std::vector<std::pair<Point, double>> anchors;
            for (short y = 0; y < num_cells.h; y++) {
                for (short x = 0; x < num_cells.w; x++) {
                    anchors.push_back({{(x + rnd()) * cell_size.w, (y + rnd()) * cell_size.h}, rnd()}); 
                }
            }
            for (short y_map = 0; y_map < map_size.h; y_map++) {
                for (short x_map = 0; x_map < map_size.w; x_map++) {
                    tiles_ground->get(x_map, y_map) = 0;
                    tiles_above->get(x_map, y_map) = 0;
                }
            }

            std::vector<std::pair<Point, double>> current_anchors;
            for (short y_cell = 0; y_cell < num_cells.h; y_cell++) {
                for (short x_cell = 0; x_cell < num_cells.w; x_cell++) {
                    Box sample_cells(
                        Point(x_cell > sample_dist ? x_cell - sample_dist : 0,
                              y_cell > sample_dist ? y_cell - sample_dist : 0), 
                        Point(x_cell + sample_dist < num_cells.w - 1 ? x_cell + sample_dist : num_cells.w - 1, 
                              y_cell + sample_dist < num_cells.h - 1 ? y_cell + sample_dist : num_cells.h - 1)
                    );
                    current_anchors.clear();
                    for (short y_cells = sample_cells.a.y; y_cells <= sample_cells.b.y; y_cells++) {
                        for (short x_cells = sample_cells.a.x; x_cells <= sample_cells.b.x; x_cells++) {
                            current_anchors.push_back(anchors[y_cells * num_cells.w + x_cells]);
                        }
                    }
                    for (short y_map = y_cell * cell_size.h; static_cast<short>(y_map < (y_cell + 1) * cell_size.h); y_map++) {
                        for (short x_map = x_cell * cell_size.w; static_cast<short>(x_map < (x_cell + 1) * cell_size.w); x_map++) {
                            double total_val = 0;
                            int total_samples = 0;
                            for (auto& anchor : current_anchors) {
                                short dist = anchor.first.distance({x_map, y_map}); 
                                int num_samples = max_samples - dist * dist;
                                if (num_samples < 0) {
                                    num_samples = 1;
                                }
                                total_val += num_samples * anchor.second;
                                total_samples += num_samples;
                            }
                            total_val /= total_samples;

                            double current_val = 0;
                            for (Tilemap::Config::Biome& biome : config.biomes) {
                                current_val += biome.perc;
                                if (total_val - current_val <= 0.01) {
                                    tiles_ground->get(x_map, y_map) = biome.blocking ? -biome.id() : biome.id();
                                    double val = rnd();
                                    current_val = 0;
                                    for (auto& item : biome.items) {
                                        current_val += item.perc;
                                        if (val - current_val <= 0.01) {
                                            set(item.id(), {x_map, y_map}, item.size() / tile_dim);
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

    private:
        std::vector<Tilemap::Listener*> click_listeners;
        constexpr static double MAX_ZOOM = 4.0;
        constexpr static double MIN_ZOOM = 0.125;
        bool listener_registered = false;
        Matrix<Texture::ID>* tiles_ground = nullptr;
        Matrix<Texture::ID>* tiles_above = nullptr;
        Size tile_dim = {0, 0};
        Size map_size = {0, 0};
        BigPoint camera_pos = {0, 0};
        float zoom = 1;
        std::string cursor_texture;
        Point last_mouse_pos = {0, 0};

        void mouse_clicked(Point p) {
            BigPoint click_pos = camera_pos + p - pos;
            Point click_tile = {click_pos.x / (tile_dim.w * zoom), click_pos.y / (tile_dim.h * zoom)};
            for (auto& l : click_listeners) {
                l->tile_clicked(click_tile);
            }
        }

        void fix_camera() {
            if (camera_pos.x < 0) camera_pos.x = 0;
            if (camera_pos.y < 0) camera_pos.y = 0;
            BigPoint camera_max = {(zoom * tile_dim.w) * map_size.w - size.w, (zoom * tile_dim.h) * map_size.h - size.h};
            camera_pos.x = camera_pos.x < camera_max.x ? camera_pos.x : camera_max.x;
            camera_pos.y = camera_pos.y < camera_max.y ? camera_pos.y : camera_max.y;
        }

        void draw() {
            if (!listener_registered) {
                Engine.input()->addMouseClickListener(this, {pos, size});
                listener_registered = true;
            }

            Texture* t = Engine.textures()->get(cursor_texture);
            Box b(pos, size);
            Point mouse_pos = Engine.input()->mouse_pos();
            bool do_update = t && b.inside(mouse_pos) && mouse_pos != last_mouse_pos;

            if (needs_update() || do_update) {
                Box visible = visible_tiles();
                for (short y = visible.a.y; y <= visible.b.y; y++) { 
                    for (short x = visible.a.x; x <= visible.b.x; x++) { 
                        Texture::ID id = tiles_ground->get(x,y);
                        Engine.screen()->blit(Engine.textures()->get(id < 0 ? -id : id), 
                        {pos.x - camera_pos.x + x * tile_dim.w * zoom, pos.y - camera_pos.y + y * tile_dim.h * zoom },
                        {pos, size}, zoom);
                    }
                }
                for (short y = visible.a.y; y <= visible.b.y; y++) {
                    for (short x = visible.a.x; x <= visible.b.x; x++) {
                        Texture::ID id = tiles_above->get(x, y);
                        if (id > 0)
                        Engine.screen()->blit(Engine.textures()->get(id),
                        {pos.x - camera_pos.x + x * tile_dim.w * zoom, pos.y - camera_pos.y + y * tile_dim.h * zoom},
                        {pos, size}, zoom);
                    }
                }        
                if (t && b.inside(mouse_pos)) { // snap to tile
                    BigPoint mouse_abs = camera_pos + mouse_pos - pos;
                    Point tile_abs = { mouse_abs.x / (tile_dim.w * zoom), mouse_abs.y / (tile_dim.h * zoom) };
                    mouse_abs = { tile_abs.x * (tile_dim.w * zoom), tile_abs.y * (tile_dim.h * zoom) };
                    tile_abs = { mouse_abs.x - camera_pos.x + pos.x, mouse_abs.y - camera_pos.y + pos.y };
                    Engine.screen()->blit(t, tile_abs, Box(pos, size), zoom);
                }
                last_mouse_pos = mouse_pos;
                set_update(false);
            }
            Composite::draw();
        }

        bool set(Texture::ID id, Point p, Size s) {
            if (p.x + s.w >= tiles_ground->width() || p.y + s.h >= tiles_ground->height()) {
                return false;
            }
            for (short y = p.y; y < p.y + s.h; y++) {
                for (short x = p.x; x < p.x + s.w; x++) {
                    if (tiles_ground->get(x, y) < 0 || tiles_above->get(x, y) != 0) {
                        return false;
                    }
                }
            }
            for (short y = p.y; y < p.y + s.h; y++) {
                for (short x = p.x; x < p.x + s.w; x++) {
                    if (x == p.x && y == p.y) {
                        tiles_above->get(p.x, p.y) = id;
                    } else {
                        tiles_above->get(x, y) = -id;
                    }
                }
            }
            return true;
        }
};

#endif
