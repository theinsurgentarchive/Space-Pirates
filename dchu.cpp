#include "dchu.h"

/*planetBase Definitions*/

/*
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

*/
/*End planetBase Definitions*/