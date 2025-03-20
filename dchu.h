#pragma once
#include "jlo.h"
#include "fonts.h"

//Entity Components
struct oxygen_resource
{
	//Oxygen Level
	float oxygen;
	//Max O2 Level
	float max;
    //Is O2 Depleted?
	bool depleted;
};

struct fuel_resource
{
	//Fuel Level
	float fuel;
	//Max Fuel Level
	float max;
    //Is Fuel Depleted?
	bool depleted;
};

//A* Pathfinding Algorithm
//(NEEDS TO BE UPDATED TO MATCH AND INTEGRATE WITH PROJECT)
class Node
{
    public:
        //Node Conditionals
        bool obstacle, visited;
        
        //Node Position
        uint16_t x, y;
        
        //Node Distance from Initial Node
        float local_dist;

        //Node Distance from Goal Node
        float global_dist;
        
        //Vector Matrix of Neighboring Nodes
        std::vector<Node*> neighbors;
        
        //Node Directly Preceding Current Node
        Node* parent;
        
        //Constructor
        Node();
        Node(bool);
};

//AStarGrid of Node Elements, Used in A* Search
class AStarGrid 
{
    private:
        //AStarGrid's X & Y Axis Size
        uint16_t grid_size[2];
    public:
        //Dynamic Node Grid
        std::vector<std::vector<Node>> node_grid;

        //Constructor
        AStarGrid();
        AStarGrid(Node**);
        AStarGrid(uint16_t, uint16_t);

        //Sets a Node to an Obstacle in A*
        void setObstacle(uint16_t, uint16_t);

        //Get The Node Grid's Size
        uint16_t getSizeX();
        uint16_t getSizeY();

        //Retrieves a Pointer to The Node
        Node* getNode(uint16_t, uint16_t);

        //Initializes The Node Grid
        void initGrid();

        //Generate All Neighbors for Each Node
        void genNeighbors();

        //Check If The Passed Node has Neighbors
        bool hasNeighbors(Node*);

        //A* Search Algorithm
        void aStar(uint16_t[2], uint16_t[2]);

        //Node Refresh
        void resetNodes();

        //Calculates The Distance From One Node to The Next
        float distance(Node*, Node*);

        //Generates Biased Data Based On Two Given Input Nodes
        float heuristics(Node*, Node*);
};

/*
struct item
{
    std::string item_name;
    uint16_t slot_num;
    //TBD
};

//Declaration of Inventory Management
class Inventory
{
    private:
        void initStoreVolume(uint16_t, uint16_t);
        bool full;
        item** storage;
        Vec2<uint16_t> inv_size;
        public:
        //Constructor
        Inventory();

        //Destructor
        ~Inventory();

        //Setters
        void addItem(item);
        void useItemSlot(); //To Be Completed

        //Getters:
        void getInventory();
        item returnItemSlot(uint16_t, uint16_t);
};

//ECS: Inventory Management System
namespace ecs
{
    class InventorySystem : public System
    {
        public:
            InventorySystem();
            void update(float dt) override;
    };
}
*/