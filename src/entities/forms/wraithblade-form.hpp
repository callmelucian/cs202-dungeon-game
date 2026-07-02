#ifndef WRAITHBLADE_FORM_HPP
#define WRAITHBLADE_FORM_HPP

#include "../player-form.hpp"

class WraithbladeForm : public PlayerForm {
public:
    WraithbladeForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    void triggerSpecial1(Player& player, Chamber& chamber) override;
    void triggerSpecial2(Player& player, Chamber& chamber) override;
};

#endif // WRAITHBLADE_FORM_HPP
