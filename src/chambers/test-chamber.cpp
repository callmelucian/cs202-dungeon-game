#include "test-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/enemy/waterlogged-scribe.hpp"
#include "../entities/enemy/shard-soldier.hpp"
#include "../entities/enemy/enemy-factory.hpp"
#include "../entities/enemy/bone-sprinter.hpp"
#include "../entities/effects/slowed-effect.hpp"
#include "../economy/item.hpp"
#include "../entities/player.hpp"

TestChamber::TestChamber(Player& player) : Chamber(player) {
    float ox = SettingManager::getInstance().getGridOffsetX();
    float oy = SettingManager::getInstance().getGridOffsetY();
    float cell = SettingManager::getInstance().getCellSize();

    auto scribe = std::make_unique<WaterloggedScribe>(player);
    scribe->setPosition({ox + 3.5f * cell, oy + 3.5f * cell});
    spawnEnemy(std::move(scribe));

    auto soldier = std::make_unique<ShardSoldier>(player);
    soldier->setPosition({ox + 7.5f * cell, oy + 3.5f * cell});
    spawnEnemy(std::move(soldier));

    auto sprinter = std::make_unique<BoneSprinter>(player, true);
    sprinter->setPosition({ox + 5.5f * cell, oy + 7.5f * cell});
    spawnEnemy(std::move(sprinter));

    auto shardSoldier = EnemyFactory::createEnemy(EnemyType::SHARD_SOLDIER, player);
    shardSoldier->setPosition({300.f, 400.f});
    spawnEnemy(std::move(shardSoldier));
    
    // Spawn a dummy EchoFragment for testing physics and magnet
    items.push_back(std::make_unique<EchoFragment>(sf::Vector2f(200.f, 200.f), 10.0f));
}

void TestChamber::update(float dt) {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (!(*it)->isAlive()) {
            (*it)->onDeath();
            it = enemies.erase(it);
        } else {
            (*it)->update(dt);
            (*it)->updateState(dt, *this);
            ++it;
        }
    }
    
    // Update items and check collection
    for (auto it = items.begin(); it != items.end(); ) {
        (*it)->update(dt, player.getPosition());
        if ((*it)->isCollected()) {
            (*it)->onCollect(player, *this);
            it = items.erase(it);
        } else {
            ++it;
        }
    }
    
    checkCollisions(dt);
    
    for (auto it = debugHitboxes.begin(); it != debugHitboxes.end(); ) {
        it->timer -= dt;
        if (it->timer <= 0) {
            it = debugHitboxes.erase(it);
        } else {
            ++it;
        }
    }
}

void TestChamber::draw(sf::RenderWindow& window) {
    window.draw(tileMap);

    for (const auto& enemy : enemies) {
        enemy->draw(window);
    }
    
    for (const auto& item : items) {
        window.draw(*item);
    }

    for (const auto& hb : debugHitboxes) {
        CollisionSolver::drawDebug(window, hb.shape);
    }
}

void TestChamber::processPlayerAttack(const Hitbox& hitbox) {
    debugHitboxes.push_back({hitbox, 0.2f});
    for (auto& enemy : enemies) {
        if (enemy->isAlive()) {
            if (CollisionSolver::checkCollision(hitbox, enemy->getBounds())) {
                enemy->takeDamage(player.getEffectiveStats().damage);
            }
        }
    }
}
