#!/usr/bin/env bash
# Generate a lua file system from current repo, unpack files from the generated file, 
# and then compare their contents.

set -xeuo pipefail

root_dir="${1:-$( cd "$( dirname "${BASH_SOURCE[0]}" )"/../../ && pwd )}"
temp_dir="$(mktemp -p ~ -d)"

cleanup() {
  rm -rf "$temp_dir"
}

trap cleanup EXIT INT TERM

initial_dir="$temp_dir/initial"
final_dir="$temp_dir/final"
packed_file="$temp_dir/packed"
cp -r "$root_dir" "$initial_dir"
cd "$initial_dir" || exit 1
# Our fs util only packs files. Deleting empty directories and other things to make comparison below work.
find . -depth -type d -empty -delete
find . -not \( -type d -or -type f \) -delete
find . -type f | lua ./utils/fs.lua pack "$packed_file" > /dev/null
lua ./utils/fs.lua unpack "$packed_file" "$final_dir" > /dev/null
diff --brief --recursive "$initial_dir" "$final_dir"
