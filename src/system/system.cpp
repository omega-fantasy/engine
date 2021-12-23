#include "player.h"
#include "buildings.h"
#include "simulate.h"
#include "research.h"
#include "system.h"
#include "engine/scripting.h"

GameSystem System;

void GameSystem::init_script_api() {
    auto script = Engine.script();
    script->add_function("towns", {}, [&](Script::ParamList&){
        std::vector<double>* ret = new std::vector<double>();
        for (auto& t : System.buildings()->townlist()) { ret->push_back(t.second); }
        return ret;
    });
    script->add_function("buildings", {Script::Type::Number}, [&](Script::ParamList& params){
        std::vector<double>* ret = new std::vector<double>();
        for (auto& b : System.buildings()->buildinglist(Script::d(params[0]))) { ret->push_back(b); }
        return ret;
    });
    script->add_function("has_property", {Script::Type::Number, Script::Type::String}, [&](Script::ParamList& params){
        return System.buildings()->has_property(int(Script::d(params[0])), Script::s(params[1])) ? 1.0 : 0.0;
    });
    script->add_function("property_value", {Script::Type::Number, Script::Type::String}, [&](Script::ParamList& params){
        return System.buildings()->property_value(int(Script::d(params[0])), Script::s(params[1]));
    });
    script->add_function("change_player_money", {Script::Type::Number}, [&](Script::ParamList& params){
        System.player()->change_cash(Script::d(params[0])); return 0.0;
    });  
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
