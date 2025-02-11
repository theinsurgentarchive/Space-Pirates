#ifndef JLO_H
#define JLO_H

#include <cmath>
#include <bitset>
#include <vector>
#include <cstdint>
#include <memory>
const int MAX_COMPONENTS = 16;

extern int s_componentCounter;
template <class T>
int GetId()
{
  static int s_componentId = s_componentCounter++;
  return s_componentId;
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
        uint32_t id;
        std::bitset<MAX_COMPONENTS> mask;
    public:
        Entity(uint32_t i, std::bitset<MAX_COMPONENTS> m);
        uint32_t getId() const;
        std::bitset<MAX_COMPONENTS> getMask();
};

class ComponentPool
{
    private:
        uint32_t elementSize;
        char* pData;
    public:
        ~ComponentPool();
        ComponentPool(uint32_t e);
        inline void* get(int32_t index);
};

class Scene
{
    private:
        std::vector<Entity> entities;
        std::vector<ComponentPool> pools;
    public:
        uint32_t createEntity();
        template<typename T> 
        T* addComponent(uint32_t entityId);
};

struct Transform
{
    Vec2 pos;
    Vec2 scale;
    float rotation;
    Transform(const Vec2& pos = Vec2(0,0), const Vec2&scale = Vec2(0,0), float rotation = 0.0f);
    Transform(float posX = 0.0f, float posY = 0.0f, float scaleX = 0.0f, float scaleY = 0.0f, float rotation = 0.0f);
};

struct Health
{
    int current;
    int max;
    Health(int current = 0, int max = 0);
};

struct Velocity {
    Vec2 velocity;
    Velocity(const Vec2& v = Vec2(0,0));
    Velocity(float x, float y);
};

struct Acceleration {
    Vec2 acceleration;
    Acceleration(const Vec2& a = Vec2(0,0));
    Acceleration(float x, float y);
};
#endif
