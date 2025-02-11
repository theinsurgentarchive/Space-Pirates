#include "jlo.h"
#include <bitset>
#include <cstdint>
#include <vector>

int s_componentCounter = 0;
//Start - Vec2
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
//End - Vec2 



//Start - Entity
Entity::Entity(uint32_t i, std::bitset<MAX_COMPONENTS> m) : id{i}, mask{m} {}

uint32_t Entity::getId() const {
    return id;
}

std::bitset<MAX_COMPONENTS> Entity::getMask() {
    return mask;
}
//End - Entity



//Start - Component Pool
ComponentPool::ComponentPool(uint32_t n) : elementSize(n), pData {nullptr} {}

ComponentPool::~ComponentPool() {
    delete[] pData;
}

inline void* ComponentPool::get(int32_t index) {
    return pData + index * elementSize; 
}
//End - Component Pool



//Start - Scene
uint32_t Scene::createEntity() {
    entities.push_back({ static_cast<uint32_t>(entities.size()), std::bitset<MAX_COMPONENTS>() });
    return entities.back().getId();
}

template <typename T>
T* Scene::addComponent(uint32_t entityId) {
    int cid = GetId<T>();

    if (pools.size() <= cid) {
        pools.resize(cid + 1,nullptr);
    }

    // if (pools[cid] == nullptr) {
    //     pools[cid] = new ComponentPool(static_cast<uint32_t>(sizeof(T)));
    // }

    // T* pComponent = new (pools[cid]->get(id)) T();

    entities[entityId].getMask().set(cid);
    return nullptr;
}
//End - Scene



//Start - Transform
Transform::Transform(const Vec2& pos, const Vec2&scale, float rotation) : pos{pos}, scale{scale}, rotation{rotation} {}

Transform::Transform(float posX, float posY, float scaleX, float scaleY, float rotation) : pos{posX,posY}, scale{scaleX,scaleY}, rotation{rotation} {}
//End - Transform



//Start - Health
Health::Health(int current, int max) : current(current), max(max) {}
//End - Health



//Start - Velocity
Velocity::Velocity(const Vec2& v) : velocity(v) {}
Velocity::Velocity(float x, float y) : velocity(x,y) {}
//End - Velocity



//Start - Acceleration
Acceleration::Acceleration(const Vec2& a) : acceleration(a) {}
Acceleration::Acceleration(float x, float y) : acceleration(x,y) {}
//End - Acceleration

template Transform* Scene::addComponent<Transform> (uint32_t entityId);