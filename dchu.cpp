#include "dchu.h"
#include <iostream>

//Credit Print Function
void show_dchu()
{
    std::cout << "Developer - David Chu\n";
}
void show_dchu(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Developer - David Chu");
}