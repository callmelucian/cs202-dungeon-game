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
    void draw(sf::RenderWindow& window) override;
    void processPlayerAttack(const Hitbox& hitbox) override;

    void setExitPosition(sf::Vector2f pos);
    void onCarrierHit(Enemy* enemy, bool lethal);
};

#endif // PREVENT_CHAMBER_HPP
