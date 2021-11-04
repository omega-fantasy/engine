#ifndef HUD_H
#define HUD_H

#include "engine/engine.h"
#include "engine/ui.h"
class MiniMap;

class HUD : public Composite {
    public:
        HUD(Size s);
        void init();
        void change_layout(const std::vector<std::pair<Composite*, Point>>& new_layout, bool back=true);
        std::vector<std::pair<Composite*, Point>> create_standard_layout();
    private:
        MiniMap* mini_map;
};

#endif
