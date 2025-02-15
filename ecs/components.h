class Transform
{
    public:
        Vec2 pos, scale;
        float rotation;
        Transform();
        Direction direction();
};

struct Sprite
{
    uint16_t width, height;
    std::string texture;
};

struct Physics
{
    Vec2 velocity;
    Vec2 acceleration;
};

class Health
{
    public:
        float hp, maxHp;
        Health();
        Health(float h, float m);
        float percent();
};

class AABBHitbox
{
    public:
        Vec2 corners[2];
        AABBHitbox(float centerX, float centerY, float radius);
        bool collided(AABBHitbox hitbox);
};
