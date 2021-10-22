#ifndef SYSTEM_H
#define SYSTEM_H

class Buildings;
class Player;
class Simulate;

class GameSystem {
    public:
        void init();

        Player* player() { return m_player; }
        Buildings* buildings() { return m_buildings; }
        Simulate* simulate() { return m_simulate; }

    private:
        Player* m_player = nullptr;
        Buildings* m_buildings = nullptr;
        Simulate* m_simulate = nullptr;
};

extern GameSystem System;

#endif
