#include "player.h"
#include "buildings.h"
#include "simulate.h"
#include "research.h"
#include "system.h"

GameSystem System;

void GameSystem::init_script_api() {
    Engine.register_script_function({"towns", ScriptType::LIST, {}, [&](const std::vector<ScriptParam>&) {
        std::vector<ScriptParam> ret;
        for (auto& t : System.buildings()->townlist()) { ret.emplace_back(t.second); }
        return ret;
    }});
    Engine.register_script_function({"buildings", ScriptType::LIST, {ScriptType::NUMBER}, [&](const std::vector<ScriptParam>& params) {
        std::vector<ScriptParam> ret;
        for (auto& b : System.buildings()->buildinglist(params[0].d())) { ret.emplace_back(b); }
        return ret;
    }});
    Engine.register_script_function({"property_exists", ScriptType::NUMBER, {ScriptType::NUMBER, ScriptType::STRING}, [&](const std::vector<ScriptParam>& params) {
        return System.buildings()->has_property(int(params[0].d()), params[1].s()) ? 1.0 : 0.0;
    }});
    Engine.register_script_function({"property_get", ScriptType::NUMBER, {ScriptType::NUMBER, ScriptType::STRING}, [&](const std::vector<ScriptParam>& params) {
        return System.buildings()->get_property(int(params[0].d()), params[1].s());
    }});
    Engine.register_script_function({"player_money_change", ScriptType::NUMBER, {ScriptType::NUMBER}, [&](const std::vector<ScriptParam>& params) {
        System.player()->change_cash(params[0].d()); return 0;
    }});  

}

void GameSystem::init() {
    delete m_player;
    m_player = new Player();
    delete m_buildings;
    m_buildings = new Buildings();
    delete m_research;
    m_research = new Research();
    init_script_api();
    /*
    if (m_simulate) {
        delete m_simulate;
    }
    m_simulate = new Simulate();
    */
}
