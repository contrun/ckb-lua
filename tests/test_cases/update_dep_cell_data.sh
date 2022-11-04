#!/usr/bin/env bash

set -euo pipefail

cell_index=0
in_place_update=y
debug=
filter_file=
content_providing_file=
tx_file=
while getopts "dni:f:c:t:" opt; do
  case $opt in
  d)
    debug=y
    ;;
  n)
    in_place_update=n
    ;;
  i)
    cell_index="$OPTARG"
    ;;
  f)
    filter_file="$OPTARG"
    ;;
  c)
    content_providing_file="$OPTARG"
    ;;
  t)
    tx_file="$OPTARG"
    ;;
  \?)
    echo "Invalid option: -$OPTARG" >&2
    exit 1
    ;;
  esac
done

usage() {
  echo "$0 [-n] [-i cell_index] [-f jq_filter_file] -c content_providing_file -t tx_file"
}

if [[ -z "$content_providing_file" ]] || [[ -z "$tx_file" ]]; then
  usage
  exit 1
fi

remove_filter_file() {
  rm "$filter_file"
}

if [[ -z "$filter_file" ]]; then
  filter_file="$(mktemp --tmpdir "$(basename "$0")"XXX)"
  echo -n ".mock_info.cell_deps[$cell_index].data = "'"0x' >"$filter_file"
  xxd -p <"$content_providing_file" | tr -d '\n' >>"$filter_file"
  echo -n '"' >>"$filter_file"
  if [[ -z "$debug" ]]; then
    trap remove_filter_file EXIT INT TERM
  fi
fi

if [[ "$in_place_update" == y ]]; then
  tmpfile="$(mktemp)"
  jq -f "$filter_file" <"$tx_file" >"$tmpfile"
  mv "$tmpfile" "$tx_file"
else
  jq -f "$filter_file" <"$tx_file"
fi
