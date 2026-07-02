#include "player-form.hpp"

PlayerForm::PlayerForm(FormType formType, float baseSpeed, float maxHpContribution, float baseDamage, float attackRange, float attackRate)
    : formType(formType),
      baseSpeed(baseSpeed),
      maxHpContribution(maxHpContribution),
      baseDamage(baseDamage),
      attackRange(attackRange),
      attackRate(attackRate) {}

FormStats PlayerForm::getStats() const {
    return FormStats{
        baseSpeed,
        maxHpContribution,
        baseDamage,
        attackRange,
        attackRate
    };
}

FormType PlayerForm::getFormType() const {
    return formType;
}
