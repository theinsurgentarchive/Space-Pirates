#include "system.h"

PhysicsSystem::PhysicsSystem() = default;

void PhysicsSystem::update(Scene& scene, float dt) {
    std::vector<Entity*> entities = scene.queryEntities<Transform, Physics>();
    for (auto ptr : entities) {
        Transform* tc = scene.getComponent<Transform>(ptr);
        Physics* pc = scene.getComponent<Physics>(ptr);
        pc->velocity += pc->acceleration * dt;
        tc->pos += pc->velocity * dt;
    }
}

void EntitySystemManager::update(Scene& scene, float dt) {
    for (auto& p_system : systems) {
        p_system->update(scene, dt); 
    }
}

//@theinsurgentarchive
void InventorySystem::update(Scene& scene, float dt)
{
}