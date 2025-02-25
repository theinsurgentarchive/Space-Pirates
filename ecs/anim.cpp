#include "anim.h"
#include "math.h"
// class Animation {
//     private:
//         std::string _texture_key;
//         uint16_t _frame;
//     public:
//         Animation(std::string texture);
//         std::vector<AnimationFrame> frames;
//         bool loop;
//         void step();
// };

Animation::Animation(std::string t, vec2i s_dim) : 
    _t_key { t } , _s_dim { s_dim }, frame { 0 } {}

Animation::Animation(std::string t, vec2i s_dim, uint16_t f) :
    _t_key { t }, _s_dim { s_dim }, frame { f } {} 

int Animation::sprite_width() const {
    return _s_dim[0];
}

int Animation::sprite_height() const {
    return _s_dim[1];
}

std::string Animation::texture_key() const {
    return _t_key;
}

void Animation::step() {
    if (frame >= frames.size()) {
        frame = 0;
    } else {
        frame++;
    }
}
// class AnimationBuilder {
//     private:
//         std::string _texture_key;
//         uint16_t _frame;
//     public:
//         AnimationBuilder(std::string t, uint16_t f);
//         AnimationBuilder addFrame(AnimationFrame frame);
// }

AnimationBuilder::AnimationBuilder(std::string t, vec2i s_dim, uint16_t f) : 
_t_key {t}, _s_dim { s_dim }, _frame {f} {}

AnimationBuilder& AnimationBuilder::addFrame(int ix, int iy) {
    AnimationFrame f {vec2i{ix,iy}};
    _frames.push_back(f);
    return *this;
}

Animation AnimationBuilder::build() {
    Animation a { _t_key, _s_dim };
    a.frames = _frames;
    return a;
}