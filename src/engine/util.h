#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <variant>
#include <utility>
#include <algorithm>
#include <map>
#include <functional>

// Utility data structures

struct Size {
    short w;
    short h;
    Size(): w(0), h(0) {}
    Size(short a, short b): w(a), h(b) {} 
    Size(int a, int b): w(a), h(b) {} 
    Size(double a, double b): w(a), h(b) {} 
    Size operator*(const Size& s) { return Size(w * s.w, h * s.h); }
    Size operator*(const float d) { return Size(d * w, d * h); }
    Size operator/(const Size& s) { return Size(w / s.w, h / s.h); }
};

struct Point {
    short x;
    short y;
    Point(): x(0), y(0) {}
    Point(short a, short b): x(a), y(b) {} 
    Point(short a, short b, Size max_size): x(a), y(b) { wrap(max_size); }
    Point(int a, int b): x(a), y(b) {} 
    Point(int pos) { x = pos & 0x0000FFFF; y = (pos & 0xFFFF0000) >> 16; } 
    Point(double a, double b): x(a), y(b) {} 
    Point operator+(const Point& p) { return Point(x + p.x, y + p.y); }
    Point operator-(const Point& p) { return Point(x - p.x, y - p.y); }
    Point operator*(Point& p) { return Point(x * p.x, y * p.y); }
    Point operator/(Point& p) { return Point(x / p.x, y / p.y); }
    bool operator==(const Point& p) { return x == p.x && y == p.y; }
    bool operator!=(Point& p) { return x != p.x || y != p.y; }
    short distance(const Point& p) { return std::abs(x - p.x) + std::abs(y - p.y); }
    operator int() { return *((int*)this); }
    void wrap(Size max_size) {
        if (x < 0) x += ((-x / max_size.w) + 1) * max_size.w;
        else if (x >= max_size.w) x %= max_size.w;
        if (y < 0) y += ((-y / max_size.h) + 1) * max_size.h;
        else if (y >= max_size.h) y %= max_size.h;
    }
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
    unsigned char blue = 0;
    unsigned char green = 0;
    unsigned char red = 0;
    unsigned char alpha = 0;
};

template<int N> struct String {
    String() {}
    String(const char* c) { strcpy(mem, c); }
    String(const std::string& c) { strncpy(mem, c.c_str(), c.size()); }
    String(const String<N>& c) { memcpy(mem, c.mem, N); }
    String(const String<N>&&) = delete;
    String<N>& operator=(const String<N>&&) = delete;
    String<N>& operator=(const String<N>& c) { memcpy(mem, c.mem, N); return *this;}
    String<N>& operator=(const std::string& c) { strncpy(mem, c.c_str(), c.size()); return *this;}
    //String<N>& operator=(const char* c) { strcpy(mem, c); return *this;}
    std::string toStdString() { return std::string(mem); }
    char mem[N] = {0};
};
using String8 = String<8>;
using String16 = String<16>;
using String32 = String<32>;





// Utility methods

std::pair<Color*, Size> load_bmp(const std::string& filepath);
std::vector<std::pair<Color*, Size>> load_letters(const std::string& fontpath, int height, Color color, char start, char end);

using FileHandle = void*;
FileHandle file_open(const std::string& path);
void file_close(FileHandle file);
void file_read(FileHandle file, char* buffer, int num_bytes);
void file_write(FileHandle file, char* buffer, int num_bytes);
std::string file_readline(FileHandle file);
void file_writeline(FileHandle file, const std::string& s);
bool file_isend(FileHandle file);
bool file_exists(const std::string& path);

std::vector<std::string> filelist(const std::string& path, const std::string& filter = "");
std::string filename(const std::string& filepath);
std::vector<std::string> split(const std::string& s, char delim);
void replace(std::string& s, const std::string& from, const std::string& to);
double to_double(const std::string& s);
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

void pressed_keys(std::vector<std::string>& pressed, std::vector<std::string>& released);
Point mouse_pos();

void compress(void* in_data, int in_len, void* out_data, int& out_len);
void decompress(void* in_data, int in_len, void* out_data, int out_len);

void parallel_for(int begin, int end, const std::function<void(int)>& f);



enum class ScriptType {NUMBER, STRING, LIST};

struct ScriptParam {
    using ScriptValue = std::variant<double, std::string, std::vector<ScriptParam>, std::map<std::string, ScriptParam>>;
    ScriptParam(int i): val((double)i) {}
    ScriptParam(double d): val(d) {}
    ScriptParam(const std::vector<ScriptParam>& l): val(l) {}
    ScriptParam(const ScriptValue& v): val(v) {}
    double d() const { return std::get<0>(val); }
    std::string s() const { return std::get<1>(val); }
    std::vector<ScriptParam> l() const { return std::get<2>(val); }
    ScriptValue val;
};

struct ScriptFunction {
    ScriptFunction() {}
    ScriptFunction(const std::string& n, const ScriptType& t_out, const std::vector<ScriptType>& t_in, const std::function<ScriptParam(const std::vector<ScriptParam>&)> f):
    name(n), return_type(t_out), param_types(t_in), func(f)  {}
    ScriptFunction(const ScriptFunction& other): name(other.name), return_type(other.return_type), param_types(other.param_types), func(other.func) {}
    std::string name;
    ScriptType return_type;
    std::vector<ScriptType> param_types;
    std::function<ScriptParam(const std::vector<ScriptParam>& params)> func;
};

void add_script_function(const ScriptFunction& function);
void run_script(const std::string& filepath);

#endif
