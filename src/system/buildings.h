#ifndef BUILDINGS_H
#define BUILDINGS_H

#include "engine/db.h"
#include "engine/tilemap.h"
#include "engine/config.h"
#include "engine/engine.h"
#include "system/player.h"
#include "system/system.h"

class Buildings {
    public:
        constexpr static int MAX_BUILDINGS_PER_TOWN = 64;
        
        struct Type {
            Type(): name(""), price(0) {}
            Type(const std::string& n, int p): name(n), price(p) {}
            std::string name;
            int price;
            std::map<std::string, double> properties;
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
            for (auto& p : Engine.config("buildings")["buildings"]) {
                auto& t = p.second;
                m_types[t["name"].s()] = Type(t["name"].s(), t["price"].i());
                for (auto& p : t["properties"]) {
                    m_types[t["name"].s()].properties[p.first.s()] = p.second.d(); 
                }
            }
            max_town_distance = Engine.config("buildings")["max_town_distance"].i();
        }

        virtual ~Buildings() {}

        std::vector<Buildings::Type> types() {
            std::vector<Buildings::Type> ret;
            for (auto& t : m_types) {
                ret.push_back(t.second);
            }
            return ret;
        }

        bool has_property(Point building, const std::string& property) {
            return Engine.db()->get_table<double>(property)->exists(building);
        }

        double get_property(Point building, const std::string& property) {
            if (has_property(building, property)) {
                return Engine.db()->get_table<double>(property)->get(building);
            }
            return 0.0;
        }

        void set_property(Point building, const std::string& property, double value) {
            if (has_property(building, property)) {
                Engine.db()->get_table<double>(property)->add(building);
            }
            Engine.db()->get_table<double>(property)->get(building) = value;
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

        std::vector<std::pair<std::string, Point>> townlist() {
            std::vector<std::pair<std::string, Point>> ret;
            auto table = Engine.db()->get_table<Town>("towns");
            for (auto it = table->begin(); it != table->end(); ++it) {
                ret.push_back({(*it).name.toStdString(), it.key()});
            }
            return ret;
        }

        std::vector<Point> buildinglist(Point town) {
            std::vector<Point> ret;
            auto& t = Engine.db()->get_table<Town>("towns")->get(town);
            for (auto b : t.buildings) {
                if (b.x < 0 || b.y < 0) break;
                ret.push_back(b);
            }
            return ret;
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
                for (auto& param : m_types[name].properties) {
                    Engine.db()->get_table<double>(param.first)->add(p) = param.second;
                }
                return true;
            }
            System.player()->change_cash(price);
            return false;
        }

    private:
        std::map<std::string, Buildings::Type> m_types;
        int max_town_distance;
};

#endif
