#ifndef SETTING_MANAGER_HPP
#define SETTING_MANAGER_HPP

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct ActionBinding {
    bool isMouseButton = false;
    sf::Keyboard::Scancode scancode = sf::Keyboard::Scancode::Unknown;
    sf::Mouse::Button mouseButton = sf::Mouse::Button::Right;
};

class SettingManager {
public:
    enum class Difficulty {
        Easy,
        Normal,
        Hard
    };

    static SettingManager& getInstance();

    // Settings I/O
    bool loadSettings(const std::string& filepath);
    bool saveSettings(const std::string& filepath);

    // Getters and Setters
    float getMusicVolume() const;
    void setMusicVolume(float volume);

    float getSfxVolume() const;
    void setSfxVolume(float volume);

    float getCellSize() const;
    float getSpriteMultiplier() const;
    float getCharacterSize() const;
    float getCharacterHitboxSize() const;
    float getGridOffsetX() const;
    float getGridOffsetY() const;
    float getSpeedMultiplier() const;
    float getEnemyAttackTime() const;

    unsigned int getGridCols() const;
    unsigned int getGridRows() const;

    unsigned int getWindowWidth() const;
    unsigned int getWindowHeight() const;
    void setResolution(unsigned int width, unsigned int height);

    bool isFullscreen() const;
    void setFullscreen(bool enable);

    Difficulty getDifficulty() const;
    void setDifficulty(Difficulty diff);

    sf::Keyboard::Scancode getKeyBinding(const std::string& action) const;
    void setKeyBinding(const std::string& action, sf::Keyboard::Scancode scancode);

    ActionBinding getActionBinding(const std::string& action) const;
    void setActionBinding(const std::string& action, const ActionBinding& binding);
    void setActionScancode(const std::string& action, sf::Keyboard::Scancode scancode);
    void setActionMouseButton(const std::string& action, sf::Mouse::Button button);

    bool matchesEvent(const std::string& action, const sf::Event& event) const;
    bool isActionActive(const std::string& action) const;

private:
    float musicVolume, sfxVolume;
    float cellSize;
    float spriteSizeMultiplier;
    float characterSize;
    float characterHitboxSize;
    float gridOffsetX;
    float gridOffsetY;
    float speedMultiplier;
    float enemyAttackTime;

    unsigned int gridCols;
    unsigned int gridRows;

    unsigned int windowWidth;
    unsigned int windowHeight;
    bool fullscreen;
    Difficulty difficulty;
    std::unordered_map<std::string, ActionBinding> keyBindings;

    SettingManager();
    SettingManager(const SettingManager&) = delete;
    SettingManager& operator=(const SettingManager&) = delete;

    void loadDefaults();
};

#endif // SETTING_MANAGER_HPP