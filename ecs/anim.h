#pragma once 

#include "math.h"
#include <string>
#include <vector>
#include <memory>

struct AnimationFrame {
    vec2i f_idx;
};

class Animation {
    private:
        std::string _t_key;
        vec2i _s_dim;
    public:
        uint16_t frame;
        std::vector<AnimationFrame> frames;
        int sprite_width() const;
        int sprite_height() const;
        std::string texture_key() const;
        Animation(std::string t, vec2i s_dim);
        Animation(std::string t, vec2i s_dim, uint16_t f);
        void step();
};

class AnimationBuilder {
    private:
        std::string _t_key;
        vec2i _s_dim;
        uint16_t _frame;
        std::vector<AnimationFrame> _frames;
    public:
        AnimationBuilder(std::string t, vec2i s_dim, uint16_t f);
        AnimationBuilder& addFrame(int ix, int iy);
        Animation build();
};