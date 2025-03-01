#include "../jlo.h"
#include "GL/glx.h"
#include "math.h"
#include "system.h"
#include "components.h"
#include <iostream>
#include <unordered_map>
extern Global gl; //access to gl.state for menu state render
extern std::unordered_map<std::string,std::shared_ptr<Texture>> textures;
EntitySystem::~EntitySystem() = default;
void EntitySystem::update([[maybe_unused]] Scene& scene, [[maybe_unused]] float dt) {
    
}
PhysicsSystem::PhysicsSystem() = default;

void PhysicsSystem::update(Scene& scene, float dt) {
    if (gl.state == Global::MENU || gl.state == Global::CONTROLS) {
        std::cout << "PhysicsSys: skip rendering because gl.state is menu/controls" << endl;
        return;  
    }
    std::vector<Entity*> entities = scene.queryEntities<Transform, Physics>();
    for (auto ptr : entities) {
        Transform* tc = scene.getComponent<Transform>(ptr);
        Physics* pc = scene.getComponent<Physics>(ptr);
        if (pc->physics) {
            pc->velocity += pc->acceleration * dt;
            tc->pos += pc->velocity * dt;
        }
    }
}

RenderSystem::RenderSystem() = default;
void RenderSystem::update(Scene& scene, float dt) {
    if (gl.state == Global::MENU || gl.state == Global::CONTROLS){
        std::cout << "renderSystem: skipped cuz gl.state menu" <<endl;
        return;
    }
    //clearing in main loop asteroids.
   // glClear(GL_COLOR_BUFFER_BIT);
    std::vector<Entity*> entities = scene.queryEntities<Transform, Sprite>();

    for (auto e : entities) {
        Transform* tc = scene.getComponent<Transform>(e);
        EntityID id = e->getId();
        Sprite* sc = scene.getComponent<Sprite>(e);

        if (sc->c_anim == "") {
            continue;
        }
        
        std::shared_ptr<Animation> a_ptr = sc->animations[sc->c_anim];
        if (a_ptr == nullptr) {
            continue;
        }
        if (a_ptr->frame++ == 8) {
            a_ptr->frame = 0;
        } 
        AnimationFrame frame = a_ptr->frames[a_ptr->frame];
        std::shared_ptr<Texture> t = textures[a_ptr->texture_key()];
        if (t == nullptr) {
            continue;
        }

        glBindTexture(GL_TEXTURE_2D, *t->texture);
        int sw = a_ptr->sprite_width();
        int sh = a_ptr->sprite_height();
        int ix = frame.f_idx[0];
        int iy = frame.f_idx[1];
        int columns = 8;
        int rows = 1;
        float fx = (float) ix / columns;
        float fy = (float) iy / rows;
        float xo = (float) 1 / columns;
        float xy = (float) 1 / rows;
        
        
        glBegin(GL_QUADS);
            if (scene.hasComponents<Physics>(e)) {
                Physics* pc = scene.getComponent<Physics>(e);
                if (pc->velocity[0] < 0) {
                    glTexCoord2f(fx, fy + xy);      glVertex2i(tc->pos[0] - sw, tc->pos[1] - sh);
                    glTexCoord2f(fx, fy);           glVertex2i(tc->pos[0] - sw, tc->pos[1] + sh);
                    glTexCoord2f(fx + xo, fy);      glVertex2i(tc->pos[0] + sw, tc->pos[1] + sh);
                    glTexCoord2f(fx + xo, fy + xy); glVertex2i(tc->pos[0] + sw, tc->pos[1] - sh);
                } else {
                    glTexCoord2f(fx + xo, fy + xy); glVertex2i(tc->pos[0] - sw, tc->pos[1] - sh);
                    glTexCoord2f(fx + xo, fy);      glVertex2i(tc->pos[0] - sw, tc->pos[1] + sh);
                    glTexCoord2f(fx, fy);           glVertex2i(tc->pos[0] + sw, tc->pos[1] + sh);
                    glTexCoord2f(fx, fy + xy);      glVertex2i(tc->pos[0] + sw, tc->pos[1] - sh);
                }
            }
            
           
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_ALPHA_TEST);
    }
}

void EntitySystemManager::update(Scene& scene, float dt) {
    for (auto& p_system : systems) {
        p_system->update(scene, dt); 
    }
}
