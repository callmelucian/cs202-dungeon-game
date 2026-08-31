#ifndef EXIT_GATE_HPP
#define EXIT_GATE_HPP

#include <SFML/Graphics.hpp>

class Player;

/**
 * @class ExitGate
 * @brief Represents an interactive Exit Portal that triggers chamber completion when the player enters.
 */
class ExitGate {
public:
    ExitGate(sf::Vector2f pos = {-1.0f, -1.0f}, bool active = false);
    ~ExitGate() = default;

    void setPosition(sf::Vector2f pos);
    sf::Vector2f getPosition() const { return position; }

    void setActive(bool active);
    bool isActive() const { return active; }

    sf::FloatRect getBounds() const;

    /**
     * @brief Checks if the player's bounding box overlaps with this exit gate's bounding box by at least threshold (default 50%).
     * @param player The player instance to check.
     * @param threshold Required overlap ratio (0.5f for 50%).
     * @return True if overlap area / player bounds area >= threshold.
     */
    bool checkPlayerOverlap(const Player& player, float threshold = 0.5f) const;

    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    sf::Vector2f position;
    bool active;

    float pulseTimer;
    float particleTimer;

    sf::RectangleShape baseShape;
    sf::RectangleShape innerShape;
};

#endif // EXIT_GATE_HPP
