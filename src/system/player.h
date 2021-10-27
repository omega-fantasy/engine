#ifndef PLAYER_H
#define PLAYER_H

#include "engine/db.h"
#include "engine/config.h"
#include "engine/engine.h"

class Player {
    struct Entity {
        String<16> worldname = "";
        int cash = 0;
    };

    public:
    Player() {
        auto table = Engine.db()->get_table<Entity>("player");
        int startmoney = std::stoi(Engine.config()->get("buildings")["startmoney"]);
        if (!table->exists(0)) {
            table->add(0);
        }
        table->get(0).cash = startmoney;
    }

    void set_worldname(const std::string& name) {
        Engine.db()->get_table<Entity>("player")->get(0).worldname = name;
    }

    bool change_cash(int amount) {
        auto& player = Engine.db()->get_table<Entity>("player")->get(0);
        if (player.cash + amount < 0) {
            return false;
        }
        player.cash += amount;
        return true;
    }

    int current_cash() {
        return Engine.db()->get_table<Entity>("player")->get(0).cash;
    }
};

#endif
