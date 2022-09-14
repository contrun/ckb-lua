#!/usr/bin/env bash

file="$1"
size="$(( $2 - 1))"
base_file="$(dirname "$0")/sample_data1.json"
jq -s '.[0] * .[1]' "$base_file" <(printf '{"tx": {"witnesses": ["0x'; for i in $(seq 0 "$size"); do printf "%02x" "$(( i % 255 ))"; done; printf '"]}}') > "$file"
