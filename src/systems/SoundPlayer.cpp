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

    m_pool.reserve(POOL_SIZE);
    for (int i = 0; i < POOL_SIZE; ++i) {
        m_pool.emplace_back(*m_shoot);
        m_pool.back().stop();
    }
}

void SoundPlayer::update(float dt) {
    if (m_killTimer > 0.f)
        m_killTimer -= dt;
    if (m_hitTimer > 0.f)
        m_hitTimer -= dt;
    if (m_shootTimer > 0.f)
        m_shootTimer -= dt;
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

void SoundPlayer::shoot() {
    if (m_shootTimer > 0.f) return;
    m_shootTimer = 0.08f; // 发射间隔短，保护也短
    play(m_shoot, 50.f);
}

void SoundPlayer::hit() {
    if (m_hitTimer > 0.f) return;
    m_hitTimer = 0.05f;
    play(m_hit, 45.f);
}

void SoundPlayer::kill() {
    if (m_killTimer > 0.f) return;
    m_killTimer = 0.10f; // 击杀间隔稍长，避免堆叠
    play(m_kill, 55.f);
}

void SoundPlayer::hurt() { play(m_hurt, 70.f); }
void SoundPlayer::pickup() { play(m_pickup, 40.f); }
void SoundPlayer::levelup() { play(m_levelup, 65.f); }
