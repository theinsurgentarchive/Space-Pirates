#pragma once

#include "ecs/components.h"
#include "asteroids/fonts.h"
#include "asteroids/defs.h"
#include <cstdint>
#include <string>
#include <bitset>
#include <memory>
#include <queue>
#include <vector>
#include <GL/glx.h>
class Image {
	public:
		int width, height;
		unsigned char *data;
		~Image() { delete [] data; }
		Image(const char *fname) { 
			if (fname[0] == '\0')
				return;
			//printf("fname **%s**\n", fname);
			int ppmFlag = 0;
			char name[40];
			strcpy(name, fname);
			int slen = strlen(name);
			char ppmname[80];
			if (strncmp(name+(slen-4), ".ppm", 4) == 0)
				ppmFlag = 1;
			if (ppmFlag) {
				strcpy(ppmname, name);
			} else {
				name[slen-4] = '\0';
				//printf("name **%s**\n", name);
				sprintf(ppmname,"%s.ppm", name);
				//printf("ppmname **%s**\n", ppmname);
				char ts[100];
				//system("convert eball.jpg eball.ppm");
				sprintf(ts, "convert %s %s", fname, ppmname);
				system(ts);
			}
			//sprintf(ts, "%s", name);
			//printf("read ppm **%s**\n", ppmname); fflush(stdout);
			FILE *fpi = fopen(ppmname, "r");
			if (fpi) {
				char line[200];
				fgets(line, 200, fpi);
				fgets(line, 200, fpi);
				//skip comments and blank lines
				while (line[0] == '#' || strlen(line) < 2)
					fgets(line, 200, fpi);
				sscanf(line, "%i %i", &width, &height);
				fgets(line, 200, fpi);
				//get pixel data
				int n = width * height * 3;			
				data = new unsigned char[n];			
				for (int i=0; i<n; i++)
					data[i] = fgetc(fpi); 
				fclose(fpi);
			} else {
				printf("ERROR opening image: %s\n",ppmname);
				exit(0);
			}
			if (!ppmFlag)
				unlink(ppmname);
		}
};
void show_jlo(Rect* r);
constexpr uint16_t MAX_ENTITY_COMPONENTS {32};
typedef std::bitset<MAX_ENTITY_COMPONENTS> ComponentMask;
typedef uint32_t EntityID;
extern unsigned char *buildAlphaData(Image *img);
extern uint16_t counter;
template <typename T>
uint16_t getComponentId() 
{
    static uint16_t cid = counter++;
    return cid;
}

class Entity 
{
    private:
        // Unique identifier for this entity
        EntityID id;
        // Bitfield can be queried to show components that are enabled for this entity
        ComponentMask mask;
    public:
        Entity(EntityID i, ComponentMask m);
        EntityID getId() const;
        ComponentMask& getMask();
};

/*
    An array for a singular type of component
    Credit: https://www.david-colson.com/2020/02/09/making-a-simple-ecs.html
*/
class ComponentPool
{
    private:
        uint16_t element_size;
        // Pointer to byte array for this component type
        std::unique_ptr<char[]> p_data;
    public:
        ComponentPool(uint16_t e);
        ~ComponentPool();

        // Returns a pointer to the data at 'idx'
        void* get(uint16_t idx);
};

/*
    Container to hold entity state/components
    Credit: https://austinmorlan.com/posts/entity_component_system/#the-component
*/
class Scene
{
    private:
        uint16_t max_entities;
        std::vector<Entity> free_entities;
        std::vector<Entity> checked_out;
        std::vector<std::unique_ptr<ComponentPool>> pools;
    public:
        Scene(uint16_t m);
        
        /*
            Creates an entity for this scene.
            
            @returns entity
        */
        Entity* createEntity();

        /*
            Removes the entity from this scene.
        */
        void destroyEntity(Entity* ptr);

        /*
            Adds a component 'T' to 'entity'. 
            If the component has already been added to 'entity',
            this will throw a std::runtime_error.
            
            @param entity being checked
            @returns pointer to 'T' 
        */
        template <typename T>
        T* addComponent(Entity* ptr);
        
        /*
            Returns a pointer to component 'T'.
            If the entity does not have the component, this will return a nullptr.
            This is an implementation detail.

            @params entity being checked
            @returns pointer to 'T'
        */
        template <typename T>
        T* getComponent(Entity* ptr);

        /*
            Checks whether an entity has a component type 'T'

            @param entity being checked
            @returns bool whether the entity has the component
        */
        template <typename... T>
        bool hasComponents(Entity* ptr);

        /*
            Returns the count of active entities in the scene
            
            @returns count of active entities 
        */
        uint16_t getActiveEntities() const;
        
        /* 
            Returns the count of components that belong to 'entity'
            
            @param entity being checked
            @returns count of components
        */
        uint16_t getComponentCount(Entity* ptr) const;

        /*
            Returns the list of entities that match 'T...'

            @returns a list of pointers to entities
        */
        template <typename ... T>
        std::vector<Entity*> queryEntities();

        void displayMatrix();
};

class TextureInfo {
    public:
        Image img;
        std::shared_ptr<GLuint> texture;
        TextureInfo(const char* f_name);
};

class TextureLoader
{
    private:
        const char* _fname;
        void _find_texture_names(std::vector<std::string>& t_file_names);
    public:
        TextureLoader(const char* f);
        void load_textures(std::vector<std::unique_ptr<TextureInfo>>& textures);
        
};

#include "implementation/scene.tpp"
