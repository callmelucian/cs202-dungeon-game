#ifndef PROTECT_CHAMBER_HPP
#define PROTECT_CHAMBER_HPP

#include "chamber.hpp"
#include "../economy/echo.hpp"
#include <memory>

class ProtectChamber : public Chamber {
private:
    std::unique_ptr<Echo> echo;
    sf::Vector2f echoPosition;
    float collectionTimer;
    float requiredCollectionTime;
    bool isCollected;

    // Visuals for Echo
    sf::CircleShape echoShape;
    sf::CircleShape radiusShape;

public:
    ProtectChamber(Player& player, const std::string& echoName, float requiredTime);
    virtual ~ProtectChamber() = default;

    void update(float dt) override;
    void drawBackground(sf::RenderWindow& window) override;
    void onEnemyHit(Enemy* enemy, bool lethal) override;

    void setEchoPosition(sf::Vector2f pos);
    
    // Echo mechanics
    void onEchoHit(float rawDamage);
    void onFragmentCollected(float value) override;
    bool checkIronshellRedirect();
    void applyWraithbladeKnockback(Enemy* enemy);

    Echo* getEcho() const { return echo.get(); }
    bool getIsCollected() const { return isCollected; }
};

#endif // PROTECT_CHAMBER_HPP
