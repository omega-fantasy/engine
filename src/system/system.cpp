#include "player.h"
#include "buildings.h"
#include "simulate.h"
#include "system.h"

GameSystem System;

void GameSystem::init() {
    if (m_player) {
        delete m_player;
    }
    m_player = new Player();
    if (m_buildings) {
        delete m_buildings;
    }
    m_buildings = new Buildings();
    if (m_simulate) {
        delete m_simulate;
    }
    m_simulate = new Simulate();
}
