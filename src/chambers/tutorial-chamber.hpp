#ifndef TUTORIAL_CHAMBER_HPP
#define TUTORIAL_CHAMBER_HPP

#include "chamber.hpp"
#include <functional>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

// TutorialChamber: Specialized chamber featuring 8 distinct instructional training islands.
// Renders short, crisp floor instructions directly onto the map, manages training dummies,
// form switching guide triggers, momentum replenishment, and exit completion.
class TutorialChamber : public Chamber {
public:
    explicit TutorialChamber(Player& player);
    virtual ~TutorialChamber() override;

    void update(float dt) override;
    void drawForeground(sf::RenderWindow& window) override;

    void setFormGuideCallback(std::function<void()> callback);

protected:
    void completeChamber() override;

private:
    struct FloorLabel {
        sf::Vector2f position;
        std::string keyText;
        std::string instructionText;
    };

    std::function<void()> formGuideCallback;
    bool formGuideTriggered = false;
    bool island6Active = false;

    // Island 6 Respawning Dummy management
    struct DummySpawnSlot {
        sf::Vector2f position;
        float maxHp;
        bool isAlive;
        float respawnTimer;
        class DummyEnemy* currentEnemy = nullptr;
    };
    std::vector<DummySpawnSlot> dummySlots;

    // Island 5 isolated enemy
    class DummyEnemy* isolatedEnemy = nullptr;
    bool isolatedEnemySpawned = false;

    // Island 3 dummy
    class DummyEnemy* island3Dummy = nullptr;
    bool island3DummySpawned = false;

    std::vector<FloorLabel> floorLabels;

    void setupIslandsAndLabels();
    void spawnInitialEnemies();
    void updateIslandTriggers(float dt);
    void renderFloorLabel(sf::RenderWindow& window, const FloorLabel& label, const sf::Font& headerFont, const sf::Font& bodyFont) const;
};

#endif // TUTORIAL_CHAMBER_HPP
