#ifndef MAPGEN_H
#define MAPGEN_H

#include "engine.h"
#include "db.h"
#include "config.h"

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
                auto& configfile = Engine.config()->get("mapgen");
                for (auto& elevation : configfile["elevations"]) {
                    elevations.emplace_back(std::stod(elevation["quantity"]), (bool)std::stoi(elevation["blocking"]), (bool)std::stoi(elevation["blend"]));
                    auto& new_elevation = elevations.back();
                    for (auto& biome : elevation["biomes"]) {
                        new_elevation.biomes.emplace_back(biome["name"]);
                        new_elevation.temperatures.push_back(std::stoi(biome["temperature_max"]));
                        auto& new_biome = new_elevation.biomes.back();
                        if (biome.contains("name_wall")) {
                            new_biome.name_wall = biome["name_wall"];
                            new_biome.max_height = std::stoi(biome["max_height"]);
                            new_biome.wall_height = std::stoi(biome["wall_height"]);
                        }
                        for (auto& item : biome["vegetation"]) {
                            new_biome.items.emplace_back(item["name"], std::stod(item["quantity"]));
                        }
                    }
                }
                num_cells = std::stoi(configfile["num_cells"]);
                sample_factor = std::stoi(configfile["sample_factor"]);
                sample_distance = std::stoi(configfile["sample_distance"]);
            }
            std::vector<Elevation> elevations;
            short num_cells = 0;
            short sample_factor = 0;
            short sample_distance = 0;
        };

        static void post_process(Config& config, Size map_size) {
            auto map = Engine.map();
            std::map<Texture::ID, std::set<Texture::ID>> blend_map; 
            std::map<Texture::ID, std::string> name_map; 
            std::vector<Texture::ID> prev_ids;
            for (Config::Elevation& elevation : config.elevations) {
                std::vector<Texture::ID> prev_ids_temp;
                for (auto& biome : elevation.biomes) {
                    if (elevation.blend) {
                        for (auto prev_id : prev_ids) {
                            blend_map[biome.id()].insert(prev_id);
                        }
                        for (auto& biome2 : elevation.biomes) {
                            if (biome2.id() != biome.id()) {
                                blend_map[biome.id()].insert(biome2.id());
                            }
                        }
                    }
                    name_map[biome.id()] = biome.name;
                    prev_ids_temp.push_back(biome.id());
                }
                prev_ids = prev_ids_temp;
            }
            for (short y_map = 0; y_map < map_size.h; y_map++) {
                for (short x_map = 0; x_map < map_size.w; x_map++) {
                    Texture::ID current_id = map->get_ground({x_map, y_map});
                    if (blend_map.find(current_id) != blend_map.end()) {
                        std::set<Texture::ID>& blend_set = blend_map[current_id];
                        std::vector<std::string> params(5);
                        bool blend = false;
                        if (y_map > 0 && blend_set.find(map->get_ground({x_map+0, y_map-1})) != blend_set.end()) {
                            params[1] = name_map[map->get_ground({x_map+0, y_map-1})]; blend = true;
                        }
                        if (x_map < map_size.w-1 && blend_set.find(map->get_ground({x_map+1, y_map+0})) != blend_set.end()) {
                            params[2] = name_map[map->get_ground({x_map+1, y_map-0})]; blend = true;
                        }
                        if (y_map < map_size.h-1 && blend_set.find(map->get_ground({x_map+0, y_map+1})) != blend_set.end()) {
                            params[3] = name_map[map->get_ground({x_map+0, y_map+1})]; blend = true;
                        }
                        if (x_map > 0 && blend_set.find(map->get_ground({x_map-1, y_map+0})) != blend_set.end()) {
                            params[4] = name_map[map->get_ground({x_map-1, y_map+0})]; blend = true;
                        }
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
            Anchor(Point p, double pc, char t): pos(p), perc(pc), temp(t) {}
            Point pos;
            double perc;
            char temp;
        };

        static void randomize_map(Matrix<Texture::ID>* tiles_ground, Matrix<Texture::ID>* tiles_above) {
            auto map = Engine.map();
            Size map_size = {tiles_ground->width(), tiles_ground->height()};
            Size tile_dim = map->tile_size();

            Config config;
            Size num_cells = {config.num_cells, config.num_cells};
            Size cell_size = map_size / num_cells;
            int max_samples = cell_size.w * cell_size.h * config.sample_factor; // 3
            int sample_dist = config.sample_distance; // 2
            
            Matrix<unsigned char>* heightmap = Engine.db()->get_matrix<unsigned char>("heightmap", map_size.w, map_size.h);
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
            
            for (short y_map = 0; y_map < map_size.h; y_map++) {
                for (short x_map = 0; x_map < map_size.w; x_map++) {
                    tiles_ground->get(x_map, y_map) = 0;
                    tiles_above->get(x_map, y_map) = 0;
                    heightmap->get(x_map, y_map) = 0;
                }
            }

            std::vector<Anchor> current_anchors;
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
                            double total_temp = 0;
                            int total_samples = 0;
                            for (auto& anchor : current_anchors) {
                                short dist = anchor.pos.distance({x_map, y_map}); 
                                int num_samples = max_samples - dist * dist;
                                if (num_samples < 0) {
                                    num_samples = 1;
                                }
                                total_val += num_samples * anchor.perc;
                                total_temp += num_samples * anchor.temp;
                                total_samples += num_samples;
                            }
                            total_val /= total_samples;
                            total_temp /= total_samples;

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
                                    tiles_ground->get(x_map, y_map) = elevation.blocking ? -biome.id() : biome.id();
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
            }

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
        }
};

#endif
