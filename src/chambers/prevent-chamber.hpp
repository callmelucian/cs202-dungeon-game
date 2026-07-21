#ifndef PREVENT_CHAMBER_HPP
#define PREVENT_CHAMBER_HPP

#include "chamber.hpp"
#include "../core/enums.hpp"
#include "../core/game.hpp"
#include <memory>

class PreventChamber : public Chamber {
private:
    EchoType associatedEcho;
    sf::Vector2f exitPosition;

    // Visuals for Exit
    sf::RectangleShape exitShape;

public:
    PreventChamber(Player& player, EchoType echoType);
    virtual ~PreventChamber() = default;

    void update(float dt) override;
    void drawBackground(sf::RenderWindow& window) override;
    void onEnemyHit(Enemy* enemy, bool lethal) override;

    void setExitPosition(sf::Vector2f pos);
};

#endif // PREVENT_CHAMBER_HPP
