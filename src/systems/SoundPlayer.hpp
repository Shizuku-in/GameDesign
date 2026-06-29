#pragma once

#include "core/ResourceManager.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <memory>
#include <vector>

/// 小型 sf::Sound 对象池，封装 buffer 引用，通过命名方法播放。
class SoundPlayer {
public:
    static constexpr int POOL_SIZE = 24;

    explicit SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds);

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
};
