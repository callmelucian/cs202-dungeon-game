#ifndef PLAYER_HEALTH_BAR_HPP
#define PLAYER_HEALTH_BAR_HPP

#include "health-bar.hpp"

namespace UI {

// PlayerHealthBar UI component representing the player's health & momentum HUD
class PlayerHealthBar : public HealthBar {
public:
    PlayerHealthBar();
    virtual ~PlayerHealthBar() = default;

protected:
    void updateSprites() override;
};

} // namespace UI

#endif // PLAYER_HEALTH_BAR_HPP
