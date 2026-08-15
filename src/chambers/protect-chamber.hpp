#ifndef PROTECT_CHAMBER_HPP
#define PROTECT_CHAMBER_HPP

#include "chamber.hpp"
#include "../economy/echo.hpp"
#include "../ui/widgets/enemy-health-bar.hpp"
#include <memory>

class ProtectChamber : public Chamber {
private:
    std::unique_ptr<Echo> echo;
    sf::Vector2f echoPosition;
    float collectionTimer;
    float requiredCollectionTime;
    bool isCollected;

    // UI Collector Timer using EnemyHealthBar
    UI::EnemyHealthBar collectorTimerBar;

    // Visuals for Echo
    sf::CircleShape echoShape;
    sf::CircleShape radiusShape;

public:
    ProtectChamber(Player& player, const std::string& echoName, float requiredTime);
    virtual ~ProtectChamber() = default;

    void update(float dt) override;
    void drawBackground(sf::RenderWindow& window) override;
    void drawForeground(sf::RenderWindow& window) override;
    void onEnemyHit(Enemy* enemy, bool lethal) override;

    void setEchoPosition(sf::Vector2f pos);
    void setRequiredCollectionTime(float time) { requiredCollectionTime = time; }
    void setIsNoiseHall(bool active) { isNoiseHall = active; }
    void setIsReliquaryDecoy(bool active) { isReliquaryDecoy = active; }
    
    int processPlayerAttack(const Hitbox& hitbox) override;
    
    // Echo mechanics
    void onEchoHit(float rawDamage);
    void onFragmentCollected(float value) override;
    bool checkIronshellRedirect();

    Echo* getEcho() const { return echo.get(); }
    bool getIsCollected() const { return isCollected; }

private:
    bool isNoiseHall = false;
    bool isReliquaryDecoy = false;
    int noiseStalkerCount = 0;
};

#endif // PROTECT_CHAMBER_HPP
