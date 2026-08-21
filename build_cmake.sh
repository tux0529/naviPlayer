#!/bin/bash

# EPUB Builder 构建脚本（Qt5.15 / Qt6 双版本）
# 用法:
#   ./build_cmake.sh             # 默认 Qt5
#   ./build_cmake.sh 6           # Qt6
#   QT_ROOT=/path/to/qt ./build_cmake.sh
#
# 也可以传入 Qt 前缀路径作为第一个参数:
#   ./build_cmake.sh /Users/redtux/Qt/6.10.3/macos

set -e

QT_VERSION_ARG="${1:-}"

if [ -n "$QT_VERSION_ARG" ]; then
    case "$QT_VERSION_ARG" in
        5) QT_VERSION="5";;
        6) QT_VERSION="6";;
        *)
            # 视为完整路径
            QT_ROOT="$QT_VERSION_ARG"
            ;;
    esac
fi

if [ -z "$QT_VERSION" ]; then
    QT_VERSION="${QT_VERSION:-5}"
fi

if [ -z "$QT_ROOT" ]; then
    case "$QT_VERSION" in
        6) QT_ROOT="/Users/redtux/Qt/6.10.3/macos";;
        5) QT_ROOT="/Users/redtux/Qt/5.15.19/qt";;
    esac
fi

if [ ! -d "$QT_ROOT" ]; then
    echo "错误: Qt 路径不存在: $QT_ROOT" >&2
    exit 1
fi

echo "构建 Qt$QT_VERSION 版本，Qt 路径: $QT_ROOT"

# 设置环境变量
export PATH="$QT_ROOT/bin:$PATH"
export QTDIR="$QT_ROOT"

# 清理并创建构建目录
BUILD_DIR="build/qt$QT_VERSION"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

echo "运行 cmake..."
cmake ../.. -DCMAKE_PREFIX_PATH="$QT_ROOT" -DFFMPEGLIB_DIR=/Users/redtux/Documents/Nextcloud/Projects/Qt/naviPlayer/libs/ffmpeg61_arm64

# 编译
echo "编译项目..."
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
make -j"$JOBS"

echo "构建完成！"
