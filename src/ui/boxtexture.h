#ifndef BOXTEXTURE_H
#define BOXTEXTURE_H

#include "engine/engine.h"
#include "engine/texture.h"

class BoxTexture : public Texture {
    public:
    BoxTexture(Size s, Color color1, Color color2, Color c_border): Texture(color1, s) {
        Color* pixels = (Color*)pixels_og;
        Point p1(0, 0);
        Point p2(s.w-1, s.h-1);
        int max_dist = s.w + s.h;
        unsigned char darken = 80;
        for (int y = 0; y < s.h; y++) {
            for (int x = 0; x < s.w; x++) {
                Point p(x, y);
                Color* current = pixels + x + y * s.w;
                if (x == 0 || x == 3 || x == s.w - 4 || x == s.w - 1 ||
                    y == 0 || y == 3 || y == s.h - 4 || y == s.h - 1 ) 
                {
                    *current = c_border - darken; 
                } else if (x == 1 || x == 2 || x == s.w - 2 || x == s.w - 3 ||
                           y == 1 || y == 2 || y == s.h - 2 || y == s.h - 3 ) 
                {
                    *current = c_border;
                } else {
                    double perc1 = 1 - (double)p1.distance(p) / max_dist;
                    double perc2 = 1 - (double)p2.distance(p) / max_dist;
                    current->red = perc1 * color1.red + perc2 * color2.red;
                    current->green = perc1 * color1.green + perc2 * color2.green;
                    current->blue = perc1 * color1.blue + perc2 * color2.blue;
                    current->alpha = perc1 * color1.alpha + perc2 * color2.alpha;
                }
            }
        }
    }
};

#endif
