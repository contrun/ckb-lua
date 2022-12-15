#!/usr/bin/env bash

set -xeuo pipefail

root_dir="${1:-$( cd "$( dirname "${BASH_SOURCE[0]}" )"/../../ && pwd )}"
temp_dir="$(mktemp -p ~ -d "$(basename "$0").XXXXXXXX")"

cleanup() {
  rm -rf "$temp_dir"
}

trap cleanup EXIT INT TERM

declare -a files=("lua_fs_code_in_cell_data" "lua_fs_code_with_module_in_cell_data")
for file in "${files[@]}"; do
  dir="$temp_dir/$file"
  mkdir -p "$dir"
  printf "$(< "$file.json" jq -r '.mock_info.cell_deps[].data' | sed 's/^0x//g' | fold -w 2 | xargs -n 1 printf '\\x%s')" > "$dir/packed"
  lua "$root_dir/utils/fs.lua" unpack "$dir/packed" "$dir/unpacked"
  find "$dir"
  (cd "$dir/unpacked"; lua "main.lua";) | fgrep 'hello world!'
done
