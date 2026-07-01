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

    // 防御：tile 尺寸必须为正，避免后续除零
    if (m_tileWidth <= 0 || m_tileHeight <= 0) {
        std::fprintf(stderr, "[WARN] TilemapRenderer: invalid tile size (%dx%d) in %s\n",
                     m_tileWidth, m_tileHeight, tmxPath);
        return false;
    }

    // 计算 tileset 行列数
    m_tilesetCols = static_cast<int>(m_tileset.getSize().x) / m_tileWidth;
    m_tilesetRows = static_cast<int>(m_tileset.getSize().y) / m_tileHeight;

    // 防御：纹理尺寸必须至少容纳一个 tile
    if (m_tilesetCols <= 0 || m_tilesetRows <= 0) {
        std::fprintf(stderr,
                     "[WARN] TilemapRenderer: tileset texture (%ux%u) too small for tile size "
                     "(%dx%d)\n",
                     m_tileset.getSize().x, m_tileset.getSize().y, m_tileWidth, m_tileHeight);
        return false;
    }

    // 从 TMX 读取 firstGID，而非硬编码假设为 1
    const auto& tilesets = map.getTilesets();
    if (!tilesets.empty()) {
        m_firstGid = tilesets[0].getFirstGID();
        if (tilesets.size() > 1)
            std::fprintf(
                stderr,
                "[WARN] TilemapRenderer: %zu tilesets found, only first used (firstGID=%u)\n",
                tilesets.size(), m_firstGid);
    }

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
                std::fprintf(stderr,
                             "[INFO] TilemapRenderer: loaded %s (%dx%d tiles, firstGID=%u)\n",
                             tmxPath, m_mapWidth, m_mapHeight, m_firstGid);
                return true;
            }
        }
    }

    std::fprintf(stderr, "[WARN] TilemapRenderer: no tile layer found in %s\n", tmxPath);
    return false;
}

void TilemapRenderer::buildVertices(const std::vector<std::uint32_t>& tiles) {
    const int totalTiles = m_tilesetCols * m_tilesetRows;
    const int firstGid = static_cast<int>(m_firstGid);

    // 第一遍：统计有效 tile 数（跳过空 tile 和越界 GID），以精确分配顶点
    std::size_t validCount = 0;
    for (int y = 0; y < m_mapHeight; ++y) {
        for (int x = 0; x < m_mapWidth; ++x) {
            std::uint32_t gid = tiles[static_cast<std::size_t>(y * m_mapWidth + x)];
            if (gid == 0)
                continue;
            int localId = static_cast<int>(gid) - firstGid;
            if (localId < 0 || localId >= totalTiles) {
                std::fprintf(stderr,
                             "[WARN] TilemapRenderer: GID %u out of range [%d, %d), skipping\n",
                             gid, firstGid, firstGid + totalTiles);
                continue;
            }
            ++validCount;
        }
    }

    m_vertices.clear();
    m_vertices.resize(validCount * 6); // 每有效瓦片 2 个三角形 = 6 个顶点，无浪费

    // 第二遍: 填充顶点
    std::size_t vi = 0;
    for (int y = 0; y < m_mapHeight; ++y) {
        for (int x = 0; x < m_mapWidth; ++x) {
            std::uint32_t gid = tiles[static_cast<std::size_t>(y * m_mapWidth + x)];
            if (gid == 0)
                continue;

            // GID → local index（通过 firstGid 偏移）
            int localId = static_cast<int>(gid) - firstGid;
            // 越界 GID 已在第一遍跳过，此处仅做二次保险
            if (localId < 0 || localId >= totalTiles)
                continue;

            // GID → tileset 纹理坐标
            int tx = (localId % m_tilesetCols) * m_tileWidth;
            int ty = (localId / m_tilesetCols) * m_tileHeight;

            // 世界坐标
            float px = static_cast<float>(x * m_tileWidth);
            float py = static_cast<float>(y * m_tileHeight);
            float tw = static_cast<float>(m_tileWidth);
            float th = static_cast<float>(m_tileHeight);

            // 6 个顶点（2 个三角形）
            m_vertices[vi + 0].position = {px, py};
            m_vertices[vi + 1].position = {px + tw, py};
            m_vertices[vi + 2].position = {px, py + th};
            m_vertices[vi + 3].position = {px + tw, py};
            m_vertices[vi + 4].position = {px + tw, py + th};
            m_vertices[vi + 5].position = {px, py + th};

            float u0 = static_cast<float>(tx);
            float v0 = static_cast<float>(ty);
            float u1 = static_cast<float>(tx + m_tileWidth);
            float v1 = static_cast<float>(ty + m_tileHeight);

            m_vertices[vi + 0].texCoords = {u0, v0};
            m_vertices[vi + 1].texCoords = {u1, v0};
            m_vertices[vi + 2].texCoords = {u0, v1};
            m_vertices[vi + 3].texCoords = {u1, v0};
            m_vertices[vi + 4].texCoords = {u1, v1};
            m_vertices[vi + 5].texCoords = {u0, v1};

            vi += 6;
        }
    }
}

void TilemapRenderer::draw(sf::RenderWindow& window) {
    sf::RenderStates states;
    states.texture = &m_tileset;
    window.draw(m_vertices, states);
}
