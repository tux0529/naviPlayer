# Qt 5.15 / Qt 6 跨版本兼容方案

## 1. 目标

本项目已完成对 `Qt 5.15` 与 `Qt 6` 的双版本适配，支持在编译期根据 Qt 主版本自动选择对应 API 与构建配置。

当前已验证的开发环境：

- Qt 5.15.19: `/Users/redtux/Qt/5.15.19/qt`
- Qt 6.10.3: `/Users/redtux/Qt/6.10.3/macos`

## 2. 核心不兼容点与处理方式

### 2.1 音频输出类差异

- Qt 6 使用：`QAudioSink`
- Qt 5 使用：`QAudioOutput`

处理方式：

- 新增统一兼容头：[src/qt_compat.h](file:///Users/redtux/build/naviPlayer/src/qt_compat.h)
- 通过类型别名统一成：
  - `NaviAudioOutput`
- 在 `FFmpegAudioDecoder` 中统一调用，避免业务代码散落版本判断

### 2.2 PCM 音频格式设置差异

- Qt 6 使用：`QAudioFormat::setSampleFormat(QAudioFormat::Int16)`
- Qt 5 使用：
  - `setCodec("audio/pcm")`
  - `setSampleSize(16)`
  - `setByteOrder(QAudioFormat::LittleEndian)`
  - `setSampleType(QAudioFormat::SignedInt)`

处理方式：

- 在 `qt_compat.h` 中提供 `configurePcm16StereoAudioFormat(...)`
- 音频播放模块统一调用该函数

### 2.3 QWidget::enterEvent 签名差异

- Qt 6：`enterEvent(QEnterEvent *)`
- Qt 5：`enterEvent(QEvent *)`

处理方式：

- 在 `qt_compat.h` 中提供统一别名：
  - `NaviEnterEvent`
- 在以下文件中统一使用：
  - `src/widgets/albumwidget.cpp`
  - `src/widgets/homewidget.cpp`

### 2.4 头文件显式依赖差异

Qt 5 下部分类型不会像 Qt 6 那样通过间接包含自动补齐，因此需要显式包含头文件。

本次补充：

- `homewidget.cpp` 显式增加 `#include <QMouseEvent>`

## 3. 构建系统适配

### 3.1 qmake

已更新：

- [naviPlayer.pro](file:///Users/redtux/build/naviPlayer/naviPlayer.pro)

处理内容：

- 根据 `QT_MAJOR_VERSION` 自动注入版本宏：
  - Qt 6: `NAVIPLAYER_QT6`
  - Qt 5: `NAVIPLAYER_QT5`
- 将 `src/qt_compat.h` 纳入工程头文件列表

### 3.2 CMake

已重写：

- [CMakeLists.txt](file:///Users/redtux/build/naviPlayer/CMakeLists.txt)

处理内容：

- 使用：
  - `find_package(QT NAMES Qt6 Qt5 ...)`
  - `find_package(Qt${QT_VERSION_MAJOR} ...)`
- 链接目标统一为：
  - `Qt${QT_VERSION_MAJOR}::Core`
  - `Qt${QT_VERSION_MAJOR}::Gui`
  - `Qt${QT_VERSION_MAJOR}::Widgets`
  - `Qt${QT_VERSION_MAJOR}::Network`
  - `Qt${QT_VERSION_MAJOR}::Sql`
  - `Qt${QT_VERSION_MAJOR}::Multimedia`
- 同样按版本注入：
  - `NAVIPLAYER_QT6 / NAVIPLAYER_QT5`

## 4. 已完成的构建验证

### 4.1 qmake 构建

- Qt 6:
  - 命令：
    - `"/Users/redtux/Qt/6.10.3/macos/bin/qmake" /Users/redtux/build/naviPlayer/naviPlayer.pro`
    - `make -j4`
  - 结果：通过

- Qt 5.15:
  - 命令：
    - `"/Users/redtux/Qt/5.15.19/qt/bin/qmake" /Users/redtux/build/naviPlayer/naviPlayer.pro`
    - `make -j4`
  - 结果：通过

### 4.2 CMake 配置验证

- Qt 6:
  - `cmake -S . -B build_cmake_qt6 -DCMAKE_PREFIX_PATH=/Users/redtux/Qt/6.10.3/macos`
  - 结果：通过

- Qt 5.15:
  - `cmake -S . -B build_cmake_qt5 -DCMAKE_PREFIX_PATH=/Users/redtux/Qt/5.15.19/qt`
  - 结果：通过

## 5. 注意事项

1. 当前环境已完成双版本编译验证，但**未进行完整 GUI 交互式运行回归**
2. Qt 5 构建时会提示当前 macOS SDK 版本较新，这是 Qt 5.15 的已知提示，不影响本次编译通过
3. 后续新增代码若涉及以下模块，建议优先检查 Qt 5/6 差异：
   - Multimedia
   - 输入事件
   - 绘图与高 DPI
   - 网络请求属性

## 6. 后续新增功能的接入规范

后续新增代码若需要同时兼容 Qt 5 / Qt 6，建议遵循：

1. 优先复用 `src/qt_compat.h`，避免在业务代码里散落大量 `#if/#else`
2. 若必须写条件编译，统一使用 Qt 官方宏：
   - `QT_VERSION`
   - `QT_VERSION_CHECK(...)`
3. 构建脚本中的 Qt 目标名称统一使用：
   - `Qt${QT_VERSION_MAJOR}::...`
4. 提交前至少执行：
   - 一次 Qt 5.15 编译
   - 一次 Qt 6 编译

