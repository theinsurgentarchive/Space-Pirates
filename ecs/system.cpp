#include "system.h"

void EntitySystemManager::update(Scene& scene, float dt) {
    for(const auto& p_system : systems) {
        EntitySystem* p = p_system.get();
        p->update(dt);
    }
}