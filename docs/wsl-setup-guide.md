# WSL Ubuntu 开发环境配置教程

本教程从零开始，在 Windows 上通过 WSL2 + Ubuntu 搭建 SFML 3.1 游戏开发环境。

## 1. 安装 WSL2 和 Ubuntu

### 1.1 启用 WSL

以管理员身份打开 PowerShell，执行：

```powershell
wsl --install
```

该命令会自动安装 WSL2 内核和 Ubuntu 发行版。完成后重启电脑。

如果已安装过 WSL 但未升级到 WSL2：

```powershell
wsl --set-default-version 2
```

### 1.2 验证安装

重启后在开始菜单中找到 "Ubuntu" 并打开，首次运行会要求创建 Linux 用户名和密码。

在 Ubuntu 终端中验证版本：

```bash
lsb_release -a
# 应输出 Ubuntu 24.04 LTS 或更高版本
```

## 2. 安装构建工具链

### 2.1 更新软件源

```bash
sudo apt update && sudo apt upgrade -y
```

### 2.2 安装编译器和 CMake

```bash
sudo apt install -y build-essential cmake g++ git
```

验证安装：

```bash
g++ --version     # 应 >= 11.x
cmake --version   # 应 >= 3.16
git --version
```

### 2.3 安装 SFML 依赖

SFML 依赖以下系统库，需要先安装：

```bash
sudo apt install -y \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libgl1-mesa-dev \
    libfreetype6-dev \
    libopenal-dev \
    libvorbis-dev \
    libflac-dev \
    libharfbuzz-dev
```

## 3. 编译安装 SFML 3.1

Ubuntu 24.04 官方源中的 SFML 是 2.6 版本，比项目要求的 3.1 更旧。需要从源码编译安装。

### 3.1 下载 SFML 3.1 源码

```bash
cd ~
wget https://www.sfml-dev.org/files/SFML-3.1.0-sources.zip
unzip SFML-3.1.0-sources.zip
cd SFML-3.1.0
```

### 3.2 编译并安装

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build build -j$(nproc)
sudo cmake --install build
```

参数说明：
- `-DBUILD_SHARED_LIBS=OFF` — 编译静态库，与项目 CMake 配置的 `SFML_STATIC_LIBRARIES ON` 一致
- `-j$(nproc)` — 使用全部 CPU 核心并行编译

默认安装到 `/usr/local/`，与项目 CMakeLists.txt 的查找路径一致。

### 3.3 验证安装

```bash
ls /usr/local/lib/libsfml-*-s.a
# 应看到五个文件：
# libsfml-system-s.a  libsfml-window-s.a  libsfml-graphics-s.a
# libsfml-audio-s.a   libsfml-network-s.a

ls /usr/local/include/SFML/
# 应看到 Audio.hpp Config.hpp Graphics.hpp Network.hpp System.hpp Window.hpp
```

## 4. 获取并构建项目

### 4.1 克隆代码

```bash
cd ~
git clone <your-repo-url> game
cd game
```

### 4.2 构建

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## 5. 运行游戏

WSL2 自带图形界面支持（WSLg），无需额外配置。直接在 WSL 终端运行：

```bash
./build/game
```

如果窗口没有出现，检查 WSLg 是否正常：

```bash
echo $DISPLAY
# 应输出类似 :0 的值
```

## 常见问题

### Q: cmake 找不到 SFML

确保 SFML 安装在 `/usr/local/` 下，或者手动指定查找路径：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local
```

### Q: 编译 SFML 时报缺少依赖

根据报错信息安装缺失的 `-dev` 包，例如：

```bash
sudo apt install libxxx-dev
```

然后重新从 3.2 步开始。

### Q: 窗口弹出但显示黑色/透明

这是 WSLg 的渲染兼容性问题。尝试：

1. 更新显卡驱动到最新版本
2. 确保 WSL 使用最新内核：`wsl --update`（在 PowerShell 中执行）

### Q: 不想从源码编译 SFML

可以尝试从 Ubuntu 24.10+ 的源直接安装（如果未来默认版本升到 3.1）：

```bash
sudo apt install libsfml-dev
```

但目前（2026 年中）大多数 Ubuntu 版本的官方源中 SFML 仍为 2.6，建议从源码编译 3.1。

### Q: 性能问题

WSL2 下的图形性能不如原生 Linux。如果调试游戏逻辑，可以在 Windows 原生编译器上构建（MSVC + vcpkg 安装 SFML），WSL 则仅用于 Linux 兼容性测试。
