#ifndef HUD_H
#define HUD_H

#include "engine/engine.h"
#include "engine/ui.h"

class HUD : public Composite {
    public:
        HUD(Size s);
        void init();
};

#endif
