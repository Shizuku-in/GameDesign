#pragma once

#include <cstdint>
#include <vector>

/// 通用 freelist 对象池，使用 generation-count 句柄。
///
/// T 必须可默认构造。句柄使用 {idx, gen} 对防止悬垂引用：
/// 释放后重新分配同一槽位时 generation 已变，旧句柄验证失败。
template <typename T> class Pool {
public:
    struct Handle {
        std::uint32_t idx;
        std::uint32_t gen;
    };

    /// 申请一个槽位（优先从 freelist 弹出，否则扩容）。
    /// 返回数据为默认构造；调用者应立即覆盖所有字段。
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
        if (gen == 0) {
            gen = m_nextGen++; // gen==0 保留给"空闲"状态，跳过
        }

        m_slots[idx].gen = gen;
        m_slots[idx].data = T{}; // 初始化为默认状态
        return {idx, gen};
    }

    /// 释放槽位回 freelist。句柄立即失效（其 generation 永远不会再匹配此槽位）。
    void release(Handle h) {
        if (!valid(h)) {
            return;
        }
        m_slots[h.idx].gen = 0;
        m_freelist.push_back(h.idx);
    }

    /// 返回数据指针，句柄过期则返回 nullptr。
    T* get(Handle h) {
        if (!valid(h)) {
            return nullptr;
        }
        return &m_slots[h.idx].data;
    }

    [[nodiscard]] const T* get(Handle h) const {
        if (!valid(h)) {
            return nullptr;
        }
        return &m_slots[h.idx].data;
    }

    /// 对所有占用槽位调用 fn(T&)。
    template <typename F> void forEach(F&& fn) {
        for (auto& slot : m_slots) {
            if (slot.gen != 0) {
                fn(slot.data);
            }
        }
    }

    template <typename F> void forEach(F&& fn) const {
        for (const auto& slot : m_slots) {
            if (slot.gen != 0) {
                fn(slot.data);
            }
        }
    }

    /// 对所有占用槽位调用 fn(Handle, T&)。可在回调中安全调用 release()
    /// （已释放的槽位在当前遍历的剩余部分会被跳过）。
    template <typename F> void forEachHandle(F&& fn) {
        for (std::uint32_t i = 0; i < m_slots.size(); ++i) {
            auto& slot = m_slots[i];
            if (slot.gen != 0) {
                Handle h{i, slot.gen};
                fn(h, slot.data);
            }
        }
    }

    /// 当前占用槽位数。
    [[nodiscard]] std::size_t activeCount() const { return m_slots.size() - m_freelist.size(); }

    /// 总容量（占用 + 空闲）。
    [[nodiscard]] std::size_t capacity() const { return m_slots.size(); }

    /// 清空所有槽位和 freelist。
    void clear() {
        m_slots.clear();
        m_freelist.clear();
        m_nextGen = 1;
    }

    /// 预分配至少 n 个槽位的存储空间。
    void reserve(std::size_t n) { m_slots.reserve(n); }

private:
    struct Slot {
        T data{};
        std::uint32_t gen = 0; // 0 = 空闲; >0 = 占用（与 Handle::gen 匹配）
    };

    [[nodiscard]] bool valid(Handle h) const {
        return h.idx < m_slots.size() && h.gen != 0 && m_slots[h.idx].gen == h.gen;
    }

    std::vector<Slot> m_slots;
    std::vector<std::uint32_t> m_freelist;
    std::uint32_t m_nextGen = 1;
};
