#include "playable-character.hpp"
#include "forms/wraithblade-form.hpp"
#include "forms/voidcaster-form.hpp"
#include "forms/ironshell-form.hpp"

PlayableCharacter::PlayableCharacter(const std::string& name, const std::string& description)
    : name(name), description(description) {}

const std::string& PlayableCharacter::getName() const {
    return name;
}

const std::string& PlayableCharacter::getDescription() const {
    return description;
}

Serin::Serin()
    : PlayableCharacter("Serin", "The main character who can shift forms.") {}

std::unique_ptr<PlayerForm> Serin::createForm1() {
    return std::make_unique<WraithbladeForm>();
}

std::unique_ptr<PlayerForm> Serin::createForm2() {
    return std::make_unique<VoidcasterForm>();
}

std::unique_ptr<PlayerForm> Serin::createForm3() {
    return std::make_unique<IronshellForm>();
}
