#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entities/character.hpp"
#include "../utils/collision-solver.hpp"
#include "tile-map-generator.hpp"
#include "wave-spawner.hpp"

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

    void setWaves(const std::vector<WaveConfig>& configs);

    virtual void update(float dt);
    virtual void draw(sf::RenderWindow& window);
    
    virtual void processPlayerAttack(const Hitbox& hitbox);
    virtual void onFragmentCollected(float value);
    virtual void onEnemyHit(Enemy* enemy, bool lethal);

    void setGrid (const std::vector<std::vector<int>>& newGrid);
    const std::vector<std::vector<int>>& getGrid() const;
    void setGrids2D5(const std::vector<std::vector<std::string>>& newTypeGrid, const std::vector<std::vector<int>>& newLevelGrid);
    
    void setPlayerSpawn(const sf::Vector2f& spawn) { playerSpawn = spawn; }
    sf::Vector2f getPlayerSpawn() const { return playerSpawn; }
    
    void spawnEnemy(std::unique_ptr<Enemy> enemy);
    void checkCollisions(float dt);
    
    struct DebugHitbox {
        Hitbox shape;
        float timer;
    };
    
    bool getIsCompleted() const;
    std::vector<sf::FloatRect> getObstaclesFor(const Character* character) const;
    std::vector<Enemy*> getEnemiesRaw() const;

protected:
    Player& player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    ItemManager itemManager;
    WaveSpawner waveSpawner;
    std::vector<DebugHitbox> debugHitboxes;
    
    sf::Vector2f playerSpawn = {-1.0f, -1.0f};
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<std::string>> typeGrid;
    std::vector<std::vector<int>> levelGrid;
    TilemapRenderData renderData2D5;
    std::vector<sf::FloatRect> baseObstacles;
    std::vector<sf::FloatRect> elevationObstacles;
    std::vector<sf::FloatRect> inverseElevationObstacles;
    
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
