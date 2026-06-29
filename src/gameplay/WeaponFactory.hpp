#pragma once

#include "systems/IWeaponBehavior.hpp"
#include <memory>

class WeaponFactory {
public:
    static std::unique_ptr<IWeaponBehavior> create(WeaponType type);
};
