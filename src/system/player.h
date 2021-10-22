#ifndef PLAYER_H
#define PLAYER_H

#include "engine/db.h"
#include "engine/config.h"
#include "engine/engine.h"

class Player {
    public:
    Player() {
        auto table = Engine.db()->get_table<int>("cash");
        int startmoney = std::stoi(Engine.config()->get("buildings")["startmoney"]);
        if (table->exists(0)) {
            table->get(0) = startmoney;
        } else {
            table->add(0, startmoney);
        }
    }

    bool change_cash(int amount) {
        auto& current = Engine.db()->get_table<int>("cash")->get(0);
        if (current + amount < 0) {
            return false;
        }
        current += amount;
        return true;
    }

    int current_cash() {
        return Engine.db()->get_table<int>("cash")->get(0);
    }
};

#endif
