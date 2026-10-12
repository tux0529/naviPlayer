#!/bin/bash

# naviPlayer 构建脚本（Qt5.15 / Qt6 双版本，ffmpeg 4.4/5.1/6.1/7.1 可选）
#
# 用法:
#   ./build_cmake.sh                 # 默认 Qt5 + ffmpeg6.1
#   ./build_cmake.sh 5               # Qt5.15
#   ./build_cmake.sh 6               # Qt6
#   ./build_cmake.sh 5 7             # Qt5 + ffmpeg7.1
#   ./build_cmake.sh /path/to/qt 6   # 指定 Qt 前缀 + ffmpeg6.1
#   FFMPEG_VER=4 ./build_cmake.sh    # 环境变量指定 ffmpeg 版本
#
# 可选环境变量:
#   QT_ROOT            Qt 安装前缀
#   FFMPEG_VER         4|5|6|7（默认 6）
#   FFMPEGLIB_DIR      自定义 ffmpeg 前缀（优先级最高）
#   QWINDOWKIT_DIR     自定义 QWindowKit cmake 配置目录
#   BUILD_TYPE         Debug|Release（默认 Release）
#   NO_CLEAN=1         不清理构建目录（增量编译）

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

QT_VERSION_ARG="${1:-}"
FFMPEG_ARG="${2:-}"

case "$QT_VERSION_ARG" in
    5|6) QT_VERSION="$QT_VERSION_ARG";;
    "")  QT_VERSION="${QT_VERSION:-5}";;
    *)   QT_ROOT="$QT_VERSION_ARG"; QT_VERSION="";;
esac

case "$FFMPEG_ARG" in
    4|5|6|7) FFMPEG_VER="$FFMPEG_ARG";;
    "")      :;;
    *)       echo "错误: 未知的 ffmpeg 版本: $FFMPEG_ARG（可选 4/5/6/7）" >&2; exit 1;;
esac

FFMPEG_VER="${FFMPEG_VER:-6}"

if [ -z "$QT_VERSION" ]; then
    if [[ "$QT_ROOT" == *"/6."* || "$QT_ROOT" == *"Qt6"* ]]; then
        QT_VERSION="6"
    else
        QT_VERSION="5"
    fi
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

# ffmpeg 前缀（可被 FFMPEGLIB_DIR 覆盖）
FFMPEG_DEFAULT_ROOT="/Users/redtux/.local/libs/ffmpeg${FFMPEG_VER}_audio"
FFMPEGLIB_DIR="${FFMPEGLIB_DIR:-$FFMPEG_DEFAULT_ROOT}"

if [ ! -d "$FFMPEGLIB_DIR" ]; then
    echo "错误: ffmpeg 路径不存在: $FFMPEGLIB_DIR" >&2
    exit 1
fi

# QWindowKit（Qt5 与 Qt6 安装位置不同）
if [ -z "$QWINDOWKIT_DIR" ]; then
    case "$QT_VERSION" in
        6) QWINDOWKIT_DIR="/Users/redtux/.local/libs/qwindowkit_static/lib/cmake/QWindowKit";;
        5) QWINDOWKIT_DIR="/Users/redtux/Qt/5.15.19/qwindowkits/lib/cmake/QWindowKit";;
    esac
fi

if [ ! -d "$QWINDOWKIT_DIR" ]; then
    echo "错误: QWindowKit 路径不存在: $QWINDOWKIT_DIR" >&2
    exit 1
fi

BUILD_TYPE="${BUILD_TYPE:-Release}"

echo "构建 Qt$QT_VERSION 版本"
echo "  Qt          : $QT_ROOT"
echo "  ffmpeg      : $FFMPEGLIB_DIR"
echo "  QWindowKit  : $QWINDOWKIT_DIR"
echo "  Build type  : $BUILD_TYPE"

export PATH="$QT_ROOT/bin:$PATH"
export QTDIR="$QT_ROOT"

BUILD_DIR="build/qt$QT_VERSION"
if [ -z "$NO_CLEAN" ]; then
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "运行 cmake..."
cmake ../.. \
    -DCMAKE_PREFIX_PATH="$QT_ROOT" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DFFMPEGLIB_DIR="$FFMPEGLIB_DIR" \
    -DQWindowKit_DIR="$QWINDOWKIT_DIR" \
    -DQWINDOWKIT_DIR="$QWINDOWKIT_DIR"

JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo "编译项目（-j$JOBS）..."
cmake --build . --config "$BUILD_TYPE" -j"$JOBS"

echo "构建完成！产物: $BUILD_DIR"
