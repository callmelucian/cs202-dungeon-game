#ifndef ITEM_HPP
#define ITEM_HPP

#include <SFML/Graphics.hpp>

class Player;
class Chamber;

class Item : public sf::Drawable, public sf::Transformable {
protected:
    sf::Vector2f velocity;
    float drag;
    float magnetRadius;
    float collectionRadius;
    float lifetime;
    bool collected;
    
    // Visuals
    sf::Sprite sprite;

public:
    Item(sf::Vector2f startPos, sf::IntRect textureRect);
    virtual ~Item() = default;

    virtual void update(float dt, const sf::Vector2f& playerPos);
    virtual void onCollect(Player& player, Chamber& chamber) = 0;
    
    bool isCollected() const;
    sf::FloatRect getBounds() const;

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

class EchoFragment : public Item {
private:
    float value;

public:
    EchoFragment(sf::Vector2f startPos, float value);
    
    void onCollect(Player& player, Chamber& chamber) override;
};

class FreezePotion : public Item {
public:
    FreezePotion(sf::Vector2f startPos);
    void onCollect(Player& player, Chamber& chamber) override;
};

class SpeedPotion : public Item {
public:
    SpeedPotion(sf::Vector2f startPos);
    void onCollect(Player& player, Chamber& chamber) override;
};

class AntidotePotion : public Item {
public:
    AntidotePotion(sf::Vector2f startPos);
    void onCollect(Player& player, Chamber& chamber) override;
};

class HealthPotion : public Item {
public:
    HealthPotion(sf::Vector2f startPos);
    void onCollect(Player& player, Chamber& chamber) override;
};

#endif // ITEM_HPP
