#include "util.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <filesystem>
#include <chrono>
#include <random>
#include <iostream>

std::pair<Color*, Size> load_bmp(const std::string& filepath) {
    SDL_Surface* img = SDL_LoadBMP(filepath.c_str());
    Size s = {img->w, img->h};
    unsigned* out = new unsigned[s.w * s.h];
    std::memcpy(out, img->pixels, s.w * s.h * sizeof(unsigned)); 
    SDL_FreeSurface(img);
    return {(Color*)out, s};
}

std::vector<std::pair<Color*, Size>> load_letters(const std::string& fontpath, int height, Color color, char start, char end) {
    std::vector<std::pair<Color*, Size>> ret;
    if (!TTF_WasInit()) {
        TTF_Init();
    }
    TTF_Font* font = TTF_OpenFont(fontpath.c_str(), height);
    SDL_Color fgcolor = {color.red, color.blue, color.green, color.alpha};
    union LU { char c[2] = {0, 0}; unsigned short us; };
    LU letter;
    for (char c = start; c < end; c++) {
        letter.c[0] = c;
        SDL_Surface* img = TTF_RenderGlyph_Blended(font, letter.us, fgcolor);
        Size s = {img->w, img->h};
        Color* out = new Color[s.w * s.h];
        std::memcpy(out, img->pixels, s.w * s.h * sizeof(Color)); 
        SDL_FreeSurface(img);
        ret.push_back({out, s});
    }
    return ret;
}

std::vector<std::string> filelist(const std::string& path, const std::string& filter) {
    std::vector<std::string> ret;
    for (auto& file : std::filesystem::recursive_directory_iterator(path)) {
        std::string filepath = file.path().string();
        if (filter.empty() || filepath.find(filter) != std::string::npos) {
            ret.push_back(filepath);
        }
    }
    return ret;
}

std::string filename(const std::string& filepath) {
    std::string f = filepath;
    if (f.find("\\") != std::string::npos) {
        f = f.substr(f.find_last_of("\\") + 1);
    } else if (f.find("/") != std::string::npos) {
        f = f.substr(f.find_last_of("/") + 1);
    }
    if (f.find(".") != std::string::npos) {
        f = f.substr(0, f.find_last_of("."));
    }
    return f;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    if (s.back() == delim) {
        result.emplace_back();
    }
    return result;
}

void print(const std::string& s) { std::cout << s << std::endl; }

static SDL_Window* window = nullptr;

Color* create_window(Size s, bool fullscreen) {
    if (!window) {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, s.w, s.h, 0);
        if (fullscreen) {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        }
    }
    return (Color*)SDL_GetWindowSurface(window)->pixels;
}

void update_window() {
    if (window) {
        SDL_UpdateWindowSurface(window);
    }
}

void wait(int us) {
    SDL_Delay(us / 1000);
}

long long now() {
    return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
}

double random_fast() {
    static unsigned long long r = 0;
    static bool init = false;
    if (!init) {
        unsigned seed = (unsigned)now();
        auto generator = std::default_random_engine(seed);
        std::uniform_int_distribution<int> distribution(0, 2147483647);
        r = distribution(generator);
        init = true;
    }
    r = (r * 48271) % 2147483648;
    return (double)r / 2147483648;
}

double random_uniform(double min, double max) {
    static unsigned seed = (unsigned)now();
    static std::mt19937 generator(seed);
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

double random_gauss(double mean, double dev) {
    static unsigned seed = (unsigned)now();
    static std::mt19937 generator(seed);
    std::normal_distribution<double> distribution(mean, dev);
    return distribution(generator);
}
           
AudioHandle load_wav(const std::string& filepath, bool music) {
    Mix_Init(MIX_INIT_OGG);
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_Volume(-1, 64);
    if (music) {
        return Mix_LoadMUS(filepath.c_str());
    }
    return Mix_LoadWAV(filepath.c_str()); 
}

void play_wav(AudioHandle audio, bool music) {
    if (music) {
        //Mix_VolumeMusic(volume);
        Mix_PlayMusic((Mix_Music*)audio, -1); 
    } else {
        Mix_PlayChannel(-1, (Mix_Chunk*)audio, 0);
    }
}
        
Point mouse_pos() {
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    return {mx, my};
}
