#include "graphics/TilemapRenderer.hpp"

#include <tmxlite/LayerGroup.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Tileset.hpp>

#include <SFML/Graphics/Color.hpp>

#include <cstdio>
#include <filesystem>

// ---------------------------------------------------------------------------
// 公开 API
// ---------------------------------------------------------------------------

bool TilemapRenderer::loadFromFile(const char* tmxPath) {
    // 先清空所有成员，避免失败后残留半初始化状态
    m_mapWidth = 0;
    m_mapHeight = 0;
    m_tileWidth = 0;
    m_tileHeight = 0;
    m_tilesets.clear();
    m_batches.clear();
    m_objects.clear();

    // 1. 加载 TMX
    tmx::Map map;
    if (!map.load(tmxPath)) {
        std::fprintf(stderr, "[WARN] TilemapRenderer: failed to load TMX: %s\n", tmxPath);
        return false;
    }

    if (map.isInfinite()) {
        std::fprintf(stderr, "[WARN] TilemapRenderer: infinite maps not supported: %s\n", tmxPath);
        return false;
    }

    // 2. 读取地图尺寸（先用局部变量，成功后再提交到成员）
    const auto& mapSize = map.getTileCount();
    const int mapW = mapSize.x;
    const int mapH = mapSize.y;
    const auto& tileSize = map.getTileSize();
    const int tileW = tileSize.x;
    const int tileH = tileSize.y;

    if (tileW <= 0 || tileH <= 0) {
        std::fprintf(stderr, "[WARN] TilemapRenderer: invalid tile size (%dx%d) in %s\n", tileW,
                     tileH, tmxPath);
        return false;
    }

    // 3. 加载所有 tileset 纹理
    m_tilesets.clear();
    const auto& tilesets = map.getTilesets();
    if (tilesets.empty())
        std::fprintf(stderr, "[WARN] TilemapRenderer: no tilesets found in %s\n", tmxPath);

    std::filesystem::path tmxDir(map.getWorkingDirectory());

    for (const auto& ts : tilesets) {
        const auto& imgPath = ts.getImagePath();
        if (imgPath.empty()) {
            std::fprintf(stderr,
                         "[WARN] TilemapRenderer: tileset '%s' has no image (collection not "
                         "supported), skipping\n",
                         ts.getName().c_str());
            continue;
        }

        // 尝试直接使用 ts.getImagePath()（tmxlite 已相对工作目录解析），
        // 若文件不存在则拼接 TMX 目录作为兜底。
        std::string resolvedPath = imgPath;
        if (!std::filesystem::exists(resolvedPath)) {
            auto alt = tmxDir / std::filesystem::path(imgPath);
            resolvedPath = alt.string();
        }

        TilesetTexture tt;
        if (!tt.texture.loadFromFile(resolvedPath)) {
            std::fprintf(stderr, "[WARN] TilemapRenderer: failed to load tileset texture: %s\n",
                         resolvedPath.c_str());
            return false;
        }
        tt.texture.setSmooth(false);
        tt.firstGid = ts.getFirstGID();
        tt.lastGid = ts.getLastGID();

        // 使用 tileset 自身的 tile 尺寸计算行列数
        auto tsTileSize = ts.getTileSize();
        tt.tileWidth = static_cast<int>(tsTileSize.x);
        tt.tileHeight = static_cast<int>(tsTileSize.y);
        tt.margin = static_cast<int>(ts.getMargin());
        tt.spacing = static_cast<int>(ts.getSpacing());

        // 先检查 tile 尺寸，避免后续除零
        if (tt.tileWidth <= 0 || tt.tileHeight <= 0) {
            std::fprintf(stderr,
                         "[WARN] TilemapRenderer: invalid tileset tile size (%dx%d), skipping\n",
                         tt.tileWidth, tt.tileHeight);
            continue;
        }

        // 行列数需扣除 margin 并考虑 spacing
        tt.columns = (static_cast<int>(tt.texture.getSize().x) - tt.margin + tt.spacing) /
                     (tt.tileWidth + tt.spacing);
        tt.rows = (static_cast<int>(tt.texture.getSize().y) - tt.margin + tt.spacing) /
                  (tt.tileHeight + tt.spacing);

        if (tt.columns <= 0 || tt.rows <= 0) {
            std::fprintf(stderr,
                         "[WARN] TilemapRenderer: tileset texture (%ux%u) too small for tile "
                         "(%dx%d)\n",
                         tt.texture.getSize().x, tt.texture.getSize().y, tsTileSize.x,
                         tsTileSize.y);
            continue;
        }

        m_tilesets.push_back(std::move(tt));
    }

    // 4. 递归展平 layer tree
    std::vector<FlatEntry> flatLayers;
    flattenLayers(map, flatLayers);

    // 5. 构建渲染批次 + 解析对象层
    m_batches.clear();
    m_objects.clear();

    for (const auto& entry : flatLayers) {
        if (!entry.visible)
            continue;

        switch (entry.layer->getType()) {
        case tmx::Layer::Type::Tile: {
            const auto& tileLayer = entry.layer->getLayerAs<tmx::TileLayer>();
            const auto& tiles = tileLayer.getTiles();
            if (tiles.empty())
                continue;

            const std::size_t tileCount = tiles.size();
            const std::size_t numTilesets = m_tilesets.size();

            // 第一遍：按 tileset 统计有效 tile 数
            std::vector<std::size_t> tileCounts(numTilesets, 0);
            for (std::size_t i = 0; i < tileCount; ++i) {
                std::uint32_t gid = tiles[i].ID;
                if (gid == 0)
                    continue;
                int tsIdx = findTilesetIndex(gid);
                if (tsIdx < 0)
                    continue;
                ++tileCounts[tsIdx];
            }

            // 创建 batch 并分配顶点
            std::vector<int> batchIndices(numTilesets, -1);
            std::uint8_t alpha;
            {
                float a = entry.opacity * 255.f;
                if (a < 0.f)
                    a = 0.f;
                if (a > 255.f)
                    a = 255.f;
                alpha = static_cast<std::uint8_t>(a);
            }
            for (std::size_t ti = 0; ti < numTilesets; ++ti) {
                if (tileCounts[ti] == 0)
                    continue;
                RenderBatch batch;
                batch.vertices.resize(tileCounts[ti] * 6);
                batch.texture = &m_tilesets[ti].texture;
                batchIndices[ti] = static_cast<int>(m_batches.size());
                m_batches.push_back(std::move(batch));
            }

            // 第二遍：填充顶点
            std::vector<std::size_t> vertexOffsets(numTilesets, 0);
            const auto color = sf::Color(255, 255, 255, alpha);

            for (std::size_t i = 0; i < tileCount; ++i) {
                std::uint32_t gid = tiles[i].ID;
                if (gid == 0)
                    continue;
                int tsIdx = findTilesetIndex(gid);
                if (tsIdx < 0)
                    continue;

                int batchIdx = batchIndices[tsIdx];
                if (batchIdx < 0)
                    continue;

                auto& batch = m_batches[static_cast<std::size_t>(batchIdx)];
                const auto& ts = m_tilesets[static_cast<std::size_t>(tsIdx)];
                std::size_t vi = vertexOffsets[static_cast<std::size_t>(tsIdx)];

                int y = static_cast<int>(i / static_cast<std::size_t>(mapW));
                int x = static_cast<int>(i % static_cast<std::size_t>(mapW));

                // GID → local tile index
                int localId = static_cast<int>(gid) - static_cast<int>(ts.firstGid);

                // tileset 纹理坐标（含 margin / spacing 修正）
                int col = localId % ts.columns;
                int row = localId / ts.columns;
                int tx = ts.margin + col * (ts.tileWidth + ts.spacing);
                int ty = ts.margin + row * (ts.tileHeight + ts.spacing);

                // 世界坐标：大图块底部对齐网格底部（Tiled 默认对齐规则）
                float px = static_cast<float>(x * tileW);
                float py = static_cast<float>((y + 1) * tileH) - static_cast<float>(ts.tileHeight);
                float tw = static_cast<float>(ts.tileWidth);
                float th = static_cast<float>(ts.tileHeight);

                float u0 = static_cast<float>(tx);
                float v0 = static_cast<float>(ty);
                float u1 = static_cast<float>(tx + ts.tileWidth);
                float v1 = static_cast<float>(ty + ts.tileHeight);

                // Triangles: 2 三角形 = 6 顶点（TL, TR, BL / TR, BR, BL）
                batch.vertices[vi + 0].position = {px, py};
                batch.vertices[vi + 0].texCoords = {u0, v0};
                batch.vertices[vi + 0].color = color;
                batch.vertices[vi + 1].position = {px + tw, py};
                batch.vertices[vi + 1].texCoords = {u1, v0};
                batch.vertices[vi + 1].color = color;
                batch.vertices[vi + 2].position = {px, py + th};
                batch.vertices[vi + 2].texCoords = {u0, v1};
                batch.vertices[vi + 2].color = color;
                batch.vertices[vi + 3].position = {px + tw, py};
                batch.vertices[vi + 3].texCoords = {u1, v0};
                batch.vertices[vi + 3].color = color;
                batch.vertices[vi + 4].position = {px + tw, py + th};
                batch.vertices[vi + 4].texCoords = {u1, v1};
                batch.vertices[vi + 4].color = color;
                batch.vertices[vi + 5].position = {px, py + th};
                batch.vertices[vi + 5].texCoords = {u0, v1};
                batch.vertices[vi + 5].color = color;

                vertexOffsets[static_cast<std::size_t>(tsIdx)] += 6;
            }
            break;
        }
        case tmx::Layer::Type::Object: {
            const auto& objGroup = entry.layer->getLayerAs<tmx::ObjectGroup>();
            for (const auto& obj : objGroup.getObjects()) {
                MapObject mo;
                mo.name = obj.getName();
                mo.type = obj.getClass();
                mo.x = obj.getPosition().x;
                mo.y = obj.getPosition().y;
                mo.width = obj.getAABB().width;
                mo.height = obj.getAABB().height;
                mo.rotation = obj.getRotation();
                mo.shape = static_cast<ObjectShape>(obj.getShape());
                mo.points.reserve(obj.getPoints().size());
                for (const auto& pt : obj.getPoints())
                    mo.points.emplace_back(pt.x, pt.y);
                m_objects.push_back(std::move(mo));
            }
            break;
        }
        case tmx::Layer::Type::Image:
            // Image layer 暂不渲染，仅记录
            std::fprintf(stderr, "[INFO] TilemapRenderer: skipping image layer '%s'\n",
                         entry.layer->getName().c_str());
            break;
        default:
            break;
        }
    }

    // 统计 tile 层数用于日志
    std::size_t tileLayerCount = 0;
    for (const auto& e : flatLayers)
        if (e.layer->getType() == tmx::Layer::Type::Tile)
            ++tileLayerCount;

    // 全部成功，提交地图尺寸到成员
    m_mapWidth = mapW;
    m_mapHeight = mapH;
    m_tileWidth = tileW;
    m_tileHeight = tileH;

    std::fprintf(stderr,
                 "[INFO] TilemapRenderer: loaded %s (%dx%d tiles, %zu tile layers, %zu batches, "
                 "%zu tilesets, %zu objects)\n",
                 tmxPath, m_mapWidth, m_mapHeight, tileLayerCount, m_batches.size(),
                 m_tilesets.size(), m_objects.size());
    return true;
}

void TilemapRenderer::draw(sf::RenderWindow& window) {
    for (const auto& batch : m_batches) {
        sf::RenderStates states;
        states.texture = batch.texture;
        window.draw(batch.vertices, states);
    }
}

// ---------------------------------------------------------------------------
// 内部辅助
// ---------------------------------------------------------------------------

int TilemapRenderer::findTilesetIndex(std::uint32_t gid) const {
    for (std::size_t i = 0; i < m_tilesets.size(); ++i) {
        if (gid >= m_tilesets[i].firstGid && gid <= m_tilesets[i].lastGid)
            return static_cast<int>(i);
    }
    // 仅对非空 tile 警告，减少噪音
    if (gid != 0)
        std::fprintf(stderr, "[WARN] TilemapRenderer: GID %u out of any tileset range\n", gid);
    return -1;
}

void TilemapRenderer::flattenLayers(const tmx::Map& map, std::vector<FlatEntry>& out) {
    for (const auto& layer : map.getLayers()) {
        if (layer->getType() == tmx::Layer::Type::Group) {
            flattenGroup(layer->getLayerAs<tmx::LayerGroup>(), out, 1.f, true);
        } else {
            out.push_back({layer.get(), layer->getOpacity(), layer->getVisible()});
        }
    }
}

void TilemapRenderer::flattenGroup(const tmx::LayerGroup& group, std::vector<FlatEntry>& out,
                                   float parentOpacity, bool parentVisible) {
    float accOpacity = parentOpacity * group.getOpacity();
    bool accVisible = parentVisible && group.getVisible();
    for (const auto& layer : group.getLayers()) {
        if (layer->getType() == tmx::Layer::Type::Group) {
            flattenGroup(layer->getLayerAs<tmx::LayerGroup>(), out, accOpacity, accVisible);
        } else {
            out.push_back(
                {layer.get(), accOpacity * layer->getOpacity(), accVisible && layer->getVisible()});
        }
    }
}
