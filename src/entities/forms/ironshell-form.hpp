#ifndef IRONSHELL_FORM_HPP
#define IRONSHELL_FORM_HPP

#include "../player-form.hpp"

class IronshellForm : public PlayerForm {
public:
    IronshellForm();

    void attack(Player& player, sf::Vector2f targetDir, Chamber& chamber) override;
    void triggerSpecial1(Player& player, Chamber& chamber) override;
    void triggerSpecial2(Player& player, Chamber& chamber) override;
};

#endif // IRONSHELL_FORM_HPP
