#include "jlo.h"
#include <cmath>
#include <memory>
Vec2::Vec2() : pos{0,0} {}

Vec2::Vec2(float x, float y) : pos{x,y} {}

float Vec2::getX() {
    return pos[0];
}

float Vec2::getY() {
    return pos[1];
}

void Vec2::setX(float x) {
    pos[0] = x;
}

void Vec2::setY(float y) {
    pos[1] = y;
}

Vec2 Vec2::operator-() const {
    return Vec2(-pos[0],-pos[1]);
}

float Vec2::operator[] (int i) const {
    return pos[i];
}

float& Vec2::operator[] (int i) {
    return pos[i];
}

Vec2& Vec2::operator+= (Vec2& v) {
    pos[0] += v.pos[0];
    pos[1] += v.pos[1];
    return *this;
}

Vec2& Vec2::operator*= (float scale) {
    pos[0] *= scale;
    pos[1] *= scale;
    return *this;
}

float Vec2::length() const {
    return std::sqrt(length_squared());
}
float Vec2::length_squared() const {
    return pos[0] * pos[0] + pos[1] * pos[1];
}


//Start - Entity
Entity::Entity(EntityID i, ComponentMask m) : id{i},mask{m} {}

EntityID Entity::getId() const 
{
    return id;
}

ComponentMask& Entity::getMask() 
{
    return mask;
}
//End - Entity

//Start - Component Pool
ComponentPool::ComponentPool(uint16_t e) : element_size{e}, p_data{nullptr} 
{
    p_data = std::make_unique<char[]>(element_size);
}

ComponentPool::~ComponentPool() = default;

void* ComponentPool::get(uint16_t idx) 
{
    return p_data.get() + idx * element_size;
}
//End - Component Pool

//Start - Scene
Scene::Scene(uint16_t m) : max_entities{m} 
{
    for (uint16_t i ; i < m ; i++) {
        entities.push({i,ComponentMask()});
    }
}

Entity Scene::createEntity() 
{
    Entity entity = entities.front();
    entities.pop();
    living_entities++;
    return entity;
}

void Scene::destroyEntity(Entity entity) 
{
    entity.getMask().reset();
    entities.push(entity);
    living_entities--;
}
//End - Scene
