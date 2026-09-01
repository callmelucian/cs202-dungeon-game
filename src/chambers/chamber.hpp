#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entities/character.hpp"
#include "../utils/collision-solver.hpp"
#include "tilemap-loader.hpp"
#include "wave-spawner.hpp"

#include "../entities/enemy/enemy.hpp"
#include "../economy/item-manager.hpp"
#include "../entities/effects/arrow-projectile.hpp"
#include "exit-gate.hpp"

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
    
    virtual void freezeAllEnemies(float duration);
    bool isEnemiesFrozen() const { return freezeTimer > 0.0f; }

    virtual int processPlayerAttack(const Hitbox& hitbox);
    virtual void spawnArrow(sf::Vector2f startPos, sf::Vector2f direction, float maxDistance, float speed = 1000.0f, ArrowHitMode hitMode = ArrowHitMode::SINGLE_TARGET, bool isRedLaser = false);
    virtual void onFragmentCollected(float value);
    virtual void onEnemyHit(Enemy* enemy, bool lethal);
    virtual void setGrids2D5(const std::vector<std::vector<std::string>>& newTypeGrid, const std::vector<std::vector<int>>& newLevelGrid, const std::vector<std::vector<std::string>>& newBridgeGrid = {});
    const std::vector<std::vector<std::string>>& getTypeGrid() const { return typeGrid; }
    const std::vector<std::vector<int>>& getLevelGrid() const { return levelGrid; }
    const std::vector<std::vector<std::string>>& getBridgeGrid() const { return bridgeGrid; }
    const std::vector<std::vector<uint8_t>>& getWalkableGrid() const { return renderData2D5.walkableGrid; }
    const TilemapRenderData& getRenderData2D5() const { return renderData2D5; }
    
    void setPlayerSpawn(const sf::Vector2f& spawn) { playerSpawn = spawn; }
    sf::Vector2f getPlayerSpawn() const { return playerSpawn; }
    
    virtual void setExitPosition(const sf::Vector2f& pos);
    sf::Vector2f getExitPosition() const;
    ExitGate* getExitGate() { return exitGate.get(); }
    const ExitGate* getExitGate() const { return exitGate.get(); }

    void spawnEnemy(std::unique_ptr<Enemy> enemy);
    void checkCollisions(float dt);
    
    struct DebugHitbox {
        Hitbox shape;
        float timer;
    };
    
    bool getIsCompleted() const;
    virtual std::vector<sf::FloatRect> getObstaclesFor(const Character* character) const;
    std::vector<sf::FloatRect> getArrowSolidObstacles(sf::Vector2f shooterPos = sf::Vector2f(0.0f, 0.0f)) const;
    int getElevationLevelAt(sf::Vector2f pos) const;
    bool isStairsAt(sf::Vector2f pos) const;
    bool isOnStairs(const Character* character) const;
    virtual std::vector<Enemy*> getEnemiesRaw() const;
    ItemManager& getItemManager() { return itemManager; }
    const WaveSpawner& getWaveSpawner() const { return waveSpawner; }
    size_t getEnemyCount() const { return enemies.size(); }

protected:
    Player& player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Enemy>> pendingEnemies;
    ItemManager itemManager;
    WaveSpawner waveSpawner;
    std::vector<DebugHitbox> debugHitboxes;
    std::vector<ArrowProjectile> activeArrows;
    
    sf::Vector2f playerSpawn = {-1.0f, -1.0f};
    sf::Vector2f exitPosition = {-1.0f, -1.0f};
    std::unique_ptr<ExitGate> exitGate;
    std::vector<std::vector<std::string>> typeGrid;
    std::vector<std::vector<int>> levelGrid;
    std::vector<std::vector<std::string>> bridgeGrid;
    TilemapRenderData renderData2D5;
    std::vector<sf::FloatRect> baseObstacles;
    std::vector<sf::FloatRect> elevationObstacles;
    std::vector<sf::FloatRect> inverseElevationObstacles;
    
    bool isCompleted;
    bool isFailed = false;
    bool dropsFragments = true;
    float freezeTimer = 0.0f;
    ChamberObserver* observer = nullptr;
    
    virtual void completeChamber();
    virtual void failChamber();

    virtual void drawBackground(sf::RenderWindow& window) {}
    virtual void drawForeground(sf::RenderWindow& window) {}

    RenderableTileMap tileMap;
    void buildObstaclesFromGrid();
};

#endif // CHAMBER_HPP
