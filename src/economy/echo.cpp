#include "echo.hpp"

Echo::Echo(const std::string& name, float initialPower)
    : name(name), echoPower(initialPower) {}

void Echo::takeDamage(float penalty) {
    echoPower -= penalty;
    if (echoPower < 10.0f) echoPower = 10.0f;
    notify();
}

void Echo::addPower(float amount) {
    echoPower += amount;
    if (echoPower > 100.0f) echoPower = 100.0f;
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
