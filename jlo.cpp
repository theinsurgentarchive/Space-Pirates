#include "jlo.h"
#include <memory>
#include <algorithm>
#include <iostream>
#include <dirent.h>

//written by gordon

void show_jlo(Rect* r) {
    ggprint8b(r, 16, 0x00ffffff, "Justin Lo");
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

//Start - TextureInfo
TextureInfo::TextureInfo(const char *f) : img{f}, texture {nullptr} {
    texture = std::make_shared<GLuint>();
}
//Start - TextureLoader
TextureLoader::TextureLoader(const char* f) : _fname {f} {}

void TextureLoader::_find_texture_names(std::vector<std::string>& t_file_names)
{
    DIR* dir = opendir(_fname);
	if (dir == nullptr) {
		std::cerr << "missing textures folder at: " << _fname << '\n';
		exit(1);
		return;
	}

    struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
			if (entry->d_type == DT_REG) {
			std::string s { entry->d_name };
			auto idx = s.find_last_of(".");
			if (idx != std::string::npos) {
				std::string f_ext = s.substr(idx + 1);
				if (f_ext == "jpg" || f_ext == "png") {
					t_file_names.push_back(s);
				}
			}
		}
	}
	closedir(dir);
}

void TextureLoader::load_textures(std::vector<std::unique_ptr<TextureInfo>>& textures)
{
    std::vector<std::string> t_file_names;
    _find_texture_names(t_file_names);

    for (auto tfn : t_file_names) {
        std::clog << "loading texture: " << tfn << '\n';
        char buf[128];
        snprintf(buf, sizeof(buf), "%s/%s", _fname, tfn.c_str());

        std::unique_ptr<TextureInfo> pt = std::make_unique<TextureInfo>(buf);
        //Start - Gordon
        glGenTextures(1, pt->texture.get());
        glBindTexture(GL_TEXTURE_2D,*pt->texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

        unsigned char *data = buildAlphaData(&pt->img);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pt->img.width, pt->img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        free (data);
        //End - Gordon
        textures.push_back(std::move(pt));
    }
}

