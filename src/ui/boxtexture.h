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
        unsigned border_width = 4;
        for (int y = 0; y < s.h; y++) {
            for (int x = 0; x < s.w; x++) {
                Point p(x, y);
                Color* current = pixels + x + y * s.w;
                Point pts[4] = {{x, 0}, {(int)s.w-1, y}, {0, y}, {x, (int)s.h-1}};
                short d1 = 0.25 * border_width;
                short d2 = 0.75 * border_width;
                short d3 = border_width;
                if (p.distance(pts[0]) < d1 || p.distance(pts[1]) < d1 || p.distance(pts[2]) < d1 || p.distance(pts[3]) < d1) {
                    *current = c_border - darken;
                } else if (p.distance(pts[0]) < d2 || p.distance(pts[1]) < d2 || p.distance(pts[2]) < d2 || p.distance(pts[3]) < d2) {
                    *current = c_border;
                } else if (p.distance(pts[0]) < d3 || p.distance(pts[1]) < d3 || p.distance(pts[2]) < d3 || p.distance(pts[3]) < d3) {
                    *current = c_border - darken;
                } else {
                    double perc1 = 1 - (double)p1.distance(p) / max_dist;
                    double perc2 = 1 - (double)p2.distance(p) / max_dist;
                    current->red = perc1 * color1.red + perc2 * color2.red;
                    current->green = perc1 * color1.green + perc2 * color2.green;
                    current->blue = perc1 * color1.blue + perc2 * color2.blue;
                    current->alpha = 255;
                }
            }
        }
    }
};

#endif
