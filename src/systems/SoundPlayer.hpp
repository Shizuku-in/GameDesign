#pragma once

#include "core/ResourceManager.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <memory>
#include <vector>

/// 小型 sf::Sound 对象池，封装 buffer 引用，通过命名方法播放。
/// 同一音效有多短间隔保护，避免多个实例同时播放导致音量叠加。
class SoundPlayer {
public:
    static constexpr int POOL_SIZE = 24;

    explicit SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds);

    /// 每帧调用，递减冷却计时器。
    void update(float dt);

    void shoot();
    void hit();
    void kill();
    void hurt();
    void pickup();
    void levelup();

private:
    void play(const sf::SoundBuffer* buf, float volume);

    sf::SoundBuffer* m_shoot = nullptr;
    sf::SoundBuffer* m_hit = nullptr;
    sf::SoundBuffer* m_kill = nullptr;
    sf::SoundBuffer* m_hurt = nullptr;
    sf::SoundBuffer* m_pickup = nullptr;
    sf::SoundBuffer* m_levelup = nullptr;

    std::vector<sf::Sound> m_pool;
    int m_next = 0;

    // 同音效最短间隔（秒），避免多个实例叠加
    float m_killTimer = 0.f;
    float m_hitTimer = 0.f;
    float m_shootTimer = 0.f;
};
