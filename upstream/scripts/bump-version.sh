#!/usr/bin/env bash
# Bump the CMake project version.
set -euo pipefail

usage() {
  echo "Usage: $0 <major.minor.patch>"
  echo "Example: $0 2.0.2"
}

if [ "$#" -ne 1 ]; then
  usage >&2
  exit 1
fi

VERSION="$1"

if ! [[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Invalid version: $VERSION" >&2
  echo "Expected a plain CMake project version: major.minor.patch" >&2
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CMAKE_LISTS="$PROJECT_ROOT/CMakeLists.txt"

if [ ! -f "$CMAKE_LISTS" ]; then
  echo "Unable to find CMakeLists.txt at $CMAKE_LISTS" >&2
  exit 1
fi

CURRENT_VERSION=$(sed -n 's/^  VERSION \([0-9][0-9.]*\).*/\1/p' "$CMAKE_LISTS" | head -n 1)

if [ -z "$CURRENT_VERSION" ]; then
  echo "Unable to read project version from $CMAKE_LISTS" >&2
  exit 1
fi

if [ "$CURRENT_VERSION" = "$VERSION" ]; then
  echo "Version is already $VERSION"
  exit 0
fi

if [[ "$OSTYPE" == darwin* ]]; then
  sed -i '' "s/^  VERSION [0-9][0-9.]*/  VERSION $VERSION/" "$CMAKE_LISTS"
else
  sed -i "s/^  VERSION [0-9][0-9.]*/  VERSION $VERSION/" "$CMAKE_LISTS"
fi

echo "Bumped CMake project version: $CURRENT_VERSION -> $VERSION"
