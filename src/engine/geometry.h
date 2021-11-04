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
    bool operator==(const Point& p) { return x == p.x && y == p.y; }
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

struct Color {
    Color() {}
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255): red(r), green(g), blue(b), alpha(a) {}
    Color(unsigned u) { *this = u; }
    Color operator-(unsigned char d) { return Color(blue > d ? blue - d : 0, green > d ? green - d : 0, red > d ? red - d : 0, alpha); }
    Color& operator=(int i) { *((int*)this) = i; return *this; } 
    Color& operator=(unsigned u) { *((unsigned*)this) = u; return *this; } 
    Color& operator=(Color c) { *((unsigned*)this) = unsigned(c); return *this; } 
    operator int() { return *((int*)this); }
    operator unsigned() { return *((unsigned*)this); }
    unsigned char& operator[](int i) {
        switch (i) {
            case 0: return red;
            case 1: return green;
            case 2: return blue;
            case 3: return alpha;
            default: return red;
        }
    }
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
};

#endif
