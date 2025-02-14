#include "system.h"

PhysicsSystem::PhysicsSystem() = default;
void PhysicsSystem::update(Scene& scene, float dt) {
    std::vector<Entity*> entities = scene.queryEntities<Transform, Velocity>();
    for (auto ptr : entities) {
        if (scene.hasComponents<Acceleration>(ptr)) {
            Acceleration* ac = scene.getComponent<Acceleration>(ptr);
            Velocity* vc = scene.getComponent<Velocity>(ptr);
            vc->velocity[0] += ac->acceleration[0] * dt;
            vc->velocity[1] += ac->acceleration[1] * dt;
        }

        Transform* tc = scene.getComponent<Transform>(ptr);
        Velocity* vc = scene.getComponent<Velocity>(ptr);
        tc->pos[0] += vc->velocity[0] * dt;
        tc->pos[1] += vc->velocity[1] * dt;
    }
}

void EntitySystemManager::update(Scene& scene, float dt) {
    for (auto& p_system : systems) {
        if (p_system) { 
            p_system->update(scene, dt); 
        }
    }
}
