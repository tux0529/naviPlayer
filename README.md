# naviPlayer

naviPlayer 是一款基于 Qt 框架开发的 Subsonic 协议音乐播放客户端。

## 功能特性

### 核心功能
- ✅ Subsonic 协议完整对接
  - 用户身份认证
  - 服务器地址配置
  - API 请求封装（支持获取艺术家、专辑、歌曲、搜索、封面图等）
- ✅ 音频播放核心模块
  - 使用 Qt Multimedia 框架实现音频播放
  - 支持播放、暂停、停止、进度跳转、音量调节
- ✅ 歌曲缓冲管理
- ✅ 基础播放列表系统

### 界面与交互
- ✅ 基于 Qt Widgets 的响应式界面
  - 左侧导航栏（我的收藏、全部歌曲、专辑、艺术家、搜索）
  - 主内容区
  - 底部固定播放控制栏
- ✅ 适配不同屏幕尺寸
- ✅ 播放状态可视化

### 数据与缓存管理
- ✅ SQLite 本地存储
  - 用户配置
  - 播放历史
  - 收藏列表
- ✅ 本地缓存机制
  - 已加载的封面图缓存
  - 减少重复网络请求
  - 页面数据缓存 TTL（专辑页 / 艺术家页 / 主页）

## 系统要求

- Qt 5.15 或 Qt 6（本项目已完成 Qt 5.15.19 与 Qt 6.10.3 的构建兼容）
- C++17 兼容的编译器
  - Windows: MSVC 2019 或更高
  - macOS: Clang 12 或更高
  - Linux: GCC 9 或更高

## 构建说明

### 1. 安装依赖

#### macOS
1. 下载并安装 Qt 在线安装器: https://www.qt.io/download/
2. 安装 Qt 5.15 或 Qt 6（推荐 Qt 5.15.19 / Qt 6.10.3）

或者使用 Homebrew：
```bash
brew install qt
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install build-essential qtbase5-dev qtmultimedia5-dev
# 或安装 Qt 6 对应开发包
```

#### Windows
1. 下载并安装 Qt 在线安装器: https://www.qt.io/download/
2. 安装 Qt 5.15 或 Qt 6

### 2. 克隆项目

```bash
git clone <repository-url>
cd naviPlayer
```

### 3. 构建项目

#### 使用 qmake (推荐)

```bash
# macOS 和 Linux（使用环境变量中的 qmake）
mkdir build && cd build
qmake ..
make
```

或使用具体路径的 qmake：
```bash
# Qt 6 示例
/Users/yourname/Qt/6.10.3/macos/bin/qmake ..
make

# Qt 5.15 示例
/Users/yourname/Qt/5.15.19/qt/bin/qmake ..
make
```

#### 使用 CMake

```bash
# Qt 6
cmake -S . -B build_cmake_qt6 -DCMAKE_PREFIX_PATH=/Users/yourname/Qt/6.10.3/macos
cmake --build build_cmake_qt6 -j4

# Qt 5.15
cmake -S . -B build_cmake_qt5 -DCMAKE_PREFIX_PATH=/Users/yourname/Qt/5.15.19/qt
cmake --build build_cmake_qt5 -j4
```

#### Windows
```cmd
mkdir build
cd build
qmake ..
nmake
```

### 4. 运行程序

构建完成后，可执行文件位于：
- Windows: `Release/naviPlayer.exe`
- macOS: `build/naviPlayer.app`
- Linux: `build/naviPlayer`

在 macOS 上，你可以直接：
```bash
open naviPlayer.app
```

## 使用说明

### 首次运行

1. 启动 naviPlayer
2. 在弹出的设置对话框中，配置 Subsonic 服务器信息：
   - 服务器地址（例如：http://localhost:4533）
   - 用户名
   - 密码
3. 点击"保存"按钮

### 缓存TTL 配置

- 设置页提供统一的 `缓存TTL` 配置项，单位为“分钟”，默认值为 `10 分钟`
- `缓存TTL` 用于控制页面数据缓存的有效期：
  - 专辑页：优先显示已缓存页数据，再按 TTL 在后台静默刷新
  - 艺术家页：优先显示已缓存列表，再按 TTL 在后台静默刷新
  - 主页：四个模块沿用相同 TTL 策略，避免频繁切页时重复 loading
- 详细实现说明与后续页面接入规范见 [CACHE_TTL.md](CACHE_TTL.md)

### 专辑列表性能设置

- 设置页新增 `每页展示专辑数量`
- 支持选项：`10 / 20 / 24 / 30 / 50`
- 默认值：`24`
- 保存后无需重启应用，会立即作用到专辑列表分页请求与渲染逻辑
- 页面命中缓存时会优先显示已缓存内容，同时后台静默刷新，并预加载相邻页

### Qt 5 / Qt 6 兼容说明

- 项目已增加统一兼容层，处理 Qt 5 / Qt 6 在以下方面的差异：
  - 音频输出类：`QAudioOutput / QAudioSink`
  - PCM 音频格式设置
  - `enterEvent` 事件签名
  - 构建系统中的 Qt 目标与版本宏
- 详细适配记录见 [QT5_QT6_COMPATIBILITY.md](QT5_QT6_COMPATIBILITY.md)

### 基本操作

1. **导航菜单**
   - 我的收藏：查看收藏的歌曲
   - 专辑：浏览音乐专辑
   - 艺术家：浏览艺术家
   - 搜索：搜索音乐

2. **播放控制**
   - 双击歌曲列表中的歌曲开始播放
   - 使用底部播放控制栏进行播放、暂停、调整音量、跳转进度

3. **添加收藏**
   - （待实现）

## 项目结构

```
naviPlayer/
├── CMakeLists.txt              # CMake 构建配置
├── README.md                   # 项目文档
├── resources.qrc               # Qt 资源文件
├── src/
│   ├── main.cpp                # 程序入口
│   ├── mainwindow.h/cpp        # 主窗口
│   ├── mainwindow.ui           # 主窗口 UI 文件
│   ├── subsonicapi.h/cpp       # Subsonic API 封装
│   ├── databasemanager.h/cpp   # SQLite 数据库管理
│   ├── audioplayer.h/cpp       # 音频播放器
│   ├── playlistmanager.h/cpp   # 播放列表管理
│   ├── cachemanager.h/cpp      # 缓存管理
│   ├── settingsdialog.h/cpp    # 设置对话框
│   ├── settingsdialog.ui       # 设置对话框 UI
│   ├── models/                 # 数据模型
│   │   ├── song.h/cpp
│   │   ├── album.h/cpp
│   │   ├── artist.h/cpp
│   │   └── playlist.h/cpp
│   └── widgets/                # 自定义 UI 组件
│       ├── navigationwidget.h/cpp
│       ├── playercontrolbar.h/cpp
│       ├── songlistwidget.h/cpp
│       ├── albumwidget.h/cpp
│       ├── artistwidget.h/cpp
│       └── searchwidget.h/cpp
```

## 支持的服务器

naviPlayer 支持任何兼容 Subsonic API 的服务器，包括：
- Navidrome
- Airsonic
- Subsonic 官方服务器

## 开发状态

### 已完成
- ✅ 基础项目结构
- ✅ Subsonic API 封装
- ✅ 数据库管理模块
- ✅ 音频播放模块
- ✅ 缓存管理模块
- ✅ 主界面框架
- ✅ 导航栏
- ✅ 播放控制栏
- ✅ 歌曲列表

### 待完善
- ⏳ 专辑展示界面
- ⏳ 艺术家展示界面
- ⏳ 搜索功能
- ⏳ 播放列表功能
- ⏳ 收藏功能
- ⏳ 网络音频缓冲优化

## 许可证

（待添加）

## 贡献

欢迎提交 Issue 和 Pull Request！
