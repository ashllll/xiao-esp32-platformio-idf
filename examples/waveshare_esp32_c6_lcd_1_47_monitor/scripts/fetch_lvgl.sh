#!/usr/bin/env sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
target="$project_dir/components/lvgl"

if [ -d "$target/.git" ]; then
    current=$(git -C "$target" describe --tags --exact-match 2>/dev/null || true)
    if [ "$current" = "v9.2.2" ]; then
        echo "LVGL v9.2.2 already present"
        exit 0
    fi
    echo "Refusing to replace existing LVGL checkout: $target" >&2
    exit 1
fi

if [ -e "$target" ]; then
    echo "Refusing to replace existing path: $target" >&2
    exit 1
fi

git clone --depth 1 --branch v9.2.2 https://github.com/lvgl/lvgl.git "$target"
