#include "asset-manager.hpp"
#include <iostream>

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

bool AssetManager::loadTexture(const std::string& name, const std::string& filename) {
    if (textures.find(name) != textures.end()) return true;
    
    auto texture = std::make_unique<sf::Texture>();
    if (texture->loadFromFile(filename)) {
        textures[name] = std::move(texture);
        return true;
    }
    
    std::cerr << "AssetManager: Failed to load texture '" << name << "' from '" << filename << "'\n";
    return false;
}

const sf::Texture& AssetManager::getTexture(const std::string& name) const {
    auto it = textures.find(name);
    if (it != textures.end()) return *(it->second);
    
    throw std::runtime_error("AssetManager: Texture '" + name + "' not found. Did you forget to load it?");
}

bool AssetManager::loadFont(const std::string& name, const std::string& filename) {
    if (fonts.find(name) != fonts.end()) return true;
    
    auto font = std::make_unique<sf::Font>();
    if (font->openFromFile(filename)) {
        fonts[name] = std::move(font);
        return true;
    }
    
    std::cerr << "AssetManager: Failed to load font '" << name << "' from '" << filename << "'\n";
    return false;
}

const sf::Font& AssetManager::getFont(const std::string& name) const {
    auto it = fonts.find(name);
    if (it != fonts.end()) return *(it->second);
    
    throw std::runtime_error("AssetManager: Font '" + name + "' not found. Did you forget to load it?");
}