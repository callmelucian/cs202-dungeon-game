#ifndef ENEMY_HEALTH_BAR_HPP
#define ENEMY_HEALTH_BAR_HPP

#include "health-bar.hpp"

namespace UI {

// EnemyHealthBar UI component representing an enemy's health HUD
class EnemyHealthBar : public HealthBar {
public:
    EnemyHealthBar();
    virtual ~EnemyHealthBar() = default;

protected:
    void updateSprites() override;
};

} // namespace UI

#endif // ENEMY_HEALTH_BAR_HPP
