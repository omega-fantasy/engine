#ifndef AUDIO_H
#define AUDIO_H

#include "util.h"

class AudioPlayer {
   public:
        void play_music(const std::string& name, int) { play_wav(music[name], true); }
        void play_sound(const std::string& name) { play_wav(sounds[name], false); }
        void add_music_folder(const std::string folder) { add_folder(folder, true); }
        void add_sound_folder(const std::string folder) { add_folder(folder, false); }

   private:
       std::map<std::string, AudioHandle> sounds;
       std::map<std::string, AudioHandle> music;

       void add_folder(const std::string folder, bool is_music) {
           for (auto& filepath : filelist(folder, ".wav")) {
               if (is_music) {
                   music[filename(filepath)] = load_wav(filepath, is_music);
               } else {
                   sounds[filename(filepath)] = load_wav(filepath, is_music);
               }
           }
       }
};

#endif
