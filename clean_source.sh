#!/bin/bash
# clean_source.sh
# 用于清理整个工程的源码文件，让 Windows 可以正常编译

# 1. 找到所有 .cpp 和 .h 文件
find ./ -type f \( -name "*.cpp" -o -name "*.h" \) | while read file
do
    echo "Cleaning $file"

    # 2. 转为 UTF-8 无 BOM
    iconv -f UTF-8 -t UTF-8 "$file" -o "$file.tmp" || continue
    mv "$file.tmp" "$file"

    # 3. 删除所有非 ASCII 字符（0x00~0x7F 以外）
    LC_ALL=C tr -cd '\11\12\15\40-\176' < "$file" > "$file.tmp"
    mv "$file.tmp" "$file"

    # 4. 统一换行符为 CRLF
    unix2dos "$file" 2>/dev/null
done

echo "✅ 所有文件已清理完成"
