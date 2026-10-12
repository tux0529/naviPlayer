#!/bin/bash

# ============================================================
# 用法: ./replace_svg_color.sh <前缀> <#RRGGBB>
# 示例: ./replace_svg_color.sh nav_ #FF5733
# 功能: 将当前目录下以 <前缀> 开头的 .svg 文件中所有
#       #XXXXXX 颜色替换为指定颜色
# ============================================================

# ---------- 参数校验 ----------
if [ $# -ne 2 ]; then
    echo "用法: $0 <文件名前缀> <#RRGGBB>" >&2
    echo "示例: $0 nav_ #FF5733" >&2
    exit 1
fi

prefix="$1"
color="$2"

# 前缀不能为空
if [ -z "$prefix" ]; then
    echo "错误: 文件名前缀不能为空。" >&2
    exit 1
fi

# 去掉可能的前导 #
if [[ "$color" == \#* ]]; then
    color="${color#\#}"
fi

# 校验：必须恰好 6 位十六进制
if ! [[ "$color" =~ ^[0-9a-fA-F]{6}$ ]]; then
    echo "错误: '$2' 不是合法的 6 位十六进制颜色值。" >&2
    exit 1
fi

# 统一为小写
color=$(echo "$color" | tr 'ABCDEF' 'abcdef')

# ---------- 平台兼容的 sed -i ----------
if [[ "$(uname)" == "Darwin" ]]; then
    SED_INPLACE=(-i '')   # macOS
else
    SED_INPLACE=(-i)      # Linux
fi

# ---------- 执行替换 ----------
shopt -s nullglob
files=("${prefix}"*.svg)

if [ ${#files[@]} -eq 0 ]; then
    echo "当前目录未找到以 '${prefix}' 开头的 .svg 文件。"
    exit 0
fi

for f in "${files[@]}"; do
    echo "替换: $f  →  #${color}"
    sed "${SED_INPLACE[@]}" "s/#\([0-9a-fA-F]\{6\}\)/#${color}/g" "$f"
done

echo "完成，共处理 ${#files[@]} 个文件。"
