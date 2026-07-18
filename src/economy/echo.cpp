#include "echo.hpp"

Echo::Echo(const std::string& name, float initialPower)
    : name(name), echoPower(initialPower) {}

void Echo::takeDamage(float penalty) {
    echoPower -= penalty;
    if (echoPower < 0) echoPower = 0;
    notify();
}

void Echo::addPower(float amount) {
    echoPower += amount;
    notify();
}

float Echo::getPower() const {
    return echoPower;
}

void Echo::attach(EchoObserver* observer) {
    if (observer && std::find(observers.begin(), observers.end(), observer) == observers.end()) {
        observers.push_back(observer);
    }
}

void Echo::notify() {
    for (auto* observer : observers) {
        observer->onEchoPowerChanged(echoPower);
    }
}
