#pragma once
#include "jlo.h"

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

//Can-Be-Displayed Check Function
bool isDisplayable(ecs::Entity*);

//A* Pathfinding Algorithm
//(INTEGRATION IN PROCESS)
class Node
{
    private:
        //World Position
        v2f world_pos;

        //Local Position
        v2u local_pos;

        //Hitbox Scale
        v2f scale;
    public:

        //Variables
        bool obstacle, visited;
        float local_dist, global_dist;
        std::vector<Node*> neighbors;
        
        //Node Directly Preceding Current Node
        Node* parent;
        
        //Constructor
        Node();
        Node(bool);

        //Functions
        v2f getWorld();
        void setWorld(v2f);
        v2u getLocal();
        void setLocal(v2u);
};

//AStarGrid of Node Elements, Used in A* Search
class AStar
{
    private:
        //AStar's X & Y Axis Size
        v2u grid_size;

        //The Position in The World that The Grid is Generated From.
        v2f origin_pos;
    public:
        //Dynamic Node Grid
        std::vector<std::vector<Node>> node_grid;

        //Constructor
        AStar();
        AStar(v2f, v2u, v2f);
        AStar(uint16_t, uint16_t);

        //Sets a Node to an Obstacle in A*
        void toggleObstacle(uint16_t, uint16_t);

        //Get The Node Grid's Size
        v2u size();

        //Retrieves a Pointer to The Node
        Node* getNode(uint16_t, uint16_t);

        //Initializes The Node Grid
        void initGrid(v2f tile_dim = {1.0f, 1.0f});

        //Generate All Neighbors for Each Node
        void genNeighbors();

        //Check If The Passed Node has Neighbors
        bool hasNeighbors(Node*);

        //A* Search Algorithm
        Node* aStar(uint16_t[2], uint16_t[2]);

        //Node Refresh
        void resetNodes();

        //Calculates The Distance From One Node to The Next
        float distance(Node*, Node*);

        //Generates Biased Data Based On Two Given Input Nodes
        float heuristics(Node*, Node*);
};
