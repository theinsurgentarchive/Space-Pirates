#include "dchu.h"

/*color Definitions*/
color::color()
{
    red = 0;
    blue = 0;
    green = 0;
}

color::color(uint8_t new_red, uint8_t new_blue, uint8_t new_green)
{
    red = new_red;
    blue = new_blue;
    green = new_green;
}

uint8_t color::getRed()
{
    return red;
}

uint8_t color::getBlue()
{
    return blue;
}

uint8_t color::getGreen()
{
    return green;
}

void color::setColors(uint8_t new_red, uint8_t new_blue, uint8_t new_green)
{
    setRed(new_red);
    setBlue(new_blue);
    setGreen(new_green);
}
void color::setColors(color RGB)
{
    setRed(RGB.getRed());
    setBlue(RGB.getBlue());
    setGreen(RGB.getGreen());
}

void color::setRed(uint8_t new_red)
{
   red = new_red;
}

void color::setBlue(uint8_t new_blue)
{
   blue = new_blue;
}

void color::setGreen(uint8_t new_green)
{
   green = new_green;
}
/*End color Definitions*/

/*planetBase Definitions*/
planetBase::planetBase(EntityID id, ComponentMask mask) : Entity(id, mask)
{
    is_exhausted = false;
    rgb.setColors(0, 0, 0);
}

planetBase::planetBase(EntityID id, ComponentMask mask, std::string new_type,
Sprite new_img, color RGB, Transform PSR) : Entity(id, mask)
{
    setSprite(new_img);
    setTransform(PSR);
    is_exhausted = false;
    rgb.setColors(RGB);
}

void planetBase::setSprite(Sprite new_img)
{
    img.height = new_img.height;
    img.width = new_img.width;
    img.texture = new_img.texture;
}

void planetBase::setTransform(Transform PSR)
{
    psr.pos.setX(PSR.pos.getX());
    psr.pos.setY(PSR.pos.getY());
    psr.scale.setX(PSR.scale.getX());
    psr.scale.setY(PSR.scale.getY());
    psr.rotation = PSR.rotation;
}
std::string planetBase::getType()
{
    return type;
}

Vec2 planetBase::getPosition()
{
    return psr.pos;
}

void planetBase::exhausted()
{
    is_exhausted = true;
    rgb.setColors(128, 128, 128);
}


/*End planetBase Definitions*/