#pragma once

#include "../jlo.h"
#include <vector>
#include <memory>
#include <iostream>
class EntitySystem
{
    public:
        virtual ~EntitySystem() = default;
        virtual void update(Scene& scene, float dt);
};

class PhysicsSystem : public EntitySystem {
    public:
        PhysicsSystem();
        void update(Scene& scene, float dt) override;
};

class EntitySystemManager
{
    private:
        std::vector<std::shared_ptr<EntitySystem>> systems;
    public:
        void update(Scene& scene, float dt);
        template <typename T>
        std::weak_ptr<T> registerSystem();
        template <typename T>
        bool hasSystem();
};

//@theinsurgentarchive
class InventorySystem : public EntitySystem
{
    public:
    void update(Scene&, float);
}

#include "../implementation/ecs/system.tpp"