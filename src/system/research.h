#ifndef RESEARCH_H
#define RESEARCH_H

#include "engine/db.h"
#include "engine/config.h"
#include "engine/engine.h"

class Research {
    public:
    using Info = std::tuple<std::string, std::string, int>;

    Research() {
        int i = 0;
        auto table = Engine.db()->get_table<Entity>("research");
        if (!table->exists(i)) {
            for (auto& item : Engine.config()->get("research")["items"]) {
                auto& new_research = table->add(i++);
                new_research.name = item["name"];
                new_research.description = item["description"];
                new_research.cost = std::stoi(item["cost"]);
                new_research.max_progress = std::stoi(item["max_progress"]);
            }
        }
    }

    std::vector<Research::Info> itemlist() {
        std::vector<Research::Info> ret;
        auto table = Engine.db()->get_table<Entity>("research");
        for (auto& item : *table) {
            int prog =  100 * (double)item.current_progress / item.max_progress;
            ret.emplace_back(item.name.toStdString(), item.description.toStdString(), prog);
        }
        return ret;
    }

    // returns total progress percentage (same as before on failure)
    int progress(const std::string& research_name) {
        auto table = Engine.db()->get_table<Entity>("research");
        for (auto& item : *table) {
            if (item.name.toStdString() == research_name) {
                if (item.current_progress < item.max_progress && System.player()->change_cash(item.cost)) {
                    item.current_progress++;
                }
                return 100 * (double)item.current_progress / item.max_progress;
            }
        }
        return 0;
    }

    private:
    struct Entity {
        String<32> name = "";
        String<1024> description = "";
        int cost;
        int max_progress;
        int current_progress = 0;
    };
};

#endif
