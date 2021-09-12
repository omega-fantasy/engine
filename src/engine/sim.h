#ifndef SIM_H
#define SIM_H

#include "engine.h"
#include "db.h"

#include <map>
#include <vector>

class Simulation {
    public:
    class Event {
        public:
            virtual void execute() = 0;
    };

    Simulation() {
        reset();
    }
   
    void step(int t) {
        if (!run) {
            return;
        }
        executing = true;
        int target = simtime() + t;
        std::vector<int> delete_slices;
        Table<Slice>* table = Engine.db()->get_table<Slice>("events");
        for (auto it = table->begin(); it != table->end(); ++it) {
            if (it.key() > target) {
                break;
            }
            for (int id : (*it).events) {
                if (!id) break;
                events[id]->execute();
            }
            delete_slices.push_back(it.key());
        }
        for (int slice : delete_slices) {
            table->erase(slice);
        }
        Engine.db()->get_table<int>("simtime")->get(0) = target;
        executing = false;
        for (auto& e : queue) {
            queue_event(e.first, e.second);
        }
        queue.clear();
    }
    
    
    void register_event(const std::string& name, Event* event) {
        events[hash(name.c_str())] = event;
    }

    void queue_event(const std::string& name, int time_from_now) {
        if (executing) {
            queue.push_back({name, time_from_now});
            return;
        }
        int t = simtime() + time_from_now; 
        int id = hash(name.c_str());
        Table<Slice>* table = Engine.db()->get_table<Slice>("events");
        if (!table->exists(t)) {
            table->add(t);
        }
        table->get(t).add(id);
    }
    
    int simtime() { return Engine.db()->get_table<int>("simtime")->get(0); }
    
    void toggle(bool running) { run = running; }
    bool running() { return run; }

    void reset() {
        Table<int>* table = Engine.db()->get_table<int>("simtime");
        if (!table->exists(0)) {
            table->add(0);
        }
        table->get(0) = 0;
    }

   private:
    std::map<int, Event*> events;
    std::vector<std::pair<std::string, int>> queue;
    bool run = true;
    bool executing = false;

    struct Slice {
        constexpr static int DEPTH = 1024;
        void add(int id) { 
            for (int i = 0; i < DEPTH; i++) {
                if (events[i] == 0) {
                    events[i] = id;
                    return;
                }
            }
        }
        int events[DEPTH] = {0};
    };

    int hash(const char *str) {
        int h = 0;
        while (*str) {
            h = h << 1 ^ *str++;
        }
        return h;
    }
};

#endif
