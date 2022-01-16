#include "util.h"

#include <iostream>
#include <cstdlib>
#include "extern/SDL2/SDL.h"

std::pair<Color*, Size> load_bmp(const std::string& filepath) {
    SDL_Surface* img = SDL_LoadBMP(filepath.c_str());
    Size s = {img->w, img->h};
    unsigned* out = new unsigned[s.w * s.h];
    std::memcpy(out, img->pixels, s.w * s.h * sizeof(unsigned)); 
    SDL_FreeSurface(img);
    return {(Color*)out, s};
}


#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "extern/stb_truetype.h"
#include <stdio.h>
std::vector<std::pair<Color*, Size>> load_letters(const std::string& fontpath, int height, Color color, char start, char end) {
    static unsigned char ttf_buffer[1<<25];
    std::vector<std::pair<Color*, Size>> ret;
    FILE* fontfile = fopen(fontpath.c_str(), "rb");
    fread(ttf_buffer, 1, 1<<25, fontfile);
    fclose(fontfile);
    stbtt_fontinfo font;
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
    float scale = stbtt_ScaleForPixelHeight(&font, (float)height);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);  
    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);

    for (char c = start; c < end; c++) {
        int leftSideBearing;
	    int advanceWidth;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
        advanceWidth *= c == ' ' ? scale : 0;
        leftSideBearing *= scale;
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        int y_char = ascent + c_y1;
        int w = c_x2 - c_x1;
        int h = c_y2 - c_y1;
        unsigned char* bitmap = new unsigned char[w * h];
        stbtt_MakeCodepointBitmap(&font, bitmap, w, h, w, scale, scale, c);
        Size s(w + leftSideBearing + advanceWidth, y_char + h);
        Color* out = new Color[s.w * s.h];
        for (int y = y_char; y < s.h; y++) {
            for (int x = leftSideBearing; x < w + leftSideBearing; x++) {
                out[y * s.w + x] = color;
                out[y * s.w + x].alpha = bitmap[(y - y_char) * w + (x - leftSideBearing)];
            }
        }
        delete[] bitmap;
        ret.push_back({out, s});
    }
    return ret;
}



#include <filesystem>
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

#include <sstream>
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (delim != ' ' || !item.empty()) {
            result.push_back(item);
        }
    }
    if (s.empty() || s.back() == delim) {
        result.emplace_back();
    }
    return result;
}

void replace(std::string& s, const std::string& from, const std::string& to) {
    if(from.empty()) {
        return;
    }
    size_t start_pos = 0;
    while ((start_pos = s.find(from, start_pos)) != std::string::npos) {
        s.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

double to_double(const std::string& s) {
    const char* cp = s.c_str();
    char* endptr;
    double d = strtod(cp, &endptr);
    if (cp == endptr) {
        return std::nan("");
    }
    return d;
}

void print(const std::string& s) { std::cout << s << std::endl; }



static SDL_Window* window = nullptr;

Color* create_window(Size s, bool fullscreen) {
    if (!window) {
#ifdef _WIN32
        SDL_setenv("SDL_AUDIODRIVER", "directsound", true); 
#endif
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
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




#include <chrono>
#include <random>
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
    static std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

double random_gauss(double mean, double dev) {
    static unsigned seed = (unsigned)now();
    static std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(mean, dev);
    return distribution(generator);
}




struct AudioFile {
    SDL_AudioSpec spec;
    Uint32 length;
    Uint8* buffer;
    int length_remaining = 0;
    bool loop;
};

static std::set<AudioFile*> queued_audio;
static SDL_AudioFormat audio_format = AUDIO_S16LSB;
static SDL_AudioDeviceID dev;   

void audio_callback(void*, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);
    std::vector<AudioFile*> remove;
    for (auto& audio : queued_audio) {
        unsigned play_len = audio->length_remaining > len ? len : audio->length_remaining;
        SDL_MixAudioFormat(stream, audio->buffer + audio->length - audio->length_remaining, audio_format, play_len, 50);
        audio->length_remaining -= play_len;
        if (!audio->length_remaining) {
            remove.push_back(audio);
        } 
    }
    for (auto& audio : remove) {
        queued_audio.erase(audio);
    }
}

AudioHandle load_wav(const std::string& filepath, bool music) {
    static bool audio_init = false;
    if (!audio_init) {
        SDL_AudioSpec want, have;
        SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
        want.freq = 48000;
        want.format = audio_format;
        want.channels = 2;
        want.samples = 1024;
        want.callback = audio_callback;
        dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
        SDL_PauseAudioDevice(dev, 0);
        audio_init = true;
    }
    AudioFile* handle = new AudioFile();
    handle->loop = music;
    SDL_LoadWAV(filepath.c_str(), &handle->spec, &handle->buffer, &handle->length);
    return handle;
}

void play_wav(AudioHandle audio, bool) {
    AudioFile* handle = (AudioFile*)audio;
    if (!handle) {
        return;
    }
    if (!handle->length_remaining && queued_audio.find(handle) == queued_audio.end()) {
        handle->length_remaining = handle->length;
        queued_audio.insert(handle);
    }
}




void pressed_keys(std::vector<std::string>& pressed, std::vector<std::string>& released) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEWHEEL: {
                pressed.push_back(event.wheel.y > 0 ? "WheelUp" : "WheelDown");
                break;
            }
            case SDL_KEYDOWN: {
                pressed.push_back(SDL_GetKeyName(event.key.keysym.sym));
                break;
            }
            case SDL_KEYUP: {
                released.push_back(SDL_GetKeyName(event.key.keysym.sym));
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    pressed.push_back("MouseLeft");
                }
                break; 
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    exit(0);
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                break;
        }
    }
}

Point mouse_pos() {
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    return {mx, my};
}



#define SINFL_IMPLEMENTATION
#define SDEFL_IMPLEMENTATION
#include "extern/sdefl.h"
#include "extern/sinfl.h"
static struct sdefl sdefl;

void compress(void* in_data, int in_len, void* out_data, int& out_len) {
    out_len = sdeflate(&sdefl, out_data, in_data, in_len, 1);
}

void decompress(void* in_data, int in_len, void* out_data, int out_len) {
    sinflate(out_data, out_len, in_data, in_len);
}




#include <deque>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class ThreadPool {
    public:
    ThreadPool(int threads = 4) {
        for (int i = 0; i < threads; ++i) {
            m_threads.emplace_back(std::thread([this]() {
                while (true) {
                    std::unique_lock<std::mutex> latch(m_queue_mutex);
                    cv_task.wait(latch, [this](){ return stop || !m_workQueue.empty(); });
                    if (!m_workQueue.empty())
                    {
                        ++busy;

                        auto fn = m_workQueue.front();
                        m_workQueue.pop_front();

                        latch.unlock();

                        fn();

                        latch.lock();
                        --busy;
                        cv_finished.notify_one();
                    } else if (stop) {
                        break;
                    }
                }
            }));
        }
    }

    ~ThreadPool() {
        std::unique_lock<std::mutex> latch(m_queue_mutex);
        stop = true;
        cv_task.notify_all();
        latch.unlock();
        for (auto& t : m_threads) {
            t.join();
        }
    }

    template<typename F>
    void add(F&& f) {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_workQueue.emplace_back(std::forward<F>(f));
        cv_task.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        cv_finished.wait(lock, [this](){ return m_workQueue.empty() && (busy == 0); });
    }

private:
    std::vector<std::thread> m_threads;
    std::deque<std::function<void(void)>> m_workQueue;
    std::mutex m_queue_mutex;
    std::condition_variable cv_task;
    std::condition_variable cv_finished;
    unsigned busy = 0;
    bool stop = false;
};

static ThreadPool pool;
constexpr static int num_threads = 4;

void parallel_for(int begin, int end, const std::function<void(int)>& f) {
    int block_size = (end - begin) / num_threads;
    int block_begin = begin;
    int block_end = begin + block_size - 1;
    for (int t = 0; t < num_threads; t++) {
        pool.add([=](){ for (int i = block_begin; i <= block_end; i++) f(i);});
        block_begin += block_size;
        block_end = t == num_threads - 2 ? end : block_end + block_size; 
    }
    pool.wait();
}