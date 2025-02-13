#pragma once

#include "../jlo.h"
#include <vector>
#include <memory>
class EntitySystem
{
    public:
        virtual void update(float dt);
};

class PhysicsSystem : public EntitySystem {
    private:
};

class EntitySystemManager
{
    private:
        std::vector<std::unique_ptr<EntitySystem>> systems;
    public:
        void update(Scene& scene, float dt);
        template <typename T>
        T* registerSystem();
        template <typename T>
        bool hasSystem();
};

#include "../implementation/ecs/system.tpp"