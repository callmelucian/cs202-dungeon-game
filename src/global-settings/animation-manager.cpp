#include "animation-manager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

AnimationManager& AnimationManager::getInstance() {
    static AnimationManager instance;
    return instance;
}

bool AnimationManager::loadAnimationSet(const std::string &character, const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open animation set file: " << filePath << std::endl;
        return false;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "Failed to parse animation set file: " << e.what() << std::endl;
        return false;
    }

    AnimationSet set;
    if (j.contains("textureKey")) {
        set.textureKey = j["textureKey"].get<std::string>();
    }

    if (j.contains("animations")) {
        for (auto& [key, animData] : j["animations"].items()) {
            bool looping = true;
            if (animData.contains("looping")) {
                looping = animData["looping"].get<bool>();
            }

            std::vector<AnimationFrame> frames;
            if (animData.contains("frames")) {
                for (const auto& frameData : animData["frames"]) {
                    AnimationFrame frame;
                    frame.frameRegion.position.x = frameData["x"].get<int>();
                    frame.frameRegion.position.y = frameData["y"].get<int>();
                    frame.frameRegion.size.x = frameData["width"].get<int>();
                    frame.frameRegion.size.y = frameData["height"].get<int>();
                    frame.duration = frameData["duration"].get<float>();
                    frames.push_back(frame);
                }
            }

            Animation animation(frames, looping);
            set.addAnimation(key, animation);
        }
    }

    animationSets[character] = set;
    return true;
}

const AnimationSet& AnimationManager::getAnimationSet(const std::string &character) const {
    auto it = animationSets.find(character);
    if (it != animationSets.end()) {
        return it->second;
    }
    static AnimationSet emptySet;
    return emptySet;
}
