#include "animation-manager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

AnimationManager& AnimationManager::getInstance() {
    static AnimationManager instance;
    return instance;
}

bool AnimationManager::loadAnimationSet(const std::string &character, const std::string &filePath, const std::string& textureKeyOverride) {
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
    if (!textureKeyOverride.empty()) {
        set.textureKey = textureKeyOverride;
    } else if (j.contains("textureKey")) {
        set.textureKey = j["textureKey"].get<std::string>();
    }

    if (j.contains("animations")) {
        for (auto& [key, animData] : j["animations"].items()) {
            bool looping = true;
            if (animData.contains("looping")) {
                looping = animData["looping"].get<bool>();
            }

            bool mirrorX = false;
            if (animData.contains("mirrorX")) {
                mirrorX = animData["mirrorX"].get<bool>();
            }
            
            bool mirrorY = false;
            if (animData.contains("mirrorY")) {
                mirrorY = animData["mirrorY"].get<bool>();
            }

            std::vector<AnimationFrame> frames;
            if (animData.contains("frames")) {
                for (const auto& frameData : animData["frames"]) {
                    AnimationFrame frame;
                    
                    int x = frameData["x"].get<int>();
                    int y = frameData["y"].get<int>();
                    int width = frameData["width"].get<int>();
                    int height = frameData["height"].get<int>();
                    
                    if (mirrorX) {
                        x = x + width;
                        width = -width;
                    }
                    if (mirrorY) {
                        y = y + height;
                        height = -height;
                    }

                    frame.frameRegion.position.x = x;
                    frame.frameRegion.position.y = y;
                    frame.frameRegion.size.x = width;
                    frame.frameRegion.size.y = height;
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
