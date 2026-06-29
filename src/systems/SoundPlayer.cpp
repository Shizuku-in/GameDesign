#include "systems/SoundPlayer.hpp"

SoundPlayer::SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds) {
    auto load = [&](const char* key) -> sf::SoundBuffer* {
        auto ptr = sounds.get(key);
        return ptr.get();
    };
    m_shoot = load("shoot");
    m_hit = load("hit");
    m_kill = load("kill");
    m_hurt = load("hurt");
    m_pickup = load("pickup");
    m_levelup = load("levelup");

    // sf::Sound 需要 buffer 才能构造，用 m_shoot 初始化全部然后 stop
    m_pool.reserve(POOL_SIZE);
    for (int i = 0; i < POOL_SIZE; ++i) {
        m_pool.emplace_back(*m_shoot);
        m_pool.back().stop();
    }
}

void SoundPlayer::play(const sf::SoundBuffer* buf, float volume) {
    if (!buf)
        return;
    for (int i = 0; i < POOL_SIZE; ++i) {
        int idx = (m_next + i) % POOL_SIZE;
        if (m_pool[idx].getStatus() == sf::Sound::Status::Stopped) {
            m_pool[idx].setBuffer(*buf);
            m_pool[idx].setVolume(volume);
            m_pool[idx].play();
            m_next = (idx + 1) % POOL_SIZE;
            return;
        }
    }
}

void SoundPlayer::shoot() { play(m_shoot, 50.f); } // 弹幕射击
void SoundPlayer::hit() { play(m_hit, 45.f); } // 命中
void SoundPlayer::kill() { play(m_kill, 55.f); } // 击杀
void SoundPlayer::hurt() { play(m_hurt, 70.f); } // 受击
void SoundPlayer::pickup() { play(m_pickup, 40.f); } // 拾取宝石
void SoundPlayer::levelup() { play(m_levelup, 65.f); } // 升级
