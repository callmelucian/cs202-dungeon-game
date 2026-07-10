#ifndef SETTING_MANAGER_HPP
#define SETTING_MANAGER_HPP

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Color.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

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

    float getSpriteMultiplier() const;
    float getCharacterSize() const;

    unsigned int getWindowWidth() const;
    unsigned int getWindowHeight() const;
    void setResolution(unsigned int width, unsigned int height);

    bool isFullscreen() const;
    void setFullscreen(bool enable);

    Difficulty getDifficulty() const;
    void setDifficulty(Difficulty diff);

    sf::Keyboard::Scancode getKeyBinding(const std::string& action) const;
    void setKeyBinding(const std::string& action, sf::Keyboard::Scancode scancode);

private:
    float musicVolume, sfxVolume;
    float spriteSizeMultiplier;
    float characterSize;

    unsigned int windowWidth;
    unsigned int windowHeight;
    bool fullscreen;
    Difficulty difficulty;
    std::unordered_map<std::string, sf::Keyboard::Scancode> keyBindings;

    SettingManager();
    SettingManager(const SettingManager&) = delete;
    SettingManager& operator=(const SettingManager&) = delete;

    void loadDefaults();
};

#endif // SETTING_MANAGER_HPP