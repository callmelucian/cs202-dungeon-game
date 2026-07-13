#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entities/character.hpp"
#include "../utils/collision-solver.hpp"

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
    void checkCollisions();

    virtual void processPlayerAttack(const Hitbox& hitbox) = 0;

    struct DebugHitbox {
        Hitbox shape;
        float timer;
    };
    
    bool getIsCompleted() const { return isCompleted; }
    const std::vector<sf::FloatRect>& getObstacles() const { return obstacles; }

protected:
    Player& player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<DebugHitbox> debugHitboxes;
    
    std::vector<std::vector<int>> grid;
    std::vector<sf::FloatRect> obstacles;
    
    bool isCompleted;
    
    void buildObstaclesFromGrid();
};

// Base concrete implementation for testing
class BaseChamber : public Chamber {
public:
    BaseChamber(Player& player);
    
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void processPlayerAttack(const Hitbox& hitbox) override;
};

#endif // CHAMBER_HPP
