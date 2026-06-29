#include "systems/SoundPlayer.hpp"

#include "data/Constants.hpp"

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

    m_pool.reserve(Config::POOL_SOUNDS_CAPACITY);
    for (int i = 0; i < static_cast<int>(Config::POOL_SOUNDS_CAPACITY); ++i) {
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
    for (int i = 0; i < static_cast<int>(Config::POOL_SOUNDS_CAPACITY); ++i) {
        int idx = (m_next + i) % Config::POOL_SOUNDS_CAPACITY;
        if (m_pool[idx].getStatus() == sf::Sound::Status::Stopped) {
            m_pool[idx].setBuffer(*buf);
            m_pool[idx].setVolume(volume);
            m_pool[idx].play();
            m_next = (idx + 1) % Config::POOL_SOUNDS_CAPACITY;
            return;
        }
    }
}

void SoundPlayer::shoot() {
    if (m_shootTimer > 0.f)
        return;
    m_shootTimer = Config::SOUND_CFG_SHOOT.interval;
    play(m_shoot, Config::SOUND_CFG_SHOOT.volume);
}
void SoundPlayer::hit() {
    if (m_hitTimer > 0.f)
        return;
    m_hitTimer = Config::SOUND_CFG_HIT.interval;
    play(m_hit, Config::SOUND_CFG_HIT.volume);
}
void SoundPlayer::kill() {
    if (m_killTimer > 0.f)
        return;
    m_killTimer = Config::SOUND_CFG_KILL.interval;
    play(m_kill, Config::SOUND_CFG_KILL.volume);
}
void SoundPlayer::hurt() { play(m_hurt, Config::SOUND_CFG_HURT.volume); }
void SoundPlayer::pickup() { play(m_pickup, Config::SOUND_CFG_PICKUP.volume); }
void SoundPlayer::levelup() { play(m_levelup, Config::SOUND_CFG_LEVELUP.volume); }
