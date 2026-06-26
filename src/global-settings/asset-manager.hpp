#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

class AssetManager {
public:
    static AssetManager& getInstance();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    bool loadTexture(const std::string& name, const std::string& filename);
    const sf::Texture& getTexture(const std::string& name) const;

    bool loadFont(const std::string& name, const std::string& filename);
    const sf::Font& getFont(const std::string& name) const;

private:
    AssetManager() = default;
    ~AssetManager() = default;

    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> _textures;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>> _fonts;
};

#endif // ASSET_MANAGER_HPP