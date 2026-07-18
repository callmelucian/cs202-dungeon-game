#include "test-chamber.hpp"
#include "../global-settings/setting-manager.hpp"
#include "../entities/enemy/waterlogged-scribe.hpp"
#include "../entities/enemy/shard-soldier.hpp"
#include "../entities/enemy/bone-sprinter.hpp"
#include "../entities/effects/slowed-effect.hpp"
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
}

void TestChamber::update(float dt) {
    for (auto& enemy : enemies) {
        enemy->update(dt);
        enemy->updateState(dt, *this);
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
                enemy->applyStatusEffect(std::make_unique<SlowedEffect>(-1.0f));
            }
        }
    }
}
