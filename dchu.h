#pragma once
#include "jlo.h"

//Define Alias for ecs Components
#define COMBAT ecs::Combat
#define NAVIGATE ecs::Navigate

namespace ecs
{
    struct Navigate
    {
        v2f dist = {0.0f, 0.0f};
        Node* next {nullptr};
        Node* prev {nullptr};
    };
}

//Can The Given Entity be Rendered?
bool canRender(ecs::Entity*);

//Generate a Randomized Float Number Between 2 Integers
float floatRand(uint16_t, uint16_t);

//A* Pathfinding Algorithm Class
class Node
{
    private:
        //Positional Variables
        v2f world_pos;
        v2u local_pos;
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
        AStar(v2u);
        AStar(u16, u16);

        //Sets a Node to an Obstacle in A*
        void toggleObstacle(u16, u16);

        //Get The Node Grid's Size
        v2u size();

        //Retrieves a Pointer to The Node
        Node* getNode(u16, u16);

        //Initializes The Node Grid
        void initGrid(v2f dim = {1.0f, 1.0f});

        //Generate All Neighbors for Each Node
        void genNeighbors();

        //Check If The Passed Node has Neighbors
        bool hasNeighbors(Node*);

        //A* Search Algorithm
        Node* aStar(v2u, v2u);

        //Node Refresh
        void resetNodes();

        //Calculates The Distance From One Node to The Next
        float distance(Node*, Node*);

        //Generates Biased Data Based On Two Given Input Nodes
        float heuristics(Node*, Node*);
};

//Enemy Types Enumeration
enum EnemyT 
{
    DEFAULT,  //0
    BANDIT,   //1
    ALIEN    //2
};

void initEnemy(ecs::Entity*);
Node* genPath(Node*);