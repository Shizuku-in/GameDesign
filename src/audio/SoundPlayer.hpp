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
    /// 从共享音效缓存初始化各音效槽位与播放对象池。
    explicit SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds);

    /// 每帧调用，递减所有冷却计时器。
    void update(float dt);

    /// 通过 ID 播放音效。
    void play(Config::SoundId id);

    /// 播放射击音效。
    void shoot() { play(Config::SoundId::Shoot); }
    /// 播放命中音效。
    void hit() { play(Config::SoundId::Hit); }
    /// 播放击杀音效。
    void kill() { play(Config::SoundId::Kill); }
    /// 播放玩家受伤音效。
    void hurt() { play(Config::SoundId::Hurt); }
    /// 播放经验宝石拾取音效。
    void pickup() { play(Config::SoundId::Pickup); }
    /// 播放升级音效。
    void levelup() { play(Config::SoundId::LevelUp); }

private:
    /// 音效枚举对应的槽位数量。
    static constexpr std::size_t K_COUNT = static_cast<std::size_t>(Config::SoundId::Count);

    /// 一种音效的缓冲区引用与播放配置。
    struct Slot {
        /// 音效数据缓冲；空值表示资源加载失败。
        sf::SoundBuffer* buffer = nullptr;
        /// 两次触发间的最短间隔（秒）；零表示不限制。
        float interval = 0.f;
        /// 播放音量，范围为 0 至 100。
        float volume = 100.f;
    };

    /// 按 SoundId 索引的音效配置。
    std::array<Slot, K_COUNT> m_slots{};
    /// 按 SoundId 索引的剩余触发冷却时间。
    std::array<float, K_COUNT> m_timers{};

    /// 循环复用的 SFML 播放实例池，支持同一音效叠加播放。
    std::vector<sf::Sound> m_pool;
    /// 下一个要复用的播放实例索引。
    int m_next = 0;
};
