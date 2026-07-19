#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entities/character.hpp"
#include "../utils/collision-solver.hpp"
#include "tile-map-generator.hpp"

#include "../entities/enemy/enemy.hpp"
#include "../economy/item.hpp"

// Forward declarations for missing components
class Player;

class Chamber {
public:
    Chamber(Player& player);
    virtual ~Chamber() = default;

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    
    void setGrid(const std::vector<std::vector<int>>& newGrid);
    
    void spawnEnemy(std::unique_ptr<Enemy> enemy);
    void checkCollisions(float dt);
    
    // Pathfinding & collision
    bool isWalkable(sf::Vector2f position) const;
    std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f target) const;

    virtual void processPlayerAttack(const Hitbox& hitbox) = 0;

    void spawnFragments(sf::Vector2f position, int count);
    void updateItems(float dt);
    virtual void onFragmentCollected(float value) {}

    struct DebugHitbox {
        Hitbox shape;
        float timer;
    };
    
    bool getIsCompleted() const { return isCompleted; }
    const std::vector<sf::FloatRect>& getObstacles() const { return obstacles; }
    std::vector<Enemy*> getEnemiesRaw() const;

protected:
    Player& player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<DebugHitbox> debugHitboxes;
    
    std::vector<std::vector<int>> grid;
    std::vector<sf::FloatRect> obstacles;
    
    bool isCompleted;
    
    RenderableTileMap tileMap;
    
    void buildObstaclesFromGrid();
};

#endif // CHAMBER_HPP
