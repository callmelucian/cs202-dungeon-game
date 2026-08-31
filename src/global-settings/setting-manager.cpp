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

static int encodeBinding(const ActionBinding& b) {
    if (b.isMouseButton) {
        return -100 - static_cast<int>(b.mouseButton);
    }
    return static_cast<int>(b.scancode);
}

static ActionBinding decodeBinding(int val) {
    ActionBinding b;
    if (val <= -100) {
        b.isMouseButton = true;
        b.mouseButton = static_cast<sf::Mouse::Button>(-(val + 100));
        b.scancode = sf::Keyboard::Scancode::Unknown;
    } else {
        b.isMouseButton = false;
        b.scancode = static_cast<sf::Keyboard::Scancode>(val);
    }
    return b;
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
    characterHitboxSize = cellSize * 0.7f;
    gridOffsetX = (windowWidth - gridCols * cellSize) / 2.0f;
    gridOffsetY = (windowHeight - gridRows * cellSize) / 2.0f;
    spriteSizeMultiplier = 1.2f;
    speedMultiplier = 0.35f;
    enemyAttackTime = 1.5f;

    fullscreen = false;
    difficulty = Difficulty::Normal;

    keyBindings["MoveUp"] = {false, sf::Keyboard::Scancode::W, sf::Mouse::Button::Left};
    keyBindings["MoveDown"] = {false, sf::Keyboard::Scancode::S, sf::Mouse::Button::Left};
    keyBindings["MoveLeft"] = {false, sf::Keyboard::Scancode::A, sf::Mouse::Button::Left};
    keyBindings["MoveRight"] = {false, sf::Keyboard::Scancode::D, sf::Mouse::Button::Left};
    keyBindings["Dash"] = {true, sf::Keyboard::Scancode::Unknown, sf::Mouse::Button::Right};
    keyBindings["SwitchForm1"] = {false, sf::Keyboard::Scancode::Num1, sf::Mouse::Button::Left};
    keyBindings["SwitchForm2"] = {false, sf::Keyboard::Scancode::Num2, sf::Mouse::Button::Left};
    keyBindings["SwitchForm3"] = {false, sf::Keyboard::Scancode::Num3, sf::Mouse::Button::Left};
    keyBindings["Attack"] = {false, sf::Keyboard::Scancode::J, sf::Mouse::Button::Left};
    keyBindings["Special1"] = {false, sf::Keyboard::Scancode::Q, sf::Mouse::Button::Left};
    keyBindings["Special2"] = {false, sf::Keyboard::Scancode::E, sf::Mouse::Button::Left};
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
                keyBindings[it.key()] = decodeBinding(it.value().get<int>());
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
    for (const auto& [action, binding] : keyBindings) {
        keys[action] = encodeBinding(binding);
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

float SettingManager::getCharacterHitboxSize() const {
    return characterHitboxSize;
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
    if (it != keyBindings.end() && !it->second.isMouseButton) {
        return it->second.scancode;
    }
    return sf::Keyboard::Scancode::Unknown;
}

void SettingManager::setKeyBinding(const std::string& action, sf::Keyboard::Scancode scancode) {
    keyBindings[action] = ActionBinding{false, scancode, sf::Mouse::Button::Left};
}

ActionBinding SettingManager::getActionBinding(const std::string& action) const {
    auto it = keyBindings.find(action);
    if (it != keyBindings.end()) {
        return it->second;
    }
    return ActionBinding{};
}

void SettingManager::setActionBinding(const std::string& action, const ActionBinding& binding) {
    keyBindings[action] = binding;
}

void SettingManager::setActionScancode(const std::string& action, sf::Keyboard::Scancode scancode) {
    keyBindings[action] = ActionBinding{false, scancode, sf::Mouse::Button::Left};
}

void SettingManager::setActionMouseButton(const std::string& action, sf::Mouse::Button button) {
    keyBindings[action] = ActionBinding{true, sf::Keyboard::Scancode::Unknown, button};
}

bool SettingManager::matchesEvent(const std::string& action, const sf::Event& event) const {
    auto it = keyBindings.find(action);
    if (it == keyBindings.end()) return false;

    const auto& binding = it->second;
    if (binding.isMouseButton) {
        if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
            return mousePress->button == binding.mouseButton;
        }
    } else {
        if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
            return keyPress->scancode == binding.scancode;
        }
    }
    return false;
}

bool SettingManager::isActionActive(const std::string& action) const {
    auto it = keyBindings.find(action);
    if (it == keyBindings.end()) return false;

    const auto& binding = it->second;
    if (binding.isMouseButton) {
        return sf::Mouse::isButtonPressed(binding.mouseButton);
    } else {
        return sf::Keyboard::isKeyPressed(binding.scancode);
    }
}

float SettingManager::getSpeedMultiplier() const {
    return speedMultiplier;
}