#include "setting-manager.hpp"
#include "color-palette-manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

SettingManager::SettingManager() {
    loadDefaults();
}

SettingManager& SettingManager::getInstance() {
    static SettingManager instance;
    return instance;
}

void SettingManager::loadDefaults() {
    musicVolume = 50.0f;
    sfxVolume = 50.0f;
    windowWidth = 1800;
    windowHeight = 900;
    gridCols = 40;
    gridRows = 40;
    cellSize = 40.0f;
    characterSize = cellSize;
    gridOffsetX = (windowWidth - gridCols * cellSize) / 2.0f;
    gridOffsetY = (windowHeight - gridRows * cellSize) / 2.0f;
    spriteSizeMultiplier = 1.2f;
    speedMultiplier = 1.f;
    enemyAttackTime = 1.5f;

    fullscreen = false;
    difficulty = Difficulty::Normal;

    keyBindings["MoveUp"] = sf::Keyboard::Scancode::W;
    keyBindings["MoveDown"] = sf::Keyboard::Scancode::S;
    keyBindings["MoveLeft"] = sf::Keyboard::Scancode::A;
    keyBindings["MoveRight"] = sf::Keyboard::Scancode::D;
    keyBindings["SwitchForm1"] = sf::Keyboard::Scancode::Num1;
    keyBindings["SwitchForm2"] = sf::Keyboard::Scancode::Num2;
    keyBindings["SwitchForm3"] = sf::Keyboard::Scancode::Num3;
    keyBindings["Attack"] = sf::Keyboard::Scancode::J;
    keyBindings["Special1"] = sf::Keyboard::Scancode::Q;
    keyBindings["Special2"] = sf::Keyboard::Scancode::E;
}

bool SettingManager::loadSettings(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        if (j.contains("audio")) {
            const auto& audio = j["audio"];
            if (audio.contains("musicVolume")) {
                musicVolume = audio["musicVolume"].get<float>();
            }
            if (audio.contains("sfxVolume")) {
                sfxVolume = audio["sfxVolume"].get<float>();
            }
        }

        if (j.contains("video")) {
            const auto& video = j["video"];
            if (video.contains("resolution")) {
                const auto& res = video["resolution"];
                if (res.contains("width")) {
                    windowWidth = res["width"].get<unsigned int>();
                }
                if (res.contains("height")) {
                    windowHeight = res["height"].get<unsigned int>();
                }
            }
            if (video.contains("fullscreen")) {
                fullscreen = video["fullscreen"].get<bool>();
            }
        }

        if (j.contains("gameplay")) {
            const auto& gameplay = j["gameplay"];
            if (gameplay.contains("difficulty")) {
                std::string diffStr = gameplay["difficulty"].get<std::string>();
                if (diffStr == "Easy" || diffStr == "easy") {
                    difficulty = Difficulty::Easy;
                }
                else if (diffStr == "Normal" || diffStr == "normal") {
                    difficulty = Difficulty::Normal;
                }
                else if (diffStr == "Hard" || diffStr == "hard") {
                    difficulty = Difficulty::Hard;
                }
            }
        }

        if (j.contains("keyBindings")) {
            const auto& keys = j["keyBindings"];
            for (auto it = keys.begin(); it != keys.end(); ++it) {
                keyBindings[it.key()] = static_cast<sf::Keyboard::Scancode>(it.value().get<int>());
            }
        }

        if (j.contains("palette")) {
            ColorPaletteManager::getInstance().load(j["palette"]);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse settings JSON: " << e.what() << "\n";
        return false;
    }

    return true;
}

bool SettingManager::saveSettings(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    nlohmann::json j;
    j["audio"]["musicVolume"] = musicVolume;
    j["audio"]["sfxVolume"] = sfxVolume;
    j["video"]["resolution"]["width"] = windowWidth;
    j["video"]["resolution"]["height"] = windowHeight;
    j["video"]["fullscreen"] = fullscreen;

    std::string diffStr = "Normal";
    if (difficulty == Difficulty::Easy) {
        diffStr = "Easy";
    }
    else if (difficulty == Difficulty::Hard) {
        diffStr = "Hard";
    }
    j["gameplay"]["difficulty"] = diffStr;

    nlohmann::json keys = nlohmann::json::object();
    for (const auto& [action, scancode] : keyBindings) {
        keys[action] = static_cast<int>(scancode);
    }
    j["keyBindings"] = keys;

    nlohmann::json palJson = nlohmann::json::object();
    ColorPaletteManager::getInstance().save(palJson);
    j["palette"] = palJson;

    file << j.dump(4);
    return true;
}

float SettingManager::getMusicVolume() const {
    return musicVolume;
}

void SettingManager::setMusicVolume(float volume) {
    musicVolume = volume;
}

float SettingManager::getSfxVolume() const {
    return sfxVolume;
}

void SettingManager::setSfxVolume(float volume) {
    sfxVolume = volume;
}

float SettingManager::getCellSize() const {
    return cellSize;
}

float SettingManager::getSpriteMultiplier() const {
    return spriteSizeMultiplier;
}

float SettingManager::getCharacterSize() const {
    return characterSize;
}

float SettingManager::getEnemyAttackTime() const {
    return enemyAttackTime;
}

float SettingManager::getGridOffsetX() const {
    return gridOffsetX;
}

float SettingManager::getGridOffsetY() const {
    return gridOffsetY;
}

unsigned int SettingManager::getGridCols() const {
    return gridCols;
}

unsigned int SettingManager::getGridRows() const {
    return gridRows;
}

unsigned int SettingManager::getWindowWidth() const {
    return windowWidth;
}

unsigned int SettingManager::getWindowHeight() const {
    return windowHeight;
}

void SettingManager::setResolution(unsigned int width, unsigned int height) {
    windowWidth = width;
    windowHeight = height;
}

bool SettingManager::isFullscreen() const {
    return fullscreen;
}

void SettingManager::setFullscreen(bool enable) {
    fullscreen = enable;
}

SettingManager::Difficulty SettingManager::getDifficulty() const {
    return difficulty;
}

void SettingManager::setDifficulty(Difficulty diff) {
    difficulty = diff;
}

sf::Keyboard::Scancode SettingManager::getKeyBinding(const std::string& action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return it->second;
    }
    return sf::Keyboard::Scancode::Unknown;
}

void SettingManager::setKeyBinding(const std::string& action, sf::Keyboard::Scancode scancode) {
    keyBindings[action] = scancode;
}

float SettingManager::getSpeedMultiplier() const {
    return speedMultiplier;
}