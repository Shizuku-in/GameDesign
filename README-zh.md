# Survivor-like

一款类《吸血鬼幸存者》的 2D 弹幕天堂游戏，使用 SFML 3.1 和现代 CMake 构建。

WASD 移动，武器自动索敌攻击，击杀敌人掉落经验宝石，升级后从随机选项中挑选强化——目标只有一个：活得更久。

## 环境要求

- **CMake** >= 3.16
- **SFML** >= 3.1（头文件 + 静态库）
- 支持 C++17 的编译器（Clang / GCC / MSVC）

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
从 [sfml-dev.org](https://www.sfml-dev.org/download.php) 下载，并把 `CMAKE_PREFIX_PATH` 指向安装目录。

## 编译 & 运行

```bash
# 配置项目
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build

# 运行
./build/game
```

Windows:
```bash
cmake -B build
cmake --build build --config Release
.\build\Release\game.exe
```

字体等资源文件会通过 CMake 的 `POST_BUILD` 自动复制到构建目录，不用手动处理。

## 操作方式

| 按键 | 功能 |
|-----|--------|
| WASD / 方向键 | 移动 |
| 方向键 / 1–3 | 升级界面选择强化 |
| 回车 / 空格 | 确认升级 / 开始游戏 |
| Esc | 退出 |

## 项目结构

```
├── CMakeLists.txt
├── README.md
├── CLAUDE.md                # 架构说明与编码规范
├── .clang-format            # 代码风格配置（基于 WebKit）
├── assets/
│   └── fonts/
│       └── DejaVuSans.ttf   # UI 字体
├── docs/
│   ├── design-doc.md        # 完整设计文档
│   └── wsl-setup-guide.md   # WSL 环境配置指南
├── scripts/
│   └── pre-commit           # clang-format 检查的 git hook
└── src/
    ├── main.cpp
    ├── core/                # 引擎层
    │   ├── Game.hpp/.cpp        # 游戏主循环、场景管理、字体加载
    │   ├── Scene.hpp/.cpp       # 场景抽象基类
    │   ├── Pool.hpp             # 通用对象池（空闲链表 + 世代编号）
    │   └── ResourceManager.hpp  # 资源缓存模板
    ├── data/                # 纯数据定义（全部 header-only）
    │   ├── Constants.hpp        # 所有可调参数
    │   ├── EntityTypes.hpp      # 敌人、弹幕、经验宝石结构体
    │   ├── PlayerState.hpp      # 玩家状态
    │   └── Collision.hpp        # 圆-圆碰撞检测
    ├── systems/             # 玩法系统
    │   ├── WeaponDefs.hpp/.cpp  # 武器数值表 + 升级公式
    │   ├── WeaponSystem.hpp/.cpp# 自动索敌 + 射击逻辑
    │   ├── UpgradeDefs.hpp/.cpp # 随机升级生成与应用
    │   └── HUD.hpp/.cpp         # 血条、经验条、等级、计时、武器列表
    └── scenes/              # 游戏场景
        ├── PlayScene.hpp/.cpp   # 核心对局场景
        ├── TitleScene.hpp/.cpp  # 标题界面
        └── GameOverScene.hpp/.cpp # 结算界面
```

## 代码格式化

项目用 **clang-format** 统一代码风格，配置文件在仓库根目录的 `.clang-format`。

### 安装 clang-format

| 平台 | 命令 |
|---|---|
| macOS | `brew install clang-format` |
| Linux | `sudo apt install clang-format` |
| Windows | 从 [LLVM 发布页](https://github.com/llvm/llvm-project/releases) 下载 |

### 提交前格式化

```bash
cmake --build build --target format        # 原地格式化所有源文件
cmake --build build --target format-check  # 仅检查（CI 用）
```

### 推荐配置 Git Hook

```bash
git config core.hooksPath scripts
```

这样每次 `git commit` 前会自动检查格式，不通过会阻止提交。

### 编辑器集成

大多数编辑器能自动识别项目中的 `.clang-format` 文件：

- **VS Code**：装 "C/C++" 插件，开启保存时格式化（`"editor.formatOnSave": true`）
- **CLion / IntelliJ**：默认支持 C++ 文件的 clang-format
- **Vim / Neovim**：`:!clang-format -i %`
- **Emacs**：`clang-format-buffer`（需要 `clang-format.el`）

## 许可证

[MIT](LICENSE)
