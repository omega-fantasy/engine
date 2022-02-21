#ifndef MAPGEN_H
#define MAPGEN_H

#include "engine.h"
#include "db.h"
#include "texture.h"
#include "tilemap.h"

class MapGen {
    public:
        class Config {
          public:
            struct Item {
                Item(const std::string& n, double p): name(n), perc(p) {}
                Size size() { return m_size; }
                Texture::ID id() {
                    if (m_id < 0) {
                        Texture* t = Engine.textures()->get(name);
                        m_id = t->id();
                        m_size = t->size();
                    }
                    return m_id;
                }
                std::string name = "";
                double perc = 0.0;
                Texture::ID m_id = -1;
                Size m_size = {0, 0};
            };
            
            struct Biome {
                Biome(const std::string& n): name(n) {}
                bool operator==(const Biome& b) { return b.m_id == m_id; }
                Texture::ID id() {
                    if (m_id < 0) {
                        m_id = Engine.textures()->get(name)->id();
                    }
                    return m_id;
                }
                std::string name;
                std::string name_wall = "";
                short max_height = 0;
                short wall_height = 0;
                Texture::ID m_id = -1;
                std::vector<Item> items;
            };

            struct Elevation {
                Elevation(double p, bool b, bool bl): perc(p), blocking(b), blend(bl) {}
                double perc = 0.0;
                bool blocking = false;
                bool blend = false;
                std::vector<Biome> biomes;
                std::vector<char> temperatures;
            };
            
            Config() {
                auto& configfile = Engine.config("mapgen");
                for (auto& el : configfile["elevations"]) {
                    auto& elevation = el.second;
                    elevations.emplace_back(elevation["quantity"].d(), (bool)elevation["blocking"].i(), (bool)elevation["blend"].i());
                    auto& new_elevation = elevations.back();
                    for (auto& bm : elevation["biomes"]) {
                        auto& biome = bm.second;
                        new_elevation.biomes.emplace_back(biome["name"].s());
                        new_elevation.temperatures.push_back(biome["temperature_max"].i());
                        auto& new_biome = new_elevation.biomes.back();
                        if (biome.contains("name_wall")) {
                            new_biome.name_wall = biome["name_wall"].s();
                            new_biome.max_height = biome["max_height"].i();
                            new_biome.wall_height = biome["wall_height"].i();
                        }
                        for (auto& item : biome["vegetation"]) {
                            new_biome.items.emplace_back(item.second["name"].s(), item.second["quantity"].d());
                        }
                    }
                }
                num_cells = configfile["num_cells"].i();
                sample_factor = configfile["sample_factor"].i();
                sample_distance = configfile["sample_distance"].i();
            }
            std::vector<Elevation> elevations;
            short num_cells = 0;
            short sample_factor = 0;
            short sample_distance = 0;
        };

        static void post_process(Config& config, Size map_size) {
            auto map = Engine.map();
            std::map<Texture::ID, std::map<Texture::ID, int>> blend_map; 
            std::map<Texture::ID, std::string> name_map; 
            std::vector<Texture::ID> prev_ids;
            for (Config::Elevation& elevation : config.elevations) {
                std::vector<Texture::ID> prev_ids_temp;
                for (auto& biome : elevation.biomes) {
                    if (elevation.blend) {
                        for (auto prev_id : prev_ids) {
                            blend_map[biome.id()][prev_id] = 1;
                        }
                        for (auto& biome2 : elevation.biomes) {
                            if (biome2.id() == biome.id()) {
                                break;
                            }
                            blend_map[biome.id()][biome2.id()] = 1;
                        }
                    }
                    name_map[biome.id()] = biome.name;
                    prev_ids_temp.push_back(biome.id());
                }
                prev_ids = prev_ids_temp;
            }
            std::vector<std::string> params(5);
            for (short y_map = 0; y_map < map_size.h; y_map++) {
                for (short x_map = 0; x_map < map_size.w; x_map++) {
                    Texture::ID current_id = map->get_ground({x_map, y_map});
                    if (blend_map.find(current_id) != blend_map.end()) {
                        auto& blend_set = blend_map[current_id];
                        bool blend = false;
                        if (y_map > 0 && blend_set.find(map->get_ground({x_map+0, y_map-1})) != blend_set.end()) {
                            params[1] = name_map[map->get_ground({x_map+0, y_map-1})]; blend = true;
                        } else { params[1] = ""; }
                        if (x_map < map_size.w-1 && blend_set.find(map->get_ground({x_map+1, y_map+0})) != blend_set.end()) {
                            params[2] = name_map[map->get_ground({x_map+1, y_map-0})]; blend = true;
                        } else { params[2] = ""; }
                        if (y_map < map_size.h-1 && blend_set.find(map->get_ground({x_map+0, y_map+1})) != blend_set.end()) {
                            params[3] = name_map[map->get_ground({x_map+0, y_map+1})]; blend = true;
                        } else { params[3] = ""; }
                        if (x_map > 0 && blend_set.find(map->get_ground({x_map-1, y_map+0})) != blend_set.end()) {
                            params[4] = name_map[map->get_ground({x_map-1, y_map+0})]; blend = true;
                        } else { params[4] = ""; }
                        if (blend) {
                            params[0] = name_map[current_id];
                            std::string n = Engine.textures()->generate_name("blend", params);
                            map->set_ground(n, {x_map, y_map}, false);
                        }
                     }
                }
            }
        }

        struct Anchor {
            static constexpr int PERC_FACTOR = 100000;
            Anchor(Point p, double pc, char t): pos(p), perc(PERC_FACTOR * pc), temp(t) {}
            Point pos;
            long long perc;
            char temp;
        };

        static void randomize_map() {
            auto map = Engine.map();
            Size map_size = map->tilemap_size();
            Size tile_dim = map->tile_size();

            Config config;
            Size num_cells = {config.num_cells, config.num_cells};
            Size cell_size = map_size / num_cells;
            int max_samples = cell_size.w * cell_size.h * config.sample_factor; // 3
            int sample_dist = config.sample_distance; // 2
            
            Matrix<unsigned char>* heightmap = new Matrix<unsigned char>("heightmap", map_size.w, map_size.h);
            Config::Biome& mountain_biome = config.elevations.back().biomes[0];
            unsigned char max_height = mountain_biome.max_height - 1;
            double height_cutoff = 1 - config.elevations.back().perc;
            short wall_height = mountain_biome.wall_height;

            const int climate_cluster_factor = 4;
            double r;
            std::vector<Anchor> anchors;
            for (short y = 0; y < num_cells.h; y++) {
                for (short x = 0; x < num_cells.w; x++) {
                    if (x % climate_cluster_factor == 0 && y % climate_cluster_factor == 0) {
                        r = random_uniform(20, 80);
                    } else {
                        short use_x = x - (x % climate_cluster_factor);
                        short use_y = (y - (y % climate_cluster_factor)) * num_cells.w; 
                        r = anchors[use_y + use_x].temp;
                    }
                    anchors.emplace_back(
                        Point((x + random_fast()) * cell_size.w, (y + random_fast()) * cell_size.h), 
                        random_fast(),
                        r
                    ); 
                }
            }
            /*
            for (short y_map = 0; y_map < map_size.h; y_map++) {
                for (short x_map = 0; x_map < map_size.w; x_map++) {
                    tiles_ground->get(x_map, y_map) = 0;
                    tiles_above->get(x_map, y_map) = 0;
                    heightmap->get(x_map, y_map) = 0;
                }
            }
            */
            parallel_for(0, num_cells.h-1, [&](int y_cell) {
                for (short x_cell = 0; x_cell < num_cells.w; x_cell++) {
                    Box sample_cells(Point(x_cell - sample_dist, y_cell - sample_dist), Point(x_cell + sample_dist, y_cell + sample_dist));
                    std::vector<Anchor> current_anchors;
                    for (short y_cells = sample_cells.a.y; y_cells <= sample_cells.b.y; y_cells++) {
                        for (short x_cells = sample_cells.a.x; x_cells <= sample_cells.b.x; x_cells++) {
                            short x_cur = x_cells;
                            short x_offset = 0;
                            if (x_cur < 0) {
                                x_offset = -map_size.w;
                                x_cur += num_cells.w;
                            } else if (x_cur > num_cells.w - 1) {
                                x_offset = map_size.w;
                                x_cur -= num_cells.w;
                            }
                            short y_cur = y_cells;
                            short y_offset = 0;
                            if (y_cur < 0) {
                                y_offset = -map_size.h;
                                y_cur += num_cells.h;
                            } else if (y_cur > num_cells.h - 1) {
                                y_offset = map_size.h;
                                y_cur -= num_cells.h;
                            }
                            current_anchors.emplace_back(anchors[y_cur * num_cells.w + x_cur]);
                            current_anchors.back().pos.x += x_offset;
                            current_anchors.back().pos.y += y_offset;
                        }
                    }
                    for (short y_map = y_cell * cell_size.h; y_map < (short)((y_cell + 1) * cell_size.h); y_map++) {
                        for (short x_map = x_cell * cell_size.w; x_map < (short)((x_cell + 1) * cell_size.w); x_map++) {
                            unsigned long long sum_val = 0;
                            unsigned long long sum_temp = 0;
                            unsigned long long total_samples = 0;
                            for (int i = 0; i < (int)current_anchors.size(); i++) {
                                int diffx = current_anchors[i].pos.x - x_map;
                                int diffy = current_anchors[i].pos.y - y_map;
                                int dist = ((diffx ^ (diffx >> 31)) - (diffx >> 31)) + ((diffy ^ (diffy >> 31)) - (diffy >> 31));
                                int num_samples = max_samples - dist * dist;
                                num_samples = 1 + (num_samples & -((num_samples >> 31) ^ 1));
                                sum_val += num_samples * current_anchors[i].perc;
                                sum_temp += num_samples * current_anchors[i].temp;
                                total_samples += num_samples;
                            }
                            double total_val = (double)sum_val / (total_samples * Anchor::PERC_FACTOR);
                            double total_temp = (double)sum_temp / total_samples;

                            double current_val = 0;
                            for (Config::Elevation& elevation : config.elevations) {
                                current_val += elevation.perc;
                                if (total_val - current_val <= 0.001) {
                                    int biome_index = 0;
                                    if (elevation.biomes.size() > 1) {
                                        for (biome_index = 0; biome_index < (int)elevation.biomes.size()-1; biome_index++) {
                                            if (total_temp < elevation.temperatures[biome_index]) {
                                                break;
                                            }
                                        }
                                    }
                                    Config::Biome& biome = elevation.biomes[biome_index];
                                    if (biome.max_height > 0) {
                                        char height = max_height * (total_val - height_cutoff) / (1 - height_cutoff);
                                        heightmap->get(x_map, y_map) = height < 0 ? 0 : height;
                                    }
                                    map->set_ground(biome.id(), {x_map, y_map}, elevation.blocking);
                                    double val = random_fast();
                                    current_val = 0;
                                    for (auto& item : biome.items) {
                                        current_val += item.perc;
                                        if (val - current_val <= 0.01) {
                                            map->set_tile(item.id(), {x_map, y_map}, item.size() / tile_dim);
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            });

            for (short y_map = 1; y_map < map_size.h - wall_height; y_map++) {
                for (short x_map = 1; x_map < map_size.w - 1; x_map++) {
                    std::string postfix = "";
                    if (heightmap->get(x_map, y_map) && heightmap->get(x_map, y_map) > heightmap->get(x_map, y_map-1)) {
                        postfix += "top";
                    }
                    if (heightmap->get(x_map, y_map) > heightmap->get(x_map, y_map+1)) {
                        postfix += "bottom";
                        for (int i = 1; i <= wall_height; i++) { 
                            map->set_ground(mountain_biome.name_wall, {x_map+0, y_map+i}, true);
                        }
                    }
                    if (heightmap->get(x_map, y_map) > heightmap->get(x_map-1, y_map)) {
                        postfix += "left";
                    }
                    if (heightmap->get(x_map, y_map) > heightmap->get(x_map+1, y_map)) {
                        postfix += "right";
                    }
                    if (!postfix.empty()) {
                        std::string n = Engine.textures()->generate_name("border_alpha", {mountain_biome.name, postfix});
                        map->set_ground(n, {x_map, y_map}, false);
                    }
                }
            }

            post_process(config, map_size);
            delete heightmap;
        }
};

#endif
