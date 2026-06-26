#include "sound-manager.hpp"
#include <iostream>
#include <algorithm>

SoundManager::SoundManager() : _soundPool(MAX_SOUNDS) {}

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

bool SoundManager::loadSound(const std::string& name, const std::string& filepath) {
    if (_soundBuffers.find(name) != _soundBuffers.end()) {
        std::cerr << "SoundManager: Sound " << name << " is already loaded.\n";
        return true;
    }

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (buffer->loadFromFile(filepath)) {
        _soundBuffers[name] = std::move(buffer);
        return true;
    }

    std::cerr << "SoundManager: Failed to load sound " << name << " from " << filepath << "\n";
    return false;
}

void SoundManager::playSound(const std::string& name) {
    if (_soundBuffers.find(name) == _soundBuffers.end()) {
        std::cerr << "SoundManager: Sound '" << name << "' not found. Did you forget to load it?\n";
        return;
    }

    // Find a slot that is either empty or has finished playing its sound
    for (size_t i = 0; i < MAX_SOUNDS; ++i) {
        if (!_soundPool[i].has_value() || _soundPool[i]->getStatus() == sf::Sound::Status::Stopped) {
            // emplace() cleanly constructs a new sf::Sound directly in the pre-allocated memory slot
            _soundPool[i].emplace(*_soundBuffers[name]);
            _soundPool[i]->play();
            return;
        }
    }
    
    // If all 32 channels are blasting at once, we drop the sound to prevent lag and audio distortion
    std::cerr << "SoundManager: Sound pool full (32 playing)! Dropping sound.\n";
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
