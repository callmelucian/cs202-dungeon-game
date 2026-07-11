#ifndef CHAMBER_HPP
#define CHAMBER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../entities/character.hpp"
#include "../utils/collision-solver.hpp"

#include "../entities/enemies/enemy.hpp"
#include "../economy/item.hpp"

// Forward declarations for missing components
class Player;

class Chamber {
public:
    Chamber(Player& player) : player(player), isCompleted(false) {}
    virtual ~Chamber() = default;

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    
    void spawnEnemy(std::unique_ptr<Enemy> enemy) {
        enemies.push_back(std::move(enemy));
    }
    
    void checkCollisions() {
        // To be implemented
    }

    virtual void processPlayerAttack(const Hitbox& hitbox) = 0;

    struct DebugHitbox {
        Hitbox shape;
        float timer;
    };

protected:
    Player& player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<DebugHitbox> debugHitboxes;
    bool isCompleted;
};

// Base concrete implementation for testing
class BaseChamber : public Chamber {
public:
    BaseChamber(Player& player) : Chamber(player) {}
    
    void update(float dt) override {
        for (auto it = debugHitboxes.begin(); it != debugHitboxes.end(); ) {
            it->timer -= dt;
            if (it->timer <= 0) {
                it = debugHitboxes.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void draw(sf::RenderWindow& window) override {
        // Draw debug hitboxes
        for (const auto& hb : debugHitboxes) {
            CollisionSolver::drawDebug(window, hb.shape);
        }
    }

    void processPlayerAttack(const Hitbox& hitbox) override {
        debugHitboxes.push_back({hitbox, 0.2f});
    }
};

#endif // CHAMBER_HPP
