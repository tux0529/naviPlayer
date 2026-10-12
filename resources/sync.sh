#!/usr/bin/env bash
#
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

rsync -av --delete ${PROJECT_DIR}/resources ${PROJECT_DIR}/src \
    ${PROJECT_DIR}/build_cmake.sh ${PROJECT_DIR}/CMakeLists.txt \
    ${PROJECT_DIR}/prd.txt  ${PROJECT_DIR}/naviPlayer_zh_CN.ts \
    /Users/redtux/Documents/Nextcloud/Projects/Qt/naviPlayer/

