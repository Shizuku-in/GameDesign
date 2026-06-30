# Survivor-like

一款类《吸血鬼幸存者》的 2D 弹幕天堂游戏，使用 SFML 3.1 和现代 CMake（C++20）构建。

WASD 移动，武器自动索敌攻击，击杀敌人掉落经验宝石，升级后从随机选项中挑选强化——目标只有一个：活得更久。

## 环境要求

- **CMake** >= 3.16
- **SFML** >= 3.1（头文件 + 静态库）
- 支持 C++20 的编译器（GCC 13+ / Clang 17+ / MSVC 2022+）
- **tmxlite** — 已内置在 `third_party/`，需手动克隆：
  ```bash
  mkdir -p third_party && cd third_party
  git clone https://github.com/fallahn/tmxlite.git
  ```

### 安装 SFML 3.1

**macOS：**
```bash
brew install sfml
```

**Linux（apt）：**
```bash
sudo apt install libsfml-dev
```

**Windows：**
从 [sfml-dev.org](https://www.sfml-dev.org/download.php) 下载，把 `CMAKE_PREFIX_PATH` 指向安装目录。

### 地图编辑器（可选）

地图使用 [Tiled](https://www.mapeditor.org/) 编辑：
```bash
sudo apt install tiled   # Linux
brew install tiled       # macOS
```

## 编译 & 运行

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/game
```

Windows：
```bash
cmake -B build
cmake --build build --config Release
.\build\Release\game.exe
```

资源文件通过 CMake `POST_BUILD` 自动复制到构建目录。

## 操作方式

| 按键 | 功能 |
|-----|--------|
| WASD | 移动 |
| 方向键 / 1–3 | 升级界面选择强化 |
| 回车 / 空格 | 确认升级 / 开始游戏 |
| Esc | 暂停 / 返回标题 |

## 项目结构

```
├── CMakeLists.txt
├── README.md
├── README-zh.md
├── CLAUDE.md                 # 架构说明与编码规范
├── .clang-format             # 代码风格配置（基于 WebKit）
├── assets/
│   ├── fonts/                # UI 字体
│   ├── sounds/
│   │   ├── bgm/              # 背景音乐 (.ogg)
│   │   └── sfx/              # 音效 (.wav)
│   ├── sprites/
│   │   ├── character/elf/    # 角色精灵（待机/移动/立绘/技能）
│   │   └── enemies/          # 敌人精灵表
│   ├── tilesets/             # 地图瓦片素材
│   └── maps/                 # Tiled .tmx 地图文件
├── third_party/
│   └── tmxlite/              # Tiled 地图解析库（内置）
└── src/
    ├── main.cpp
    ├── core/                 # 引擎层
    │   ├── Game.hpp/.cpp         # 游戏主循环、场景管理、资源加载
    │   ├── Scene.hpp/.cpp        # 场景抽象基类
    │   ├── Pool.hpp              # 通用对象池（空闲链表 + 世代编号）
    │   ├── ResourceManager.hpp   # 资源缓存模板
    │   └── Random.hpp/.cpp       # Mersenne Twister 随机数
    ├── data/                 # 纯数据定义
    │   ├── Constants.hpp         # 通用可调参数 + 颜色
    │   ├── EntityTypes.hpp       # 敌人、弹幕、经验宝石、伤害飘字
    │   └── PlayerState.hpp       # 玩家状态结构体
    ├── math/                 # 数学工具
    │   └── Collision.hpp         # constexpr 圆-圆碰撞检测
    ├── audio/                # 音频
    │   └── SoundPlayer.hpp/.cpp  # sf::Sound 对象池，间隔保护防叠加
    ├── graphics/             # 渲染器
    │   ├── SpriteSheet.hpp       # 精灵表加载器
    │   ├── TilemapRenderer.hpp/.cpp  # Tiled TMX → VertexArray 瓦片渲染
    │   └── WorldRenderer.hpp/.cpp  # 实体 + 伤害飘字渲染
    ├── gameplay/             # 数据驱动的定义表
    │   ├── WeaponDefs.hpp/.cpp   # 武器数值表 + 升级公式 + 工厂
    │   ├── EnemyDefs.hpp/.cpp    # 敌人定义表
    │   ├── CharacterDefs.hpp/.cpp   # 角色定义表（属性 + 精灵）
    │   ├── MapDefs.hpp/.cpp      # 地图定义表（生成参数 + 资源路径）
    │   └── UpgradeDefs.hpp/.cpp  # 升级定义表 + 随机生成
    ├── systems/              # 运行时玩法系统
    │   ├── IWeaponBehavior.hpp   # 武器策略接口（fire / tickAoE）
    │   ├── WeaponBehaviors.hpp/.cpp  # 5 种武器行为
    │   ├── WeaponSystem.hpp/.cpp # 6 槽位武器管理器
    │   ├── CollisionSystem.hpp/.cpp  # 空间哈希碰撞检测 + 清理
    │   └── SpawningSystem.hpp/.cpp   # 波次生成 + 难度递增
    ├── ui/                   # 屏幕空间 UI
    │   ├── HUD.hpp/.cpp          # 血条、经验条、等级、计时、武器列表
    │   ├── PauseMenu.hpp/.cpp    # 暂停菜单
    │   └── UpgradeUI.hpp/.cpp    # 升级选择界面
    └── scenes/               # 游戏场景
        ├── PlayScene.hpp/.cpp      # 核心对局场景
        ├── TitleScene.hpp/.cpp     # 标题界面
        └── GameOverScene.hpp/.cpp  # 结算界面
```

## 添加内容（数据驱动）

所有游戏内容遵循同一种表驱动模式，添加新内容只需 2–3 步：

| 内容 | 步骤 |
|------|------|
| 新武器 | 枚举 → WeaponDef 条目 → 行为类 |
| 新敌人 | 枚举 → EnemyDef 条目 |
| 新角色 | 枚举 → CharacterDef 条目（属性 + 精灵） |
| 新地图 | 枚举 → MapDef 条目 → Tiled 创建 .tmx |
| 新属性提升 | UpgradeDef 表加一行 |

## 代码格式化

```bash
cmake --build build --target format        # 格式化所有源文件
cmake --build build --target format-check  # 仅检查（CI 用）
```

风格：`.clang-format`（WebKit，4 空格，100 列）。

## 许可证

[MIT](LICENSE)
