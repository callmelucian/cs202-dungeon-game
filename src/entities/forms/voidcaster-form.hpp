#ifndef VOIDCASTER_FORM_HPP
#define VOIDCASTER_FORM_HPP

#include "../player-form.hpp"

class VoidcasterForm : public PlayerForm {
public:
    VoidcasterForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    void triggerSpecial1(Player& player, Chamber& chamber) override;
    void triggerSpecial2(Player& player, Chamber& chamber) override;
};

#endif // VOIDCASTER_FORM_HPP
