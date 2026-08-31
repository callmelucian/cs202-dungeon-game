#ifndef PREVENT_CHAMBER_HPP
#define PREVENT_CHAMBER_HPP

#include "chamber.hpp"
#include "../core/enums.hpp"
#include "../core/game.hpp"
#include <optional>

class PreventChamber : public Chamber {
private:
    std::optional<EchoType> associatedEcho;
    bool wavesCleared = false;

    // Visuals for Carrier Escape Zone
    sf::RectangleShape exitShape;

public:
    PreventChamber(Player& player, std::optional<EchoType> echoType = std::nullopt);
    virtual ~PreventChamber() = default;

    void update(float dt) override;
    void drawBackground(sf::RenderWindow& window) override;
    void onEnemyHit(Enemy* enemy, bool lethal) override;

    void setExitPosition(const sf::Vector2f& pos) override;
};

#endif // PREVENT_CHAMBER_HPP
