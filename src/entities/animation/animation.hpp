#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>

struct AnimationFrame {
    sf::IntRect frameRegion;
    float duration;
};

class Animation {
public:
    Animation (std::vector<AnimationFrame> frames, bool looping = true);
    
    const AnimationFrame& getFrame (float elapsed) const;
    bool isFinished (float elapsed) const; // only true for non-looping animation

private:
    std::vector<AnimationFrame> frames;
    std::vector<double> checkpoints; // accumlated sum of frames' duration for binary searching
    bool looping;
};

struct AnimationSet { // one animation set corresponds to one character
    std::string textureKey;
    std::unordered_map<std::string, Animation> animations;

    void addAnimation (const std::string &animationKey, const Animation &animation);
    const Animation& getAnimation (const std::string &animationKey) const;
};

#endif // ANIMATION_HPP