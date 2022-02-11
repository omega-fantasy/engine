#ifndef TEXTURE_H
#define TEXTURE_H

#include "engine.h"

class Texture {
    public:
        using ID = short;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Size s, Color* pixels);
        Texture(Color color, Size s);
        virtual ~Texture();
        static inline int zoom2idx(float zoom) {
            if (zoom == 0.125f) return 0;
            if (zoom == 0.25f) return 1;
            if (zoom == 0.5f) return 2;
            if (zoom == 1.0f) return 3;
            if (zoom == 2.0f) return 4;
            return 5;
        }
        inline Color* pixels(float zoom = 1.0f) { return pixel_map[zoom2idx(zoom)]; }
        Size size(float zoom = 1) { return m_size * zoom; }
        bool transparent() { return hasTransparency; }
        void set_transparent(bool b) { hasTransparency = b; }   
        ID id() { return m_id; }

    protected:
        friend class TextureManager;
        friend class Tilemap;
        ID m_id = 0;
        Size m_size;
        Color* pixel_map[6] = {nullptr};
        bool hasTransparency = false;
        Color* load_scaled(float zoom);
};

class TextureManager {
    public:
        TextureManager();
        ~TextureManager();
        void reinit();
        void register_texture(const std::string& name, Texture* t);
        void add_folder(const std::string& folder);
        inline Texture* get(Texture::ID id) { return id_to_texture[id]; }
        Texture* get(const std::string& name);
        Texture* get(char letter, int size);
        std::string generate_name(const std::string& command, const std::vector<std::string>& params);
        void set_font(const std::string& path) { fontpath = path; }

    private:
        friend class Tilemap;
        std::string fontpath;
        constexpr static char DELIMITER = '$';
        Texture* id_to_texture[15000] = {0};
        std::map<std::string, Texture*> name_to_texture;
        std::vector<std::vector<Texture*>> letter_to_texture;
        Texture::ID currentID = 1;
        Texture* generate_texture(const std::string& name);
        Texture* get_blended(const std::string& base, const std::string& top, const std::string& right, const std::string& bottom, const std::string& left);
        Texture* get_alpha_bordered(const std::string& basename, const std::string& postfix);
        void init_letters(int height, Color color = {255, 255, 255});
};

#endif
