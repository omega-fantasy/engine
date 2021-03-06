#ifndef SYSTEM_H
#define SYSTEM_H

class Buildings;
class Player;
class Simulate;
class Research;

class GameSystem {
    public:
        void init();

        Player* player() { return m_player; }
        Buildings* buildings() { return m_buildings; }
        Research* research() { return m_research; }
        Simulate* simulate() { return m_simulate; }

    private:
        void init_script_api();
        Player* m_player = nullptr;
        Buildings* m_buildings = nullptr;
        Research* m_research = nullptr;
        Simulate* m_simulate = nullptr;
};

extern GameSystem System;

#endif
