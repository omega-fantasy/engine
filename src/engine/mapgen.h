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
                Texture::ID m_id = -1;
                Size m_size = {0, 0};
            };

            struct Biome {
                Biome(const std::string& n, double p, bool b, const std::vector<Item>& i) : name(n), perc(p), blocking(b), items(i) {}
                std::string name;
                double perc;
                bool blocking;
                short max_height = 0;
                short wall_height = 0;
                std::string name_wall = "";
                bool operator==(const Biome& b) { return b.m_id == m_id; }
                std::vector<Item> items;
                Texture::ID id() {
                    if (m_id < 0) {
                        m_id = Engine.textures()->get(name)->id();
                    }
                    return m_id;
                }
                Texture::ID m_id = -1;
            };

            Config() {
                auto& configfile = Engine.config()->get("mapgen");
                std::string lastname = (*(configfile["biomes"].end()-1))["name"];
                for (auto& biome : configfile["biomes"]) {
                    std::vector<Config::Item> items;
                    for (auto& item : biome["vegetation"]) {
                        items.emplace_back(item["name"], std::stod(item["quantity"]));
                    }
                    Biome new_biome(biome["name"], std::stod(biome["quantity"]), (bool)std::stoi(biome["blocking"]), items);
                    if (new_biome.name == lastname) {
                        new_biome.max_height = std::stoi(biome["max_height"]);
                        new_biome.wall_height = std::stoi(biome["wall_height"]);
                        new_biome.name_wall = biome["name_wall"];
                    }
                    biomes.push_back(new_biome);
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

        static void randomize_map(Matrix<Texture::ID>* tiles_ground, Matrix<Texture::ID>* tiles_above) {
            auto map = Engine.map();
            Size map_size = {tiles_ground->width(), tiles_ground->height()};
            Size tile_dim = map->tile_size();

            Config config;
            Size num_cells = {config.num_cells, config.num_cells};
            Size cell_size = map_size / num_cells;
            int max_samples = cell_size.w * cell_size.h * config.sample_factor; // 3
            int sample_dist = config.sample_distance; // 2
           
            Config::Biome& mountain_biome = config.biomes.back();
            Matrix<unsigned char>* heightmap = Engine.db()->get_matrix<unsigned char>("heightmap", map_size.w, map_size.h);
            unsigned char max_height = mountain_biome.max_height - 1;
            double height_cutoff = 1 - mountain_biome.perc;

            std::vector<std::pair<Point, double>> anchors;
            for (short y = 0; y < num_cells.h; y++) {
                for (short x = 0; x < num_cells.w; x++) {
                    anchors.push_back({{(x + random_uniform()) * cell_size.w, (y + random_uniform()) * cell_size.h}, random_uniform()}); 
                }
            }
            for (short y_map = 0; y_map < map_size.h; y_map++) {
                for (short x_map = 0; x_map < map_size.w; x_map++) {
                    tiles_ground->get(x_map, y_map) = 0;
                    tiles_above->get(x_map, y_map) = 0;
                    heightmap->get(x_map, y_map) = 0;
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
                            for (Config::Biome& biome : config.biomes) {
                                current_val += biome.perc;
                                if (total_val - current_val <= 0.00001) {
                                    if (biome == mountain_biome) {
                                        char height = 1 + max_height * (total_val - height_cutoff) / (1 - height_cutoff);
                                        heightmap->get(x_map, y_map) = height < 0 ? 0 : height;
                                    }
                                    tiles_ground->get(x_map, y_map) = biome.blocking ? -biome.id() : biome.id();
                                    double val = random_uniform();
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
       
            short wall_height = mountain_biome.wall_height;
            for (short y_map = 1; y_map < map_size.h - wall_height; y_map++) {
                for (short x_map = 1; x_map < map_size.w - 1; x_map++) {
                    std::string postfix = "";
                    if (heightmap->get(x_map, y_map) && heightmap->get(x_map, y_map) > heightmap->get(x_map, y_map-1)) {
                        postfix += "top";
                    }
                    if (heightmap->get(x_map, y_map) > heightmap->get(x_map, y_map+1)) {
                        postfix += "bottom";
                        for (int i = 1; i <= wall_height; i++) { 
                            map->set_ground(mountain_biome.name_wall+"__left_right", {x_map+0, y_map+i}, true);
                        }
                    }
                    if (heightmap->get(x_map, y_map) > heightmap->get(x_map-1, y_map)) {
                        postfix += "left";
                    }
                    if (heightmap->get(x_map, y_map) > heightmap->get(x_map+1, y_map)) {
                        postfix += "right";
                    }
                    if (!postfix.empty()) {
                        map->set_ground(mountain_biome.name + "__" + postfix, {x_map, y_map}, false);
                    } 
                }
            }
        }
};

#endif
