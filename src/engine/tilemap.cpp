#include "tilemap.h"
#include "config.h"
#include "mapgen.h"

void Tilemap::create_map(Size screen_size) {
    map_size = {std::stoi(Engine.config()->get("settings")["mapsize"]["width"]), std::stoi(Engine.config()->get("settings")["mapsize"]["height"])};
    tile_dim = {std::stoi(Engine.config()->get("settings")["tilesize"]["width"]), std::stoi(Engine.config()->get("settings")["tilesize"]["height"])};
    size = screen_size;
    tiles_ground = Engine.db()->get_matrix<Texture::ID>("tiles", map_size.w, map_size.h);
    tiles_above = Engine.db()->get_matrix<Texture::ID>("buildings", map_size.w, map_size.h);
    for (auto& listener : click_listeners) {
        listener->map_changed();
    }
}

bool Tilemap::set_ground(const std::string& texture_name, Point p, bool blocked) {
    if (tiles_ground->get(p.x, p.y) < 0) {
        return false;
    }
    Texture* texture = Engine.textures()->get(texture_name);
    tiles_ground->get(p.x, p.y) = blocked ? -texture->id() : texture->id();
    return true;
}

bool Tilemap::set_tile(const std::string& texture_name, Point p) {
    Texture* texture = Engine.textures()->get(texture_name);
    Size s = texture->size() / tile_dim;  
    return set_tile(texture->id(), p, s);
}

bool Tilemap::set_tile(Texture::ID id, Point p, Size s) {
    if (p.x + s.w >= tiles_ground->width() || p.y + s.h >= tiles_ground->height()) {
        return false;
    }
    for (short y = p.y; y < p.y + s.h; y++) {
        for (short x = p.x; x < p.x + s.w; x++) {
            if (tiles_ground->get(x, y) < 0 || tiles_above->get(x, y) != 0) {
                return false;
            }
        }
    }
    for (short y = p.y; y < p.y + s.h; y++) {
        for (short x = p.x; x < p.x + s.w; x++) {
            if (x == p.x && y == p.y) {
                tiles_above->get(p.x, p.y) = id;
            } else {
                tiles_above->get(x, y) = -id;
            }
        }
    }
    return true;
}

Point Tilemap::texture_root(Point p) {
    Texture::ID id = tiles_above->get(p.x, p.y);
    if (id < 0) {
        Size s = Engine.textures()->get(-id)->size() / tile_dim;
        for (short y = p.y; y > p.y - s.h && y >= 0; y--) { 
            for (short x = p.x; x > p.x - s.w && x >= 0 ; x--) {
                Texture::ID current_id = tiles_above->get(x, y);
                if (current_id == -id) {
                    return {x, y};
                }
            }
        }
    }
    return p;
}

void Tilemap::unset_tile(Point ps) {
    Point p = texture_root(ps);
    Texture::ID id = tiles_above->get(p.x, p.y);
    if (id <= 0) {
        return;
    }
    Size s = Engine.textures()->get(id)->size() / tile_dim;
    for (short y = p.y; y < p.y + s.h; y++) { 
        for (short x = p.x; x < p.x + s.w; x++) {
            if ((x == p.x && y == p.y) || tiles_above->get(x, y) < 0) {
                tiles_above->get(x, y) = 0;
            }
        }
    }
}

void Tilemap::move_cam(Point p) {
    camera_pos = camera_pos + p;
    if ((camera_pos.x + size.w) / (zoom * tile_dim.w) > map_size.w) {
        camera_pos.x = (map_size.w * tile_dim.w * zoom - size.w);
    }
    if ((camera_pos.y + size.h) / (zoom * tile_dim.h) > map_size.h) {
        camera_pos.y = (map_size.h * tile_dim.h * zoom - size.h);
    }
    fix_camera();
    set_update(true);
}

void Tilemap::move_cam_to_tile(Point tile_pos) {
    camera_pos = {tile_pos.x * tile_dim.w * zoom, tile_pos.y * tile_dim.h * zoom};
    Box visible = visible_tiles();
    BigPoint mid = {zoom * tile_dim.w * (visible.b.x - visible.a.x) / 2, zoom * tile_dim.h * (visible.b.y - visible.a.y) / 2};
    camera_pos = camera_pos - mid;
    fix_camera();
}

void Tilemap::zoom_cam(int factor) {
    if (factor > 0 && zoom * factor <= MAX_ZOOM) {
        move_cam({size.w/4, size.h/4});
        camera_pos.x *= factor;
        camera_pos.y *= factor;
        zoom *= factor;
    } else if (zoom * -((double)1/factor) >= MIN_ZOOM) {
        BigPoint camera_before = camera_pos;
        move_cam({-size.w/2, -size.h/2});
        camera_pos.x /= -factor;
        camera_pos.y /= -factor;
        zoom *= -((double)1/factor);
        fix_camera();
        if (camera_pos.x < 0 || camera_pos.y < 0) { // screen to small to contain map
            zoom /= -((double)1/factor);
            camera_pos = camera_before;
        }
    }
}

Box Tilemap::visible_tiles() {
    short xstart = (camera_pos.x) / (zoom * tile_dim.w);
    short xend = (camera_pos.x + size.w) / (zoom * tile_dim.w);
    short ystart = (camera_pos.y) / (zoom * tile_dim.h);
    short yend = (camera_pos.y + size.h) / (zoom * tile_dim.h);
    short pad = 1;
    xstart = xstart > pad ? xstart - pad : 0;
    ystart = ystart > pad ? ystart - pad : 0;
    xend   = xend + pad < map_size.w ? xend + pad : map_size.w - 1; 
    yend   = yend + pad < map_size.h ? yend + pad : map_size.h - 1; 
    return {Point(xstart, ystart), Point(xend, yend)};
}

void Tilemap::randomize_map() {
    for (auto& listener : click_listeners) {
        listener->map_changed();
    }
    MapGen::randomize_map(tiles_ground, tiles_above);
}

void Tilemap::mouse_clicked(Point p) {
    BigPoint click_pos = camera_pos + p - pos;
    Point click_tile = {click_pos.x / (tile_dim.w * zoom), click_pos.y / (tile_dim.h * zoom)};
    for (auto& l : click_listeners) {
        l->tile_clicked(click_tile);
    }
}

void Tilemap::fix_camera() {
    if (camera_pos.x < 0) camera_pos.x = 0;
    if (camera_pos.y < 0) camera_pos.y = 0;
    BigPoint camera_max = {(zoom * tile_dim.w) * map_size.w - size.w, (zoom * tile_dim.h) * map_size.h - size.h};
    camera_pos.x = camera_pos.x < camera_max.x ? camera_pos.x : camera_max.x;
    camera_pos.y = camera_pos.y < camera_max.y ? camera_pos.y : camera_max.y;
}

void Tilemap::draw() {
    if (!listener_registered) {
        Engine.input()->add_mouse_listener(this, {pos, size});
        listener_registered = true;
    }

    Texture* t_cursor = Engine.textures()->get(cursor_texture);
    Box canvas(pos, size);
    Point mpos = mouse_pos();
    bool do_update = t_cursor && canvas.inside(mpos) && mpos != last_mouse_pos;

    if (needs_update() || do_update) {
        Box visible = visible_tiles();
        BigPoint cam_ref(pos.x - camera_pos.x, pos.y - camera_pos.y);
        Size tile_size(tile_dim.w * zoom, tile_dim.h * zoom);
        for (short y = visible.a.y; y <= visible.b.y; y++) { 
            for (short x = visible.a.x; x <= visible.b.x; x++) { 
                Texture::ID id = tiles_ground->get(x,y);
                Texture* t = Engine.textures()->get(id < 0 ? -id : id);
                Engine.screen()->blit(t->pixels(zoom), t->size(zoom), {cam_ref.x + x * tile_size.w, cam_ref.y + y * tile_size.h}, canvas, false);
            }
        }
        for (short y = visible.a.y; y <= visible.b.y; y++) {
            for (short x = visible.a.x; x <= visible.b.x; x++) {
                Texture::ID id = tiles_above->get(x, y);
                if (id > 0) {
                    Texture* t = Engine.textures()->get(id);
                    Engine.screen()->blit(t->pixels(zoom), t->size(zoom), {cam_ref.x + x * tile_size.w, cam_ref.y + y * tile_size.h}, canvas, true);
                }
            }
        }        
        if (t_cursor && canvas.inside(mpos)) { // snap to tile
            BigPoint mouse_abs = camera_pos + mpos - pos;
            Point tile_abs = { mouse_abs.x / (tile_dim.w * zoom), mouse_abs.y / (tile_dim.h * zoom) };
            mouse_abs = { tile_abs.x * (tile_dim.w * zoom), tile_abs.y * (tile_dim.h * zoom) };
            tile_abs = { mouse_abs.x - camera_pos.x + pos.x, mouse_abs.y - camera_pos.y + pos.y };
            Engine.screen()->blit(t_cursor->pixels(zoom), t_cursor->size(zoom), tile_abs, canvas, t_cursor->transparent());
        }
        last_mouse_pos = mpos;
        set_update(false);
    }
    Composite::draw();
}
