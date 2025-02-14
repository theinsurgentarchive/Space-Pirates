#include "jlo.h"
#include <cmath>
#include <memory>
#include <algorithm>

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
Scene::Scene(uint16_t m) : max_entities{m} {
    for (auto i {0} ; i < m; i++) {
        free_entities.push_back({static_cast<EntityID>(i),ComponentMask()});
    }
}

Entity* Scene::createEntity() 
{
    if (Scene::getActiveEntities() >= max_entities) {
        throw new std::runtime_error("You are over the max entities");
    }
    Entity entity = std::move(free_entities.front());
    free_entities.erase(free_entities.begin());
    checked_out.push_back(std::move(entity));
    return &checked_out.back();
}

void Scene::destroyEntity(Entity* ptr) 
{
    auto it = std::find_if(checked_out.begin(),checked_out.end(),[ptr](const Entity& e) {return e.getId() == ptr->getId();});
    
    if (it != checked_out.end()) {
        checked_out.erase(it);
        free_entities.push_back(*it);
    }
}

uint16_t Scene::getActiveEntities() const
{
    return static_cast<uint16_t>(checked_out.size());
};

uint16_t Scene::getComponentCount(Entity* ptr) const
{   
    ComponentMask mask = ptr->getMask();
    return mask.count();
};
//End - Scene

//Start - Transform
Transform::Transform() : pos{0,0},scale{0,0},rotation{0} {}
//End - Transform
//Start - Health
Health::Health() : hp{0},maxHp{0} {}

Health::Health(float h, float m) : hp{h},maxHp{m} {}

float Health::percent() {
    return hp / maxHp;
}
//End - Health

//Start - AABBHitbox
AABBHitbox::AABBHitbox(float centerX, float centerY, float radius) {
    corners[0] = Vec2(centerX - radius, centerY - radius);
    corners[1] = Vec2(centerX + radius, centerX + radius);
}
//TODO: implement collided 

//End - AABBHitbox
