#pragma once

#include <cstdint>
#include <vector>

/// Generic freelist object pool with generation-counted handles.
///
/// T must be default-constructible. Handles use an {idx, gen} pair to prevent
/// dangling-reference bugs: after release + re-acquire of the same slot, the
/// generation changes and old handles fail validation.
template <typename T> class Pool {
public:
    struct Handle {
        std::uint32_t idx;
        std::uint32_t gen;
    };

    /// Acquire a slot (from freelist or by growing). The returned data is
    /// default-constructed; caller should overwrite fields immediately.
    Handle acquire() {
        std::uint32_t idx;
        if (!m_freelist.empty()) {
            idx = m_freelist.back();
            m_freelist.pop_back();
        } else {
            idx = static_cast<std::uint32_t>(m_slots.size());
            m_slots.emplace_back();
        }

        std::uint32_t gen = m_nextGen++;
        if (gen == 0)
            gen = m_nextGen++; // never assign gen==0 (reserved for "free")

        m_slots[idx].gen = gen;
        m_slots[idx].data = T{}; // fresh default state
        return {idx, gen};
    }

    /// Release a slot back to the freelist. The handle is invalidated
    /// (its generation will never match this slot again).
    void release(Handle h) {
        if (!valid(h))
            return;
        m_slots[h.idx].gen = 0;
        m_freelist.push_back(h.idx);
    }

    /// Return a pointer to the data, or nullptr if the handle is stale.
    T* get(Handle h) {
        if (!valid(h))
            return nullptr;
        return &m_slots[h.idx].data;
    }

    const T* get(Handle h) const {
        if (!valid(h))
            return nullptr;
        return &m_slots[h.idx].data;
    }

    /// Call fn(T&) for every occupied slot.
    template <typename F> void forEach(F&& fn) {
        for (auto& slot : m_slots) {
            if (slot.gen != 0)
                fn(slot.data);
        }
    }

    template <typename F> void forEach(F&& fn) const {
        for (const auto& slot : m_slots) {
            if (slot.gen != 0)
                fn(slot.data);
        }
    }

    /// Call fn(Handle, T&) for every occupied slot. Safe to call release()
    /// on the handle from inside the callback (the released slot is skipped
    /// for the remainder of this iteration).
    template <typename F> void forEachHandle(F&& fn) {
        for (std::uint32_t i = 0; i < m_slots.size(); ++i) {
            auto& slot = m_slots[i];
            if (slot.gen != 0) {
                Handle h{i, slot.gen};
                fn(h, slot.data);
            }
        }
    }

    /// Number of currently occupied slots.
    std::size_t activeCount() const { return m_slots.size() - m_freelist.size(); }

    /// Total capacity (occupied + free).
    std::size_t capacity() const { return m_slots.size(); }

    /// Drop all slots and freelist entries.
    void clear() {
        m_slots.clear();
        m_freelist.clear();
        m_nextGen = 1;
    }

    /// Pre-allocate storage for at least `n` total slots.
    void reserve(std::size_t n) { m_slots.reserve(n); }

private:
    struct Slot {
        T data{};
        std::uint32_t gen = 0; // 0 = free; >0 = occupied (matches Handle::gen)
    };

    bool valid(Handle h) const {
        return h.idx < m_slots.size() && h.gen != 0 && m_slots[h.idx].gen == h.gen;
    }

    std::vector<Slot> m_slots;
    std::vector<std::uint32_t> m_freelist;
    std::uint32_t m_nextGen = 1;
};
