#pragma once
#include "jlo.h"
#include "fonts.h"

//Credit Print Function
void ShowDChu(Rect*);

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

struct color
{
    uint8_t red;
    uint8_t blue;
    uint8_t green;
    uint8_t alpha;
};

struct item
{
    std::string item_name;
    uint16_t slot_num;
    /*TBD*/
};

//Declaration of Inventory Management
class Inventory
{
    private:
        void initStoreVolume(int, int);
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
        void useItemSlot(/*TBD*/);

        //Getters:
        void getInventory();
        item returnItemSlot(int, int);
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

//A* Pathfinding Algorithm
//(NEEDS TO BE UPDATED TO MATCH AND INTEGRATE WITH PROJECT)
/*
class Node
{
    public:
        //Node Conditionals
        bool obstacle, visited;
        
        //Node Position
        int x, y;
        
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

//Grid of Node Elements, Used in A* Search
class Grid 
{
    private:
        //Grid's X & Y Axis Size
        int grid_size[2];
    public:
        //Dynamic Node Grid
        Node** node_grid;

        //Constructor
        Grid();
        Grid(int, int);

        //Function
        void setObstacle(int, int);
        int getSizeX();
        int getSizeY();
        Node* getNode(int, int);
        void initGrid();
        void genNeighbors();
        bool hasNeighbors(Node*);
        
        //Destructor
        ~Grid();
};

//A* Search Algorithm
void aStar(Grid*, int[2], int[2]);

//Node Refresh
void resetNodes(Grid*);

//Calculate The Distance From One Node to The Next
float distance(Node*, Node*);

//Calls distance() (May be Deprecated in The Future Due to Redundancy)
float heuristics(Node*, Node*);
*/
