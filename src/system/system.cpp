#include "player.h"
#include "buildings.h"
#include "simulate.h"
#include "research.h"
#include "system.h"

GameSystem System;

void GameSystem::init() {
    delete m_player;
    m_player = new Player();
    delete m_buildings;
    m_buildings = new Buildings();
    delete m_research;
    m_research = new Research();
    /*
    if (m_simulate) {
        delete m_simulate;
    }
    m_simulate = new Simulate();
    */
}
