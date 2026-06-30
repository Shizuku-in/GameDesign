#pragma once

#include "systems/IWeaponBehavior.hpp"
#include <array>
#include <memory>

class WeaponFactory {
public:
    using FactoryFn = std::unique_ptr<IWeaponBehavior> (*)();
    static std::unique_ptr<IWeaponBehavior> create(WeaponType type);

private:
    static constexpr std::size_t kCount = static_cast<std::size_t>(WeaponType::Count);
    static const std::array<FactoryFn, kCount> s_factories;
};
