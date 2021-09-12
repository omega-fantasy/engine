#ifndef AUDIO_H
#define AUDIO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <filesystem>
#include <unordered_map>
#include <string>

class AudioPlayer {
   public:
       AudioPlayer() {
           Mix_Init(MIX_INIT_OGG);
           Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
           Mix_Volume(-1, 64);
       }

        void play_music(const std::string& name, int volume) { 
            Mix_VolumeMusic(volume);
            Mix_PlayMusic(music[name], -1); 
        }
        void play_sound(const std::string& name) { Mix_PlayChannel(-1, sounds[name], 0); }
        void add_music_folder(const std::string folder) { add_folder(folder, true); }
        void add_sound_folder(const std::string folder) { add_folder(folder, false); }

   private:
       std::unordered_map<std::string, Mix_Chunk*> sounds;
       std::unordered_map<std::string, Mix_Music*> music;

       void add_folder(const std::string folder, bool is_music) {
           std::string filename, filepath;
           for (auto& file : std::filesystem::recursive_directory_iterator(folder)) {
               filename = file.path().filename().string();
               filepath = file.path().string();
               if (filename.find(".") != std::string::npos) {
                   std::string name = filename.substr(0, filename.find("."));
                   std::string type = filename.substr(filename.find(".") + 1);
                   if (type == "wav") {
                       if (is_music) {
                           music[name] = Mix_LoadMUS(filepath.c_str());
                       } else {
                           sounds[name] = Mix_LoadWAV(filepath.c_str());
                       }
                   }
               }
           }
       }
};

#endif