#include "audio/SoundPlayer.hpp"

#include "data/Constants.hpp"

#include <cstdio>

SoundPlayer::SoundPlayer(ResourceManager<sf::SoundBuffer>& sounds) {
    auto load = [&](Config::SoundId id) -> sf::SoundBuffer* {
        auto ptr = sounds.get(Config::soundKey(id));
        return ptr.get();
    };

    m_slots[static_cast<int>(Config::SoundId::Shoot)] = {
        .buffer = load(Config::SoundId::Shoot),
        .interval = Config::SOUND_CFG_SHOOT.interval,
        .volume = Config::SOUND_CFG_SHOOT.volume,
    };
    m_slots[static_cast<int>(Config::SoundId::Hit)] = {
        .buffer = load(Config::SoundId::Hit),
        .interval = Config::SOUND_CFG_HIT.interval,
        .volume = Config::SOUND_CFG_HIT.volume,
    };
    m_slots[static_cast<int>(Config::SoundId::Kill)] = {
        .buffer = load(Config::SoundId::Kill),
        .interval = Config::SOUND_CFG_KILL.interval,
        .volume = Config::SOUND_CFG_KILL.volume,
    };
    m_slots[static_cast<int>(Config::SoundId::Hurt)] = {
        .buffer = load(Config::SoundId::Hurt),
        .interval = Config::SOUND_CFG_HURT.interval,
        .volume = Config::SOUND_CFG_HURT.volume,
    };
    m_slots[static_cast<int>(Config::SoundId::Pickup)] = {
        .buffer = load(Config::SoundId::Pickup),
        .interval = Config::SOUND_CFG_PICKUP.interval,
        .volume = Config::SOUND_CFG_PICKUP.volume,
    };
    m_slots[static_cast<int>(Config::SoundId::LevelUp)] = {
        .buffer = load(Config::SoundId::LevelUp),
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

void SoundPlayer::play(Config::SoundId id) {
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
