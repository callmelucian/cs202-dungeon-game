#ifndef SOUND_MANAGER_HPP
#define SOUND_MANAGER_HPP

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <optional>

class SoundManager {
public:
    static SoundManager& getInstance();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
    SoundManager(SoundManager&&) = delete;
    SoundManager& operator=(SoundManager&&) = delete;

    bool loadSound(const std::string& name, const std::string& filepath);
    void playSound(const std::string& name);
    void playMusic(const std::string& filepath, bool loop);
    void stopMusic();

    void setMusicVolume(float volume);
    void setSfxVolume(float volume);

private:
    SoundManager();
    ~SoundManager() = default;

    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> soundBuffers;
    sf::Music music;

    static const size_t MAX_SOUNDS = 32;
    std::vector<std::optional<sf::Sound>> soundPool;
};

#endif // SOUND_MANAGER_HPP