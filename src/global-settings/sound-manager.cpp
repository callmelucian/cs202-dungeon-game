#include "sound-manager.hpp"
#include <iostream>
#include <algorithm>

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

void SoundManager::playSound(const std::string& name) {
    if (_soundBuffers.find(name) == _soundBuffers.end()) {
        auto buffer = std::make_unique<sf::SoundBuffer>();
        if (buffer->loadFromFile(name)) {
            _soundBuffers[name] = std::move(buffer);
        } else {
            std::cerr << "SoundManager: Failed to load sound buffer from '" << name << "'\n";
            return;
        }
    }

    // Static pool to keep sf::Sound objects alive while playing, since the PUML lacks a class member for it
    static std::vector<std::unique_ptr<sf::Sound>> sounds;
    sounds.erase(
        std::remove_if(sounds.begin(), sounds.end(),
            [](const std::unique_ptr<sf::Sound>& s) { return s->getStatus() == sf::Sound::Status::Stopped; }),
        sounds.end()
    );
    
    auto s = std::make_unique<sf::Sound>(*_soundBuffers[name]);
    s->play();
    sounds.push_back(std::move(s));
}

void SoundManager::playMusic(const std::string& filepath, bool loop) {
    if (!_music.openFromFile(filepath)) {
        std::cerr << "SoundManager: Failed to open music from '" << filepath << "'\n";
        return;
    }

    _music.setLooping(loop);
    _music.play();
}

void SoundManager::stopMusic() {
    _music.stop();
}
