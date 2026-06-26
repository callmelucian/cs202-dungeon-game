#include "asset-manager.hpp"
#include <iostream>

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

bool AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    if (_textures.find(name) != _textures.end()) return true;
    
    auto texture = std::make_unique<sf::Texture>();
    if (texture->loadFromFile(filename)) {
        _textures[name] = std::move(texture);
        return true;
    }
    
    std::cerr << "AssetManager: Failed to load texture '" << name << "' from '" << filename << "'\n";
    return false;
}

const sf::Texture& AssetManager::getTexture(const std::string& name) const {
    auto it = _textures.find(name);
    if (it != _textures.end()) return *(it->second);
    
    throw std::runtime_error("AssetManager: Texture '" + name + "' not found. Did you forget to load it?");
}

bool AssetManager::loadFont(const std::string& name, const std::string& filename) {
    if (_fonts.find(name) != _fonts.end()) return true;
    
    auto font = std::make_unique<sf::Font>();
    if (font->openFromFile(filename)) {
        _fonts[name] = std::move(font);
        return true;
    }
    
    std::cerr << "AssetManager: Failed to load font '" << name << "' from '" << filename << "'\n";
    return false;
}

const sf::Font& AssetManager::getFont(const std::string& name) const {
    auto it = _fonts.find(name);
    if (it != _fonts.end()) return *(it->second);
    
    throw std::runtime_error("AssetManager: Font '" + name + "' not found. Did you forget to load it?");
}