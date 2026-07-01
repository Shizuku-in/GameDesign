#pragma once

#include "core/ResourceManager.hpp"
#include "data/Constants.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <array>
#include <memory>
#include <vector>

/// 小型 sf::Sound 对象池，封装 buffer 引用，表驱动播放。
/// 同一音效有多短间隔保护，避免多个实例同时播放导致音量叠加。
class SoundPlayer {
public:
    explicit SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds);

    /// 每帧调用，递减所有冷却计时器。
    void update(float dt);

    /// 通过 ID 播放音效。
    void play(Config::SoundId id);

    // 便捷方法
    void shoot() { play(Config::SoundId::Shoot); }
    void hit() { play(Config::SoundId::Hit); }
    void kill() { play(Config::SoundId::Kill); }
    void hurt() { play(Config::SoundId::Hurt); }
    void pickup() { play(Config::SoundId::Pickup); }
    void levelup() { play(Config::SoundId::LevelUp); }

private:
    static constexpr std::size_t K_COUNT = static_cast<std::size_t>(Config::SoundId::Count);

    struct Slot {
        sf::SoundBuffer* buffer = nullptr;
        float interval = 0.f; // 最短触发间隔（秒），0 = 无限制
        float volume = 100.f; // 0–100
    };

    std::array<Slot, K_COUNT> m_slots{};
    std::array<float, K_COUNT> m_timers{};

    std::vector<sf::Sound> m_pool;
    int m_next = 0;
};
