#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

template <typename Resource> class ResourceManager {
public:
    using Ptr = std::shared_ptr<Resource>;

    /// 加载或获取已缓存的资源。
    /// 首次访问时用 `args` 构造资源并缓存；后续调用返回缓存实例。
    template <typename... Args> Ptr load(const std::string& key, Args&&... args) {
        auto it = m_resources.find(key);
        if (it != m_resources.end())
            return it->second;

        auto resource = std::make_shared<Resource>(std::forward<Args>(args)...);
        m_resources.emplace(key, resource);
        return resource;
    }

    /// Retrieve a resource by key. Throws if not found.
    Ptr get(const std::string& key) const {
        auto it = m_resources.find(key);
        if (it == m_resources.end())
            throw std::runtime_error("Resource not found: " + key);
        return it->second;
    }

    /// Check whether a key exists.
    bool has(const std::string& key) const { return m_resources.contains(key); }

    /// Remove a single resource.
    void remove(const std::string& key) { m_resources.erase(key); }

    /// Remove all cached resources.
    void clear() { m_resources.clear(); }

private:
    std::unordered_map<std::string, Ptr> m_resources;
};
