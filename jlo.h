#pragma once

#include <cstdint>
#include <string>
#include <bitset>
#include <memory>
#include <queue>
#include <vector>
constexpr uint16_t MAX_COMPONENTS = 16;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;
typedef uint32_t EntityID;

extern uint16_t counter;
template <typename T>
uint16_t getComponentId() 
{
    static uint16_t cid = counter++;
    return cid;
}

class Vec2 
{
    private:
        float pos[2];
        float length_squared() const;
    public:
        Vec2();
        Vec2(float x, float y);
        float getX();
        float getY();
        void setX(float x);
        void setY(float y);
        Vec2 operator-() const;
        float operator[] (int i) const;
        float& operator[] (int i);
        Vec2& operator+= (Vec2& v);
        Vec2& operator*= (float scale);
        float length() const;
};

class Entity 
{
    private:
        EntityID id;
        ComponentMask mask;
    public:
        Entity(EntityID i, ComponentMask m);
        EntityID getId() const;
        ComponentMask& getMask();
};

class ComponentPool
{
    private:
        uint16_t element_size;
        std::unique_ptr<char[]> p_data;
    public:
        ComponentPool(uint16_t e);
        ~ComponentPool();
        void* get(uint16_t idx);
};

class Scene
{
    private:
        uint16_t max_entities;
        std::queue<Entity> entities;
        std::vector<std::unique_ptr<ComponentPool>> pools;
        uint16_t living_entities;
    public:
        Scene(uint16_t m);
        Entity createEntity();
        void destroyEntity(Entity entity);

        template <typename T>
        T* addComponent(Entity entity);
};

struct Transform
{
    Vec2 pos;
    Vec2 scale;
    float rotation;
};

struct Sprite
{
    uint16_t width, height;
    std::string texture;
};

struct Velocity
{
    Vec2 velocity;
};

struct Acceleration
{
    Vec2 acceleration;
};

struct Health
{
    //Hit points
    float hp;
    //Max hit points
    float maxHp;
};
#include "implementation/scene.tpp"
