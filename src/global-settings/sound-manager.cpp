#include "sound-manager.hpp"
#include <iostream>
#include <algorithm>

SoundManager::SoundManager() : soundPool(MAX_SOUNDS) {}

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

bool SoundManager::loadSound(const std::string& name, const std::string& filepath) {
    if (soundBuffers.find(name) != soundBuffers.end()) {
        std::cerr << "SoundManager: Sound " << name << " is already loaded.\n";
        return true;
    }

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (buffer->loadFromFile(filepath)) {
        soundBuffers[name] = std::move(buffer);
        return true;
    }

    std::cerr << "SoundManager: Failed to load sound " << name << " from " << filepath << "\n";
    return false;
}

void SoundManager::playSound(const std::string& name) {
    if (soundBuffers.find(name) == soundBuffers.end()) {
        std::cerr << "SoundManager: Sound '" << name << "' not found. Did you forget to load it?\n";
        return;
    }

    // Find a slot that is either empty or has finished playing its sound
    for (size_t i = 0; i < MAX_SOUNDS; ++i) {
        if (!soundPool[i].has_value() || soundPool[i]->getStatus() == sf::Sound::Status::Stopped) {
            // emplace() cleanly constructs a new sf::Sound directly in the pre-allocated memory slot
            soundPool[i].emplace(*soundBuffers[name]);
            soundPool[i]->play();
            return;
        }
    }
    
    // If all 32 channels are blasting at once, we drop the sound to prevent lag and audio distortion
    std::cerr << "SoundManager: Sound pool full (32 playing)! Dropping sound.\n";
}

void SoundManager::playMusic(const std::string& filepath, bool loop) {
    if (!music.openFromFile(filepath)) {
        std::cerr << "SoundManager: Failed to open music from '" << filepath << "'\n";
        return;
    }

    music.setLooping(loop);
    music.play();
}

void SoundManager::stopMusic() {
    music.stop();
}
