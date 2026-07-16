#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace tmx {
class Layer;
class LayerGroup;
class Map;
} // namespace tmx

/// 对象形状类型，与 tmxlite::Object::Shape 一一对应。
enum class ObjectShape : std::uint8_t { Rectangle = 0, Ellipse, Point, Polygon, Polyline, Text };

/// 对象层解析结果，供碰撞 / 生成等系统读取。
struct MapObject {
    /// Tiled 对象的名称。
    std::string name;
    /// Tiled 对象的类型标签。
    std::string type;
    /// 对象左上角的世界坐标 X。
    float x = 0.f;
    /// 对象左上角的世界坐标 Y。
    float y = 0.f;
    /// 矩形对象的宽度。
    float width = 0.f;
    /// 矩形对象的高度。
    float height = 0.f;
    /// 对象绕原点的旋转角度。
    float rotation = 0.f;
    /// 对象的几何形状类型。
    ObjectShape shape = ObjectShape::Rectangle;
    /// 多边形或折线对象的局部顶点列表。
    std::vector<sf::Vector2f> points;
};

/// Tiled TMX 地图渲染器。加载一次，每帧绘制。
class TilemapRenderer {
public:
    /// 从 TMX 文件加载地图并构建顶点。tileset 纹理路径由 TMX 内部解析。
    bool loadFromFile(const char* tmxPath);

    /// 绘制所有可见 tile 层。
    void draw(sf::RenderWindow& window);

    /// 地图尺寸（像素）。
    [[nodiscard]] float getWidth() const { return static_cast<float>(m_mapWidth * m_tileWidth); }
    [[nodiscard]] float getHeight() const { return static_cast<float>(m_mapHeight * m_tileHeight); }

    /// 对象层数据（只读）。
    [[nodiscard]] const std::vector<MapObject>& getObjects() const { return m_objects; }

private:
    /// 单个 tileset 纹理及其元数据。
    struct TilesetTexture {
        /// 已加载的 tileset 纹理。
        sf::Texture texture;
        /// 此 tileset 在地图全局 tile ID 中的首个 ID。
        std::uint32_t firstGid = 0;
        /// 此 tileset 在地图全局 tile ID 中的末个 ID。
        std::uint32_t lastGid = 0;
        /// tileset 每行 tile 数。
        int columns = 0;
        /// tileset 每列 tile 数。
        int rows = 0;
        /// 单个 tile 的像素宽度。
        int tileWidth = 0;
        /// 单个 tile 的像素高度。
        int tileHeight = 0;
        /// tileset 外边距。
        int margin = 0;
        /// 相邻 tile 之间的像素间距。
        int spacing = 0;
    };

    /// 一次绘制单元：一个 VertexArray 绑一个纹理，不透明度已烘焙到顶点色。
    struct RenderBatch {
        /// 已合并为三角形的 tile 顶点。
        sf::VertexArray vertices{sf::PrimitiveType::Triangles};
        /// 绘制该批次时绑定的 tileset 纹理。
        const sf::Texture* texture = nullptr;
    };

    /// 展平后的层条目（加载阶段临时使用）。
    struct FlatEntry {
        /// 原始 TMX 层。
        const tmx::Layer* layer = nullptr;
        /// 父组累计后的层不透明度。
        float opacity = 1.f;
        /// 父组累计后的层可见性。
        bool visible = true;
    };

    /// 地图使用的 tileset 纹理与元数据。
    std::vector<TilesetTexture> m_tilesets;
    /// 按纹理拆分的预构建 tile 绘制批次。
    std::vector<RenderBatch> m_batches;
    /// 从 TMX 对象层解析出的游戏对象。
    std::vector<MapObject> m_objects;

    /// 单个 tile 的像素宽度。
    int m_tileWidth = 0;
    /// 单个 tile 的像素高度。
    int m_tileHeight = 0;
    /// 地图横向 tile 数。
    int m_mapWidth = 0;
    /// 地图纵向 tile 数。
    int m_mapHeight = 0;

    /// 返回 GID 所属 tileset 的索引，-1 表示越界或空 tile。
    [[nodiscard]] int findTilesetIndex(std::uint32_t gid) const;

    /// 递归展平 layer tree，累积不透明度和可见性。
    void flattenLayers(const tmx::Map& map, std::vector<FlatEntry>& out);
    void flattenGroup(const tmx::LayerGroup& group, std::vector<FlatEntry>& out,
                      float parentOpacity, bool parentVisible);
};
