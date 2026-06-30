#include "graphics/TilemapRenderer.hpp"

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

#include <cstdio>

bool TilemapRenderer::loadFromFile(const char* tmxPath, const char* tilesetPath) {
    // 加载 TMX
    tmx::Map map;
    if (!map.load(tmxPath)) {
        std::fprintf(stderr, "[WARN] TilemapRenderer: failed to load TMX: %s\n", tmxPath);
        return false;
    }

    // 加载 tileset 纹理
    if (!m_tileset.loadFromFile(tilesetPath)) {
        std::fprintf(stderr, "[WARN] TilemapRenderer: failed to load tileset: %s\n", tilesetPath);
        return false;
    }
    m_tileset.setSmooth(false);

    // 读取地图属性
    const auto& mapSize = map.getTileCount();
    m_mapWidth = mapSize.x;
    m_mapHeight = mapSize.y;
    const auto& tileSize = map.getTileSize();
    m_tileWidth = tileSize.x;
    m_tileHeight = tileSize.y;
    m_tilesetCols = static_cast<int>(m_tileset.getSize().x) / m_tileWidth;

    // 找到地面层并构建顶点
    for (const auto& layer : map.getLayers()) {
        if (layer->getType() == tmx::Layer::Type::Tile) {
            const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
            const auto& tiles = tileLayer.getTiles();
            if (!tiles.empty()) {
                // 将 tmx::Tile 转为 GID 数组
                std::vector<std::uint32_t> gids;
                gids.reserve(tiles.size());
                for (const auto& t : tiles)
                    gids.push_back(t.ID);
                buildVertices(gids);
                std::fprintf(stderr, "[INFO] TilemapRenderer: loaded %s (%dx%d tiles)\n", tmxPath,
                             m_mapWidth, m_mapHeight);
                return true;
            }
        }
    }

    std::fprintf(stderr, "[WARN] TilemapRenderer: no tile layer found in %s\n", tmxPath);
    return false;
}

void TilemapRenderer::buildVertices(const std::vector<std::uint32_t>& tiles) {
    m_vertices.clear();
    m_vertices.resize(tiles.size() * 6); // 每瓦片 2 个三角形 = 6 个顶点

    for (int y = 0; y < m_mapHeight; ++y) {
        for (int x = 0; x < m_mapWidth; ++x) {
            std::uint32_t gid = tiles[static_cast<std::size_t>(y * m_mapWidth + x)];
            if (gid == 0)
                continue; // 空瓦片

            // GID → tileset 纹理坐标（GID 从 1 开始）
            int localId = static_cast<int>(gid) - 1;
            int tx = (localId % m_tilesetCols) * m_tileWidth;
            int ty = (localId / m_tilesetCols) * m_tileHeight;

            // 世界坐标
            float px = static_cast<float>(x * m_tileWidth);
            float py = static_cast<float>(y * m_tileHeight);
            float tw = static_cast<float>(m_tileWidth);
            float th = static_cast<float>(m_tileHeight);

            // 6 个顶点（2 个三角形）
            auto idx = static_cast<std::size_t>((y * m_mapWidth + x) * 6);
            m_vertices[idx + 0].position = {px, py};
            m_vertices[idx + 1].position = {px + tw, py};
            m_vertices[idx + 2].position = {px, py + th};
            m_vertices[idx + 3].position = {px + tw, py};
            m_vertices[idx + 4].position = {px + tw, py + th};
            m_vertices[idx + 5].position = {px, py + th};

            float u0 = static_cast<float>(tx);
            float v0 = static_cast<float>(ty);
            float u1 = static_cast<float>(tx + m_tileWidth);
            float v1 = static_cast<float>(ty + m_tileHeight);

            m_vertices[idx + 0].texCoords = {u0, v0};
            m_vertices[idx + 1].texCoords = {u1, v0};
            m_vertices[idx + 2].texCoords = {u0, v1};
            m_vertices[idx + 3].texCoords = {u1, v0};
            m_vertices[idx + 4].texCoords = {u1, v1};
            m_vertices[idx + 5].texCoords = {u0, v1};
        }
    }
}

void TilemapRenderer::draw(sf::RenderWindow& window) {
    sf::RenderStates states;
    states.texture = &m_tileset;
    window.draw(m_vertices, states);
}
