#include <iostream>
#include "fonts.h"
#include "mchitorog.h"


void show_mchitorog(Rect *r) {
    ggprint8b(r, 16, 0x00ff0000, "Developer - Mihail Chitorog");
}
