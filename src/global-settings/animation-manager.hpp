#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include "../entities/animation/animation.hpp"

class AnimationManager {
public:
    static AnimationManager& getInstance();

    AnimationManager(const AnimationManager&) = delete;
    AnimationManager& operator=(const AnimationManager&) = delete;
    AnimationManager(AnimationManager&&) = delete;
    AnimationManager& operator=(AnimationManager&&) = delete;

    // a json file path that describes the animation set
    bool loadAnimationSet (const std::string &character, const std::string &filePath);
    const AnimationSet& getAnimationSet (const std::string &character) const;

private:
    AnimationManager() = default;
    ~AnimationManager() = default;

    std::unordered_map<std::string, AnimationSet> animationSets;
};

#endif // ANIMATION_MANAGER_HPP