//balrowhany
#include <iostream>
#include "balrowhany.h"


void show_balrowhany()
{
    std::cout << "Balrowhany \n";

}
void show_balrowhany(Rect* r)
{
    ggprint8b(r, 16, 0x00ff0000, "Balrowhany");
}