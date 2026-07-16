#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

/// 按字符串键缓存可共享资源的泛型资源管理器。
template <typename Resource> class ResourceManager {
public:
    /// 资源共享指针类型，保证从缓存取得的资源可安全共享。
    using Ptr = std::shared_ptr<Resource>;

    /// 加载或获取已缓存的资源。
    /// 首次访问时用 `args` 构造资源并缓存；后续调用返回缓存实例。
    template <typename... Args> Ptr load(const std::string& key, Args&&... args) {
        auto it = m_resources.find(key);
        if (it != m_resources.end()) {
            return it->second;
        }

        auto resource = std::make_shared<Resource>(std::forward<Args>(args)...);
        m_resources.emplace(key, resource);
        return resource;
    }

    /// 按键获取已缓存资源；键不存在时抛出 std::runtime_error。
    Ptr get(const std::string& key) const {
        auto it = m_resources.find(key);
        if (it == m_resources.end()) {
            throw std::runtime_error("Resource not found: " + key);
        }
        return it->second;
    }

    /// 判断指定键是否已有缓存资源。
    [[nodiscard]] bool has(const std::string& key) const { return m_resources.contains(key); }

    /// 删除指定键的缓存资源。
    void remove(const std::string& key) { m_resources.erase(key); }

    /// 删除全部缓存资源。
    void clear() { m_resources.clear(); }

private:
    /// 从资源键到共享资源实例的缓存表。
    std::unordered_map<std::string, Ptr> m_resources;
};
