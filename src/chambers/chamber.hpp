#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entities/character.hpp"
#include "../utils/collision-solver.hpp"
#include "tile-map-generator.hpp"

#include "../entities/enemy/enemy.hpp"
#include "../economy/item-manager.hpp"

// Forward declarations for missing components
class Player;

class ChamberObserver {
public:
    virtual ~ChamberObserver() = default;
    virtual void onChamberCompleted() = 0;
    virtual void onChamberFailed() = 0;
};

class Chamber {
public:
    Chamber(Player& player);
    virtual ~Chamber() = default;

    void setObserver(ChamberObserver* obs) { observer = obs; }

    virtual void update(float dt);
    virtual void draw(sf::RenderWindow& window);
    
    virtual void processPlayerAttack(const Hitbox& hitbox);
    virtual void onFragmentCollected(float value);
    virtual void onEnemyHit(Enemy* enemy, bool lethal);

    void setGrid (const std::vector<std::vector<int>>& newGrid);
    const std::vector<std::vector<int>>& getGrid() const;
    
    void spawnEnemy(std::unique_ptr<Enemy> enemy);
    void checkCollisions(float dt);
    
    struct DebugHitbox {
        Hitbox shape;
        float timer;
    };
    
    bool getIsCompleted() const;
    const std::vector<sf::FloatRect>& getObstacles() const;
    std::vector<Enemy*> getEnemiesRaw() const;

protected:
    Player& player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    ItemManager itemManager;
    std::vector<DebugHitbox> debugHitboxes;
    
    std::vector<std::vector<int>> grid;
    std::vector<sf::FloatRect> obstacles;
    
    bool isCompleted;
    bool isFailed = false;
    bool dropsFragments = true;
    ChamberObserver* observer = nullptr;
    
    virtual void completeChamber();
    virtual void failChamber();

    virtual void drawBackground(sf::RenderWindow& window) {}
    virtual void drawForeground(sf::RenderWindow& window) {}

    RenderableTileMap tileMap;
    void buildObstaclesFromGrid();
};

#endif // CHAMBER_HPP
