#include "tilemap.h"
#include "config.h"
#include "mapgen.h"

#define groundid_get(x, y) (Texture::ID)(tiles->get(x, y) & 0x0000FFFF)
#define groundid_set(x, y, v) tiles->get(x, y) = (tiles->get(x, y) & 0xFFFF0000) | (unsigned)(((unsigned short)v) & 0x0000FFFF)
#define aboveid_get(x, y) (Texture::ID)((tiles->get(x, y) & 0xFFFF0000) >> 16)
#define aboveid_set(x, y, v) tiles->get(x, y) = (tiles->get(x, y) & 0x0000FFFF) | (unsigned)((((unsigned short)v) & 0x0000FFFF) << 16)


void Tilemap::create_map(Size screen_size) {
    map_size = {Engine.config()->get("settings")["mapsize"]["width"].i(), Engine.config()->get("settings")["mapsize"]["height"].i()};
    tile_dim = {Engine.config()->get("settings")["tilesize"]["width"].i(), Engine.config()->get("settings")["tilesize"]["height"].i()};
    size = screen_size;
    tiles = Engine.db()->get_matrix<unsigned>("tiles", map_size.w, map_size.h);
    infinite_scrolling = Engine.config()->get("settings")["infinite_scrolling"].i();
    use_fast_renderer = (bool)(Engine.config()->get("settings")["use_fast_renderer"].i());
    for (auto& listener : click_listeners) {
        listener->map_changed();
    }
}

Texture::ID Tilemap::get_ground(Point p) { 
    Texture::ID id = groundid_get(p.x, p.y); 
    return id < 0 ? -id : id; 
}

bool Tilemap::set_ground(Texture::ID id, Point p, bool blocked) {
    groundid_set(p.x, p.y, blocked ? -id : id);
    return true;
}

bool Tilemap::set_ground(const std::string& texture_name, Point p, bool blocked) {
    if (groundid_get(p.x, p.y) < 0) {
        return false;
    }
    Texture* texture = Engine.textures()->get(texture_name);
    groundid_set(p.x, p.y, blocked ? -texture->id() : texture->id());
    return true;
}

bool Tilemap::set_tile(const std::string& texture_name, Point p) {
    Texture* texture = Engine.textures()->get(texture_name);
    Size s = texture->size() / tile_dim;  
    return set_tile(texture->id(), p, s);
}

bool Tilemap::set_tile(Texture::ID id, Point p, Size s) {
    if (p.x + s.w >= map_size.w || p.y + s.h >= map_size.h) {
        return false;
    }
    for (short y = p.y; y < p.y + s.h; y++) {
        for (short x = p.x; x < p.x + s.w; x++) {
            if (groundid_get(x, y) < 0 || aboveid_get(x, y) != 0) {
                return false;
            }
        }
    }
    for (short y = p.y; y < p.y + s.h; y++) {
        for (short x = p.x; x < p.x + s.w; x++) {
            aboveid_set(x, y, x == p.x && y == p.y ? id : -id);
        }
    }
    return true;
}

Point Tilemap::texture_root(Point p) {
    Texture::ID id = aboveid_get(p.x, p.y);
    if (id < 0) {
        Size s = Engine.textures()->get(-id)->size() / tile_dim;
        for (short y = p.y; y > p.y - s.h && y >= 0; y--) { 
            for (short x = p.x; x > p.x - s.w && x >= 0 ; x--) {
                Texture::ID current_id = aboveid_get(x, y);
                if (current_id == -id) {
                    return {x, y, map_size};
                }
            }
        }
    }
    return p;
}

void Tilemap::unset_tile(Point ps) {
    Point p = texture_root(ps);
    Texture::ID id = aboveid_get(p.x, p.y);
    if (id <= 0) {
        return;
    }
    Size s = Engine.textures()->get(id)->size() / tile_dim;
    for (short y = p.y; y < p.y + s.h; y++) { 
        for (short x = p.x; x < p.x + s.w; x++) {
            if ((x == p.x && y == p.y) || aboveid_get(x, y) < 0) {
                aboveid_set(x, y, 0);
            }
        }
    }
}

void Tilemap::move_cam(Point p) {
    if ((p.x != 0 && move_vector.x == 0)) {
        move_vector.x = p.x;
        set_update(true);
    } 
    if ((p.y != 0 && move_vector.y == 0)) {
        move_vector.y = p.y;
        set_update(true);
    }
}

void Tilemap::move_cam_to_tile(Point tile_pos) {
    camera_pos = {tile_pos.x * tile_dim.w * zoom, tile_pos.y * tile_dim.h * zoom};
    Box visible = visible_tiles();
    Camera mid = {zoom * tile_dim.w * (visible.b.x - visible.a.x) / 2, zoom * tile_dim.h * (visible.b.y - visible.a.y) / 2};
    camera_pos = camera_pos - mid;
    fix_camera();
}

void Tilemap::zoom_cam(int factor) {
    if (factor > 0 && zoom * factor <= MAX_ZOOM) {
        camera_pos.x = (camera_pos.x + size.w / 4) * factor;
        camera_pos.y = (camera_pos.y + size.h / 4) * factor;
        zoom *= factor;
        fix_camera();
        set_update(true);
    } else if (zoom * -((double)1/factor) >= MIN_ZOOM) {
        camera_pos.x = (camera_pos.x - size.w / 2) / (-factor);
        camera_pos.y = (camera_pos.y - size.h / 2) / (-factor);
        zoom *= -((double)1/factor);
        fix_camera();
        set_update(true);
    }
}

Box Tilemap::visible_tiles() {
    constexpr short pad = 1;
    short xstart = (camera_pos.x) / (zoom * tile_dim.w) - pad;
    short xend = (camera_pos.x + size.w) / (zoom * tile_dim.w) + pad;
    short ystart = (camera_pos.y) / (zoom * tile_dim.h) - pad;
    short yend = (camera_pos.y + size.h) / (zoom * tile_dim.h) + pad;
    return {Point(xstart, ystart), Point(xend, yend)};
}

void Tilemap::randomize_map() {
    for (auto& listener : click_listeners) {
        listener->map_changed();
    }
    for (short y_map = 0; y_map < map_size.h; y_map++) {
        for (short x_map = 0; x_map < map_size.w; x_map++) {
            tiles->get(x_map, y_map) = 0;
        }
    }
    MapGen::randomize_map();
}

void Tilemap::mouse_clicked(Point p) {
    Camera click_pos = camera_pos + p - pos;
    Point click_tile(click_pos.x / (tile_dim.w * zoom), click_pos.y / (tile_dim.h * zoom), map_size);
    for (auto& l : click_listeners) {
        l->tile_clicked(click_tile);
    }
}

void Tilemap::fix_camera() {
    Camera camera_max = {(zoom * tile_dim.w) * map_size.w - size.w, (zoom * tile_dim.h) * map_size.h - size.h};
    if (infinite_scrolling) {
        if (camera_pos.x < -camera_max.x) camera_pos.x += (camera_max.x + size.w);
        if (camera_pos.y < -camera_max.y) camera_pos.y += (camera_max.y + size.h);
        if (camera_pos.x >= 2 * camera_max.x) camera_pos.x -= (camera_max.x + size.w);
        if (camera_pos.y >= 2 * camera_max.y) camera_pos.y -= (camera_max.y + size.h);
    } else {
        if (camera_pos.x < 0) camera_pos.x = 0;
        if (camera_pos.y < 0) camera_pos.y = 0;
        camera_pos.x = camera_pos.x < camera_max.x ? camera_pos.x : camera_max.x;
        camera_pos.y = camera_pos.y < camera_max.y ? camera_pos.y : camera_max.y;
    }
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
        if (move_vector.x == move_vector.y) {
            move_vector.x = sqrt(move_vector.x * move_vector.x + move_vector.y * move_vector.y);
            move_vector.x = move_vector.y;
        }
        camera_pos = camera_pos + move_vector;
        fix_camera();
        move_vector = {0, 0};

        if (use_fast_renderer) {
            fast_render();
        } else {
            Box canvas(pos, size);
            const Box visible = visible_tiles();
            const Size tile_size(tile_dim.w * zoom, tile_dim.h * zoom);
            for (short y = visible.a.y; y <= visible.b.y; y++) {
                for (short x = visible.a.x; x <= visible.b.x; x++) {
                    Point p(x, y, map_size);
                    Point start(pos.x - camera_pos.x + x * tile_size.w, pos.y - camera_pos.y + y * tile_size.h);
                    Texture::ID ground_id = groundid_get(p.x, p.y);
                    Texture* texture_ground = Engine.textures()->get(ground_id < 0 ? -ground_id : ground_id);
                    Engine.screen()->blit(texture_ground->pixels(zoom), tile_size, start, canvas, false);
                    Texture::ID above_id = aboveid_get(p.x, p.y);
                    if (above_id != 0) {
                        Point p_root = p;
                        if (above_id < 0) {
                            p_root = texture_root(p);
                            above_id = aboveid_get(p_root.x, p_root.y);
                        }
                        Texture* texture_above = Engine.textures()->get(above_id);
                        Size texture_size = texture_above->size(zoom);
                        Color* pixels_above = (Color*)(texture_above->pixels(zoom) + tile_size.h * texture_size.w * (p.y - p_root.y) + tile_size.w * (p.x - p_root.x)); 
                        Engine.screen()->blit(pixels_above, tile_size, start, canvas, true, texture_size.w);
                    }
                }
            }
        }
        
        if (t_cursor && canvas.inside(mpos)) { // snap to tile
            Camera mouse_abs = camera_pos + mpos - pos;
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



struct CachedTile {
    unsigned id = 0;
    Color* pixels = 0;
};
constexpr int CACHESIZE = 8;

void Tilemap::fast_render() {
    const Box canvas(pos, size);
    const Box visible = visible_tiles();
    Texture** textures_map = Engine.textures()->id_to_texture;
    const int zoom_level = Texture::zoom2idx(zoom);

    const int tile_size_w = tile_dim.w * zoom;
    const int tile_size_h = tile_dim.h * zoom;
    const int map_size_w = map_size.w;
    const int map_size_h = map_size.h;
    const int cam_ref_x = pos.x - camera_pos.x;
    const int cam_ref_y = pos.y - camera_pos.y;
    const int screen_size_w = Engine.screen()->get_size().w;
    const int visible_a_x = visible.a.x;
    const int visible_b_x = visible.b.x;
    const int visible_a_y = visible.a.y;
    const int visible_b_y = visible.b.y;
    const int canvas_a_x = canvas.a.x;
    const int canvas_b_x = canvas.b.x;
    const int canvas_a_y = canvas.a.y;
    const int canvas_b_y = canvas.b.y;

    parallel_for(visible_a_y, visible_b_y, [=](int y) {
        int p_y = y;
        if (y < 0) p_y += ((-y / map_size_h + 1)) * map_size_h;
        else if (y >= map_size_h) p_y %= map_size_h;
        
        int start_y = cam_ref_y + y * tile_size_h;
        int texture_end_y = start_y + tile_size_h;
        int texture_start_y = 0;
        int texture_endcut_y = 0;
        if (start_y < canvas_a_y) {
            texture_start_y = (canvas_a_y - start_y);
            start_y = canvas_a_y;
        } else if (texture_end_y > canvas_b_y) {
            texture_endcut_y = texture_end_y - canvas_b_y;
        }
        const int upper_bound_y = tile_size_h - texture_start_y - texture_endcut_y;

        const unsigned* __restrict elems = tiles->elems + p_y * map_size_w;     
        unsigned* __restrict screen = (unsigned*)(Engine.screen()->pixels + start_y * screen_size_w);

        static thread_local CachedTile cached_tiles[CACHESIZE];
        if (y == visible_a_y) {
            std::memset(cached_tiles, 0, sizeof(CachedTile) * CACHESIZE);
        }
        for (int x = visible_a_x; x <= visible_b_x; ++x) {
            int start_x = cam_ref_x + x * tile_size_w;
            int texture_end_x = start_x + tile_size_w;
            int texture_start_x = 0;
            int texture_endcut_x = 0;
            if (start_x < canvas_a_x) {
                texture_start_x = (canvas_a_x - start_x);
                start_x = canvas_a_x;
            } else if (texture_end_x > canvas_b_x) {
                texture_endcut_x = texture_end_x - canvas_b_x;
            }
            const int upper_bound_x = tile_size_w - texture_start_x - texture_endcut_x;

            int p_x = x;
            if (x < 0) p_x += ((-x / map_size_w) + 1) * map_size_w;
            else if (x >= map_size_w) p_x %= map_size_w;
            const unsigned current_id = elems[p_x];
            int above_id = (short)((current_id & 0xFFFF0000) >> 16);
            Color* __restrict ground_pixels = nullptr;
            int ground_size_w = tile_size_w;

            unsigned* __restrict screen_pixels = (unsigned*)(screen + start_x);   
            if (above_id > 0) {
                for (int i = 0; i < CACHESIZE; i++) {
                    if (cached_tiles[i].id == current_id) {
                        ground_pixels = cached_tiles[i].pixels;
                        ground_size_w = screen_size_w;
                        above_id = 0;
                        break;
                    } else if (!cached_tiles[i].id && upper_bound_x == tile_size_w && upper_bound_y > tile_size_h) {
                        cached_tiles[i].pixels = (Color*)(screen_pixels);
                        cached_tiles[i].id = current_id;
                        break;
                    }
                }
            }
            if (!ground_pixels) {
                const int ground_id = (short)(current_id & 0xFFFF);
                ground_pixels = textures_map[ground_id < 0 ? -ground_id : ground_id]->pixel_map[zoom_level];
            }
            unsigned* __restrict texture_pixels = (unsigned*)(ground_pixels + texture_start_y * ground_size_w + texture_start_x); 

            if (above_id) {
                Texture* above_texture = textures_map[above_id < 0 ? -above_id : above_id];
                const int above_size_w = above_texture->m_size.w * zoom;
                const int above_size_h = above_texture->m_size.h * zoom;
                Color* __restrict above_pixels = above_texture->pixel_map[zoom_level] + texture_start_y * above_size_w + texture_start_x;
                if (above_id < 0) {
                    for (int y = p_y; y > p_y - above_size_h / tile_size_h; y--) { 
                        for (int x = p_x; x > p_x - above_size_w / tile_size_w ; x--) {
                            if (x == p_x && y == p_y) continue;
                            else if ((short)((tiles->elems[x + map_size_w * y] & 0xFFFF0000) >> 16) == -above_id) {
                                Point p_root(x, y, map_size);
                                above_pixels += tile_size_h * above_size_w * (p_y - p_root.y) + tile_size_w * (p_x - p_root.x); 
                                above_id = 0;
                                break;
                            }
                        }
                        if (!above_id) break;
                    }
                }
                for (int y = 0; y < upper_bound_y; y++) {
                    for (int x = 0; x < upper_bound_x; x++) {
                        unsigned color1 = texture_pixels[y * tile_size_w + x];
                        unsigned color2 = above_pixels[y * above_size_w + x];
                        unsigned rb = (color1 & 0xff00ff) + (((color2 & 0xff00ff) - (color1 & 0xff00ff)) * ((color2 & 0xff000000) >> 24) >> 8);
                        unsigned g  = (color1 & 0x00ff00) + (((color2 & 0x00ff00) - (color1 & 0x00ff00)) * ((color2 & 0xff000000) >> 24) >> 8);
                        screen_pixels[y * screen_size_w + x] = (rb & 0xff00ff) | (g & 0x00ff00);
                    }
                }
            } else if (upper_bound_x == 2 && upper_bound_y == 2) {
                *((unsigned long long*)screen_pixels) = *((unsigned long long*)texture_pixels);
                *((unsigned long long*)(screen_pixels + screen_size_w)) = *((unsigned long long*)(texture_pixels + ground_size_w));
            } else if (upper_bound_x > 0) {
                for (int y2 = 0; y2 < upper_bound_y; y2++) {
                    std::memcpy(screen_pixels + y2 * screen_size_w, texture_pixels + y2 * ground_size_w, upper_bound_x * sizeof(unsigned));
                }
            }
        }
    });
}