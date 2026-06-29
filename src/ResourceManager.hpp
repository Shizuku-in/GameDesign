#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

template <typename Resource>
class ResourceManager {
public:
    using Ptr = std::shared_ptr<Resource>;

    /// Load or retrieve a cached resource.
    /// On first access the resource is constructed from `args` and cached;
    /// subsequent calls return the cached instance.
    template <typename... Args>
    Ptr load(const std::string& key, Args&&... args)
    {
        auto it = m_resources.find(key);
        if (it != m_resources.end())
            return it->second;

        auto resource = std::make_shared<Resource>(std::forward<Args>(args)...);
        m_resources.emplace(key, resource);
        return resource;
    }

    /// Retrieve a resource by key. Throws if not found.
    Ptr get(const std::string& key) const
    {
        auto it = m_resources.find(key);
        if (it == m_resources.end())
            throw std::runtime_error("Resource not found: " + key);
        return it->second;
    }

    /// Check whether a key exists.
    bool has(const std::string& key) const
    {
        return m_resources.find(key) != m_resources.end();
    }

    /// Remove a single resource.
    void remove(const std::string& key)
    {
        m_resources.erase(key);
    }

    /// Remove all cached resources.
    void clear()
    {
        m_resources.clear();
    }

private:
    std::unordered_map<std::string, Ptr> m_resources;
};
