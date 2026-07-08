#include "animation.hpp"
#include <algorithm>
#include <cmath>

Animation::Animation(std::vector<AnimationFrame> frames, bool looping) 
    : frames(std::move(frames)), looping(looping) {
    double total = 0.0;
    for (const auto& frame : this->frames) {
        total += frame.duration;
        checkpoints.push_back(total);
    }
}

const AnimationFrame& Animation::getFrame(float elapsed) const {
    if (frames.empty()) {
        static AnimationFrame defaultFrame{};
        return defaultFrame;
    }
    
    float totalDuration = checkpoints.empty() ? 0.0f : static_cast<float>(checkpoints.back());
    if (totalDuration <= 0.0f) {
        return frames.front();
    }

    float t = elapsed;
    if (looping) {
        t = std::fmod(elapsed, totalDuration);
    } else if (t >= totalDuration) {
        return frames.back();
    }

    auto it = std::upper_bound(checkpoints.begin(), checkpoints.end(), t);
    size_t index = std::distance(checkpoints.begin(), it);
    if (index >= frames.size()) {
        index = frames.size() - 1;
    }
    return frames[index];
}

bool Animation::isFinished(float elapsed) const {
    if (looping || checkpoints.empty()) return false;
    return elapsed >= checkpoints.back();
}

void AnimationSet::addAnimation(const std::string &animationKey, const Animation &animation) {
    animations.insert({animationKey, animation});
}

const Animation& AnimationSet::getAnimation(const std::string &animationKey) const {
    auto it = animations.find(animationKey);
    if (it != animations.end()) {
        return it->second;
    }
    static Animation defaultAnim({}, false);
    return defaultAnim;
}
