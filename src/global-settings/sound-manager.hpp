#ifndef SOUND_MANAGER_HPP
#define SOUND_MANAGER_HPP

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

class SoundManager {
public:
    static SoundManager& getInstance();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
    SoundManager(SoundManager&&) = delete;
    SoundManager& operator=(SoundManager&&) = delete;

    void playSound(const std::string& name);
    void playMusic(const std::string& filepath, bool loop);
    void stopMusic();

private:
    SoundManager() = default;
    ~SoundManager() = default;

    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> _soundBuffers;
    sf::Music _music;
};

#endif // SOUND_MANAGER_HPP
