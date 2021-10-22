#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>

struct Point {
    Point(): x(0), y(0) {}
    Point(short a, short b): x(a), y(b) {} 
    Point(int a, int b): x(a), y(b) {} 
    Point(int pos) { 
        x = pos & 0x0000FFFF;
        y = (pos & 0xFFFF0000) >> 16;
    } 
    Point(double a, double b): x(a), y(b) {} 
    short x;
    short y;
    Point operator+(const Point& p) { return Point(x + p.x, y + p.y); }
    Point operator-(const Point& p) { return Point(x - p.x, y - p.y); }
    Point operator*(Point& p) { return Point(x * p.x, y * p.y); }
    Point operator/(Point& p) { return Point(x / p.x, y / p.y); }
    bool operator==(Point& p) { return x == p.x && y == p.y; }
    bool operator!=(Point& p) { return x != p.x || y != p.y; }
    short distance(const Point& p) { return std::abs(x - p.x) + std::abs(y - p.y); }
    operator int() { return *((int*)this); }
};

struct BigPoint {
    BigPoint(short a, short b): x(a), y(b) {} 
    BigPoint(int a, int b): x(a), y(b) {} 
    BigPoint(double a, double b): x(a), y(b) {} 
    int x;
    int y;
    BigPoint operator+(const BigPoint& p) { return BigPoint(x + p.x, y + p.y); }
    BigPoint operator+(const Point& p) { return BigPoint(x + p.x, y + p.y); }
    BigPoint operator-(const BigPoint& p) { return BigPoint(x - p.x, y - p.y); }
    BigPoint operator-(const Point& p) { return BigPoint(x - p.x, y - p.y); }
    BigPoint operator*(BigPoint& p) { return BigPoint(x * p.x, y * p.y); }
    BigPoint operator/(BigPoint& p) { return BigPoint(x / p.x, y / p.y); }
    bool operator==(const BigPoint& p) { return x == p.x && y == p.y; }
    short distance(const BigPoint& p) { return std::abs(x - p.x) + std::abs(y - p.y); }
};

struct Size {
    Size(): w(0), h(0) {}
    Size(short a, short b): w(a), h(b) {} 
    Size(int a, int b): w(a), h(b) {} 
    Size(double a, double b): w(a), h(b) {} 
    short w;
    short h;
    Size operator*(const Size& s) { return Size(w * s.w, h * s.h); }
    Size operator/(const Size& s) { return Size(w / s.w, h / s.h); }
};

struct Box {
    Box(): a({0, 0}), b({0, 0}) {}
    Box(Point first, Point second): a(first), b(second) {}
    Box(Point first, Size second): a(first), b({first.x+second.w, first.y+second.h}) {}
    Point a;
    Point b;
    Point center() { return {a.x + 0.5 * (b.x - a.x), a.y + 0.5 * (b.y - a.y)}; }
    bool inside(Point p) { return p.x >= a.x && p.y >= a.y && p.x <= b.x && p.y <= b.y; }
};

/*
struct PathElement {
    Point pos;
    short g;
    short f;
};

int path_length(Point src, Point dst, int maxlen, int roadid) {
        auto& map = Engine.db()->get_matrix<Texture::ID>("tiles");
        std::priority_queue<Point> pqueue;
        
        Texture::ID id = map.get(src.x, src.y);
        std::vector<PathElement> start_tiles;
        if (id != roadid) {
            Size s = Engine.textures()->get(id)->size() / tile_dim;
            for (short y = src.y-1 > 0 ? src.y-1 : 0; y < map_size.h && y < src.y+s.h+1; y++) { 
                for (short x = src.x-1 > 0 ? src.x-1 : 0; x < map_size.w && x < src.x+s.w+1; x++) { 
                    if (map.get(x, y) == roadid) {
                        short d = dst.distance({x, y}) + 1;
                        if (d == 2) {
                            return 1;
                        }
                        pqueue.emplace_back({x, y}, d);
                    }
                }
            }
        } else {
            pqueue.emplace_back(src, 1, dst.distance(src) + 1, true);
        }

        std::unordered_set<Point> openset, closedset;

        while (!pqueue.empty()) {
            auto& pop = pqueue.pop();
            pop.open = false;
            if (pop->g >= maxlen) {
                continue;
            }

            std::array<Point, 4> adjacent = {
                {pop->pos.x, pop->pos.y-1}, {pop->pos.x+1, pop->pos.y}, {pop->pos.x, pop->pos.y+1}, {pop->pos.x-1, pop->pos.y}
            };
            for (Point adj : adjacent) {
                if (!adj.valid || openset.find(adj) != openset.end()) {
                    continue;
                }
                int h = dst.distance(adj.pos);

                if (!adj->open) {
                    continue;
                } else if (tfp->isOpen && (h + pop->g < tfp->f)) {
                    tfp->g = pop->g + 1;
                    tfp->f = h + tfp->g;
                    pqueue.update();
                    continue;
                } else {
                    tfp->g = pop->g + 1;
                    tfp->f = h + tfp->g;
                }

                if (h == 1) {
                    pqueue.clear();
                    return tfp->g;
                }

                pqueue.push(tfp);
                tfp->isOpen = true;
            }
        }
        pqueue.clear();
        return 0;
    }
*/
#endif
