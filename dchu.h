#pragma once
#include "jlo.h"

//Define Alias for ecs Components
#define COMBAT ecs::Combat
#define NAVIGATE ecs::Navigate

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

//Enemy Class
class Enemies
{
    private:
        //Store Entity
        std::vector<ecs::Entity*> entities;

        //Sprite Sheet for Set of Enemies
        string sprite_sheet;

        //Origin Spawning Position of Grouping
        v2f origin;

        //Amount of Entities Spawned
        u16 amount;

        //Damage Modifier
        float damage;

        //Health Modifier
        float health;

        //Wait for n Seconds of Time
        u16 timer;
    public:
        u16 getTimer();
        void setTimer(u16);

        //Constructor
        Enemies();
        Enemies(
            v2f, 
            u16 number = 1, u16 delay = 5,
            float hp = 1.0f, float dmg = 1.0f,
            string ssheet = "player-front"
        );
        Enemies(EnemyT);

        //Destructor
        ~Enemies();

};

//Entity Component Systems
namespace ecs
{
    //Damage Component
    struct Combat
    {
        float damage;
        bool allow {true};
    }

    //Pathing Component
    struct Navigate
    {
        v2u start, goal;
        Node* next_node {nullptr};
    };

    //Health / Damage System
    class HP_DMGSystem : public System<Transform,Health,Combat>
    {
        public:
            HP_DMGSystem(ECS& ecs, float sample_delta);
            void update(float dt) override;
    }

    //Entity Pathfinding System
    class PathSystem : public System<Transform,Navigate>
    {
        public:
            PathSystem(ECS& ecs, float sample_delta);
            void update(float dt) override;
    }
}