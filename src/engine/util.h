#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <cstring>
#include <cassert>
#include <string>
#include <vector>
#include <variant>
#include <utility>
#include <algorithm>
#include <map>
#include <set>
#include <fstream>

// Utility data structures

struct Point {
    Point(): x(0), y(0) {}
    Point(short a, short b): x(a), y(b) {} 
    Point(int a, int b): x(a), y(b) {} 
    Point(int pos) { x = pos & 0x0000FFFF; y = (pos & 0xFFFF0000) >> 16; } 
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
    Size operator*(const float d) { return Size(d * w, d * h); }
    Size operator/(const Size& s) { return Size(w / s.w, h / s.h); }
};

struct Box {
    Box(): a({0, 0}), b({0, 0}) {}
    Box(Point first, Point second): a(first), b(second) {}
    Box(Point first, Size second): a(first), b({first.x+second.w, first.y+second.h}) {}
    Point a;
    Point b;
    Point center() { return {a.x + 0.5 * (b.x - a.x), a.y + 0.5 * (b.y - a.y)}; }
    Size size() {return Size(std::abs(b.x - a.x), std::abs(b.y - a.y));}
    bool inside(Point p) { return p.x >= a.x && p.y >= a.y && p.x <= b.x && p.y <= b.y; }
};

struct Color {
    Color(const Color& c) { *((unsigned*)this) = unsigned(c); };

    Color() {}
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255): blue(b), green(g), red(r), alpha(a) {}
    Color(unsigned u) { *this = u; }
    Color(int i) { *this = i; }
    Color operator-(unsigned char d) { return Color(red > d ? red - d : 0, green > d ? green - d : 0, blue > d ? blue - d : 0, alpha); }
    Color operator+(unsigned char d) { return Color(red + d < 255 ? red + d : 255, green + d < 255 ? green + d : 255, blue + d < 255 ? blue + d : 255, alpha); }
    Color& operator=(int i) { *((int*)this) = i; return *this; } 
    Color& operator=(unsigned u) { *((unsigned*)this) = u; return *this; } 
    Color& operator=(Color c) { *((unsigned*)this) = unsigned(c); return *this; } 
    operator int() { return *((int*)this); }
    operator unsigned() const { return *((unsigned*)this); }
    unsigned char& operator[](int i) {
        switch (i) {
            case 0: return blue;
            case 1: return green;
            case 2: return red;
            case 3: return alpha;
            default: return red;
        }
    }
    unsigned char blue = 0;
    unsigned char green = 0;
    unsigned char red = 0;
    unsigned char alpha = 0;
};

struct StringBase{};
template<int N> struct String : StringBase {
    String() {}
    String(const char* c) { strcpy(mem, c); }
    String(const std::string& c) { strncpy(mem, c.c_str(), c.size()); }
    String(const String<N>& c) { memcpy(mem, c.mem, N); }
    String(const String<N>&&) = delete;
    String<N>& operator=(const String<N>&&) = delete;
    String<N>& operator=(const String<N>& c) { memcpy(mem, c.mem, N); return *this;}
    String<N>& operator=(const std::string& c) { strncpy(mem, c.c_str(), c.size()); return *this;}
    String<N>& operator=(const char* c) { strcpy(mem, c); return *this;}
    std::string toStdString() { return std::string(mem); }
    char mem[N] = {0};
};
using String8 = String<8>;
using String16 = String<16>;
using String32 = String<32>;





// Utility methods

std::pair<Color*, Size> load_bmp(const std::string& filepath);
std::vector<std::pair<Color*, Size>> load_letters(const std::string& fontpath, int height, Color color, char start, char end);

std::vector<std::string> filelist(const std::string& path, const std::string& filter = "");
std::string filename(const std::string& filepath);
std::vector<std::string> split(const std::string& s, char delim);
void print(const std::string& s);

Color* create_window(Size s, bool fullscreen);
void update_window();

void wait(int us);
long long now();

double random_fast();
double random_uniform(double min, double max);
double random_gauss(double mean, double dev);

using AudioHandle = void*;
AudioHandle load_wav(const std::string& filepath, bool music);
void play_wav(AudioHandle audio, bool music);

Point mouse_pos();

#endif
