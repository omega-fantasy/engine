#ifndef BUILDINGS_H
#define BUILDINGS_H

#include "engine/db.h"
#include "engine/tilemap.h"
#include "engine/config.h"
#include "engine/engine.h"
#include "system/player.h"
#include "system/system.h"
#include <map>
#include <string>

class Buildings {
    public:
        constexpr static int MAX_BUILDINGS_PER_TOWN = 64;
        
        struct Type {
            Type() {}
            Type(const std::string& n, int p): name(n), price(p) {}
            std::string name;
            int price;
        };

        struct Building {
        };

        struct Town {
            Town() {
                for (auto& b : buildings) {
                    b = {-1, -1};
                }
            }
            void add_building(Point p) {
                for (auto& b : buildings) {
                    if (b.x < 0 || b.y < 0) {
                        b = p;
                        return;
                    }
                }
            }
            Point buildings[MAX_BUILDINGS_PER_TOWN];
            String<16> name = "";
        };
    
        Buildings() {
            for (auto& t : Engine.config()->get("buildings")["buildings"]) {
                m_types[t["name"]] = Type(t["name"], std::stoi(t["price"]));
            }
            max_town_distance = std::stoi(Engine.config()->get("buildings")["max_town_distance"]);
        }

        virtual ~Buildings() {}

        std::vector<Buildings::Type> types() {
            std::vector<Buildings::Type> ret;
            for (auto& t : m_types) {
                ret.push_back(t.second);
            }
            return ret;
        }

        void destroy(Point p) {
            p = Engine.map()->texture_root(p);
            auto table = Engine.db()->get_table<Town>("towns");
            if (table->exists(p)) {
                Town& town = table->get(p);
                for (auto& b : town.buildings) {
                    if (b.x < 0 || b.y < 0 || b == p) break;
                    destroy(b);
                }
                table->erase(p);
            }
            Engine.map()->unset_tile(p);
        }

        void set_townname(Point p, const std::string& name) {
            auto table = Engine.db()->get_table<Town>("towns");
            if (table->exists(p)) {
                table->get(p).name = name;
            }
        }

        bool create_town(Point p) {
            auto table = Engine.db()->get_table<Town>("towns");
            for (auto it = table->begin(); it != table->end(); ++it) {
                Point pos(it.key());
                if (pos.distance(p) < 2 * max_town_distance) {
                    return false;
                }
            }
            table->add(p);
            return true;
        }

        bool create(const std::string& name, Point p) {
            auto table = Engine.db()->get_table<Town>("towns");
            bool town_found = false;
            for (auto it = table->begin(); it != table->end(); ++it) {
                Point pos(it.key());
                if (pos.distance(p) < max_town_distance) {
                    (*it).add_building(p);
                    town_found = true;
                    break;
                }
            }
            if (!town_found) {
                return false;
            }

            int price = m_types[name].price;
            if (!System.player()->change_cash(-price)) {
                System.player()->change_cash(price); // refund
                return false;
            }

            if (Engine.map()->set_tile(name, p)) {
                Engine.db()->get_table<Building>("buildings")->add(p);
                return true;
            }
            return false;
        }

    private:
        std::map<std::string, Buildings::Type> m_types;
        int max_town_distance;
};

#endif
