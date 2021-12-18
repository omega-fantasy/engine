#ifndef TILEMAP_H
#define TILEMAP_H

#include "engine.h"
#include "screen.h"
#include "input.h"
#include "db.h"

class Tilemap : public Composite, Input::Listener {
    public:
        class Listener {
            public:
                virtual void tile_clicked(Point) {}
                virtual void map_changed() {}
        };
        
        Tilemap(Size screen_size): Composite(screen_size) {}
        void create_map(Size screen_size);

        bool set_ground(const std::string& texture_name, Point p, bool blocked);
        bool set_tile(const std::string& texture_name, Point p); 
        bool set_tile(Texture::ID id, Point p, Size s);
        void unset_tile(Point pos);

        Point texture_root(Point p); 
        Box visible_tiles();
        void randomize_map();

        void move_cam(Point p);
        void move_cam_to_tile(Point tile_pos);
        void zoom_cam(int factor);
        void set_cursor_texture(const std::string& name) { cursor_texture = name; }

        Size tilemap_size() { return map_size; }
        Size tile_size() { return tile_dim; }
        float camera_zoom() { return zoom; }
        void set_zoom(float z) { zoom = z; }
        BigPoint camera_position() { return camera_pos; }
        void add_listener(Tilemap::Listener* l) { click_listeners.push_back(l); }
        void remove_listener(Tilemap::Listener* l) { click_listeners.erase(std::find(click_listeners.begin(), click_listeners.end(), l)); } 
        Texture::ID get_ground(Point p) { Texture::ID id = tiles_ground->get(p.x, p.y); return id < 0 ? -id : id; }
    
    private:
        std::vector<Tilemap::Listener*> click_listeners;
        constexpr static double MAX_ZOOM = 4.0;
        constexpr static double MIN_ZOOM = 0.125;
        bool listener_registered = false;
        Matrix<Texture::ID>* tiles_ground = nullptr;
        Matrix<Texture::ID>* tiles_above = nullptr;
        Size tile_dim = {0, 0};
        Size map_size = {0, 0};
        BigPoint camera_pos = {0, 0};
        float zoom = 1;
        std::string cursor_texture;
        Point last_mouse_pos = {0, 0};
        bool infinite_scrolling = false;

        void mouse_clicked(Point p);
        void fix_camera();
        void draw();
};

#endif
