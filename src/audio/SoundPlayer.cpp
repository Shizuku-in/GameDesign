#include "audio/SoundPlayer.hpp"

#include "data/Constants.hpp"

#include <cstdio>

SoundPlayer::SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds) {
    auto load = [&](const char* key) -> sf::SoundBuffer* {
        auto ptr = sounds.get(key);
        return ptr.get();
    };

    m_slots[static_cast<int>(SoundId::Shoot)] = {
        .buffer = load("shoot"),
        .interval = Config::SOUND_CFG_SHOOT.interval,
        .volume = Config::SOUND_CFG_SHOOT.volume,
    };
    m_slots[static_cast<int>(SoundId::Hit)] = {
        .buffer = load("hit"),
        .interval = Config::SOUND_CFG_HIT.interval,
        .volume = Config::SOUND_CFG_HIT.volume,
    };
    m_slots[static_cast<int>(SoundId::Kill)] = {
        .buffer = load("kill"),
        .interval = Config::SOUND_CFG_KILL.interval,
        .volume = Config::SOUND_CFG_KILL.volume,
    };
    m_slots[static_cast<int>(SoundId::Hurt)] = {
        .buffer = load("hurt"),
        .interval = Config::SOUND_CFG_HURT.interval,
        .volume = Config::SOUND_CFG_HURT.volume,
    };
    m_slots[static_cast<int>(SoundId::Pickup)] = {
        .buffer = load("pickup"),
        .interval = Config::SOUND_CFG_PICKUP.interval,
        .volume = Config::SOUND_CFG_PICKUP.volume,
    };
    m_slots[static_cast<int>(SoundId::LevelUp)] = {
        .buffer = load("levelup"),
        .interval = Config::SOUND_CFG_LEVELUP.interval,
        .volume = Config::SOUND_CFG_LEVELUP.volume,
    };

    m_pool.reserve(Config::POOL_SOUNDS_CAPACITY);
    for (int i = 0; i < static_cast<int>(Config::POOL_SOUNDS_CAPACITY); ++i) {
        m_pool.emplace_back(*m_slots[0].buffer);
        m_pool.back().stop();
    }
}

void SoundPlayer::update(float dt) {
    for (auto& timer : m_timers) {
        if (timer > 0.f) {
            timer -= dt;
        }
    }
}

void SoundPlayer::play(SoundId id) {
    const auto& slot = m_slots[static_cast<int>(id)];
    if (slot.buffer == nullptr) {
        return;
    }

    auto& timer = m_timers[static_cast<int>(id)];
    if (timer > 0.f) {
        return;
    }
    timer = slot.interval;

    for (int i = 0; i < static_cast<int>(Config::POOL_SOUNDS_CAPACITY); ++i) {
        int idx = (m_next + i) % Config::POOL_SOUNDS_CAPACITY;
        if (m_pool[idx].getStatus() == sf::Sound::Status::Stopped) {
            m_pool[idx].setBuffer(*slot.buffer);
            m_pool[idx].setVolume(slot.volume);
            m_pool[idx].play();
            m_next = (idx + 1) % Config::POOL_SOUNDS_CAPACITY;
            return;
        }
    }
    std::fprintf(stderr, "[WARN] SoundPlayer: pool exhausted (%zu slots), sound dropped\n",
                 Config::POOL_SOUNDS_CAPACITY);
}
