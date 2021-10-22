#ifndef SIMULATE_H
#define SIMULATE_H

#include "buildings.h"
#include "engine/scene.h"
#include "engine/tilemap.h"
#include "engine/audio.h"
#include "engine/engine.h"
#include <queue>

class Simulate : public Scene {
    public:
    virtual ~Simulate() {}

    void run() {
        Engine.scenes()->play(this);
    }

    int date() {
        return cycles;
    }

    bool advance(int current_frame) {
        if (current_frame == 0) {
            auto town_table = Engine.db()->get_table<Buildings::Town>("towns");
            for (auto it = town_table->begin(); it != town_table->end(); ++it) {
                Point pos(it.key());
                towns.push(pos);
            }
        }
        if (wait_frames > 0) {
            wait_frames--;
            return false;
        }
        if (current_town.x < 0 || current_town.y < 0) {
            if (towns.empty()) {
                cycles++;
                return true;
            }
            current_town = towns.front();
            towns.pop();
        }
        if (approach(current_town, 10)) {
            Engine.audio()->play_sound("menu2");
            for (auto& building : Engine.db()->get_table<Buildings::Town>("towns")->get(current_town).buildings) {
                if (building.x < 0 || building.y < 0) {
                    break;
                }
                System.player()->change_cash(100);
            }
            current_town = {-1, -1};
            wait_frames = 60 * 3;
        }
        return false;
    }

    private:

    bool approach(Point target, int accel) {
        auto map = Engine.map();
        BigPoint camera_target = {
            target.x * map->tile_size().w * map->camera_zoom() - 0.5 * map->get_size().w, 
            target.y * map->tile_size().h * map->camera_zoom() - 0.5 * map->get_size().h
        };
        BigPoint camera_current = map->camera_position();
        BigPoint camera_diff = camera_target - camera_current; 
        Point movement;
        movement.x = accel < std::abs(camera_diff.x) ? accel : std::abs(camera_diff.x);
        movement.x = camera_diff.x >= 0 ? movement.x : -movement.x;
        movement.y = accel < std::abs(camera_diff.y) ? accel : std::abs(camera_diff.y);
        movement.y = camera_diff.y >= 0 ? movement.y : -movement.y;
        map->move_cam(movement);
        return camera_current == map->camera_position();
    }

    Point current_town = {-1, -1};
    int wait_frames = 0;
    int cycles = 0;
    std::queue<Point> towns;
};

#endif
