#pragma once
#include "jlo.h"

//Can The Given Entity be Rendered?
bool canRender(ecs::Entity*);

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

//Enemy Class
class Enemies
{
    private:
        //Store Entity
        std::vector<ecs::Entity*> entities;

        //Damage Modifier
        float damage;

        //Health Modifier
        float hp;

        //Wait for n Seconds of Time
        u16 timer;
    public:
        u16 getTimer();
        void setTimer(u16);

        //Constructor
        Enemies();
        Enemies(
            v2f, 
            u16 delay = 5,
            float hp = 2.0f, float dmg = 1.0f,
            string sprite_sheet = "player-front"
        );
        Enemies(EnemyT);

};

//Entity Component Systems
namespace ecs
{
    //Pathing Data Container
    struct Pathfinding
    {
        v2u start, goal;
        Node* next_node;
    };

    //Entity Pathfinding System
    /*class PathSystem
    {
        public:
            PathSystem(ECS& ecs);
            void update(float dt) override;
    }*/
}