#ifndef MINIMAP_H
#define MINIMAP_H

#include "engine/engine.h"
#include "engine/ui.h"
#include "engine/input.h"
#include "engine/tilemap.h"
#include "system/buildings.h"

class MiniMap : public Composite, Input::Listener, Tilemap::Listener {
    public:
        MiniMap(Size sz): Composite(sz) {
            m_texture = new Texture((unsigned)0x00000000, sz);
        }

        void create() {
            auto map = Engine.map();
            Size map_size = map->tilemap_size();
            Color* pixels = m_texture->pixels();
            for (short y = 0; y < size.h; y++) { 
                for (short x = 0; x < size.w; x++) {
                    Point current_pos(x * ((double)map_size.w / size.w), y * ((double)map_size.h / size.h));
                    Texture* t = Engine.textures()->get(map->get_ground(current_pos));
                    int texture_center = 0.5 * t->size().h * t->size().w + t->size().w;
                    pixels[y * m_texture->size().w + x] = t->pixels()[texture_center];
                }
            }
        }

        void map_changed() { 
            recreate = true; 
        }
        
        void draw() {
            if (recreate) {
                create();
                recreate = false;
            }
            if (!listener_registered) {
                Engine.input()->add_mouse_listener(this, {pos, size});
                Engine.map()->add_listener(this);
                listener_registered = true;
            }
            if (needs_update()) {
                Engine.screen()->blit(m_texture->pixels(), m_texture->size(), pos, Box(pos, size), false);
                Size tiles_per_pixel = Engine.map()->tilemap_size() / size;
                Box corners = Engine.map()->visible_tiles();
                WrappingPoint tile_center(corners.center().x, corners.center().y, Engine.map()->tilemap_size());
                Point mini_cam_pos = pos + Point(tile_center.x / tiles_per_pixel.w, tile_center.y / tiles_per_pixel.h);
                Engine.screen()->blit(texture_cam->pixels(), texture_cam->size(), mini_cam_pos, Box(pos, size), true);
                /* disabled drawing box on mini-map due to complex edge-wrapping
                Point p1 = pos + Point(corners.a.x / tiles_per_pixel.w, corners.a.y / tiles_per_pixel.h);
                Point p2 = pos + Point(corners.b.x / tiles_per_pixel.w, corners.b.y / tiles_per_pixel.h);
                float zoom = Engine.map()->camera_zoom();
                if (red_boxes.find(zoom) == red_boxes.end()) {
                    create_box(p1, p2, zoom);
                }
                Engine.screen()->blit(red_boxes[zoom]->pixels(), red_boxes[zoom]->size(), p1, Box(pos, size), true);
                */
                auto table = Engine.db()->get_table<Buildings::Town>("towns");
                for (auto it = table->begin(); it != table->end(); ++it) {
                    Point p_town(it.key());
                    Point p_dot = pos + Point(p_town.x / tiles_per_pixel.w, p_town.y / tiles_per_pixel.h);
                    Engine.screen()->blit(texture_city->pixels(), texture_city->size(), p_dot, Box(pos, size), false);
                }
                set_update(false);
            }
        }
    
        void create_box(Point p1, Point p2, float zoom) {
            Size s(p2.x - p1.x, p2.y - p1 .y);
            if (s.w < 3 || s.h < 3) {
                s = {3, 3};
            }
            Texture* t = new Texture((unsigned)0x00000000, {s.w, s.h});
            Color* pixels = t->pixels();
            for (short y = 0; y < s.h; y++) {
                for (short x = 0; x < s.w; x++) {
                    if (x == 0 || y == 0 || y == s.h - 1 || x == s.w - 1) {
                        pixels[y * s.w + x] = Color(255, 0, 0);
                    }
                }
            }
            t->set_transparent(true);
            red_boxes[zoom] = t;
        }
    
        void mouse_clicked(Point p) {
            auto map = Engine.map();
            double x_perc = ((double)(p.x - pos.x) / size.w);
            double y_perc = ((double)(p.y - pos.y) / size.h);
            map->move_cam_to_tile({x_perc * map->tilemap_size().w, y_perc * map->tilemap_size().h});
        }

    private:
        std::map<float, Texture*> red_boxes;
        Texture* texture_city = new Texture(Color(255, 0, 0), {5, 5});
        Texture* texture_cam = new Texture(Color(255, 255, 255), {5, 5});
        bool listener_registered = false;
        bool recreate = true;
};

#endif
