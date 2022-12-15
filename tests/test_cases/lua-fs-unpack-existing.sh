#!/usr/bin/env bash

set -xeuo pipefail

root_dir="${1:-$( cd "$( dirname "${BASH_SOURCE[0]}" )"/../../ && pwd )}"
temp_dir="$(mktemp -p ~ -d)"

cleanup() {
  rm -rf "$temp_dir"
}

trap cleanup EXIT INT TERM

printf "$(< lua_fs_code_in_cell_data.json jq -r '.mock_info.cell_deps[].data' | sed 's/^0x//g' | fold -w 2 | xargs -n 1 printf '\\x%s')" > "$temp_dir/packed"
lua "$root_dir/utils/fs.lua" unpack "$temp_dir/packed" "$temp_dir/unpacked"
find "$temp_dir"
