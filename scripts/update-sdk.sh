#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK_DIR="$ROOT/external/open.mp-sdk"
REMOTE="${OPENMP_SDK_REMOTE:-https://github.com/openmultiplayer/open.mp-sdk.git}"
REF="${1:-473b732}"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "[ohmycmd] Updating vendored open.mp-sdk"
echo "  remote: $REMOTE"
echo "  ref:    $REF"

git clone "$REMOTE" "$TMP/open.mp-sdk"
cd "$TMP/open.mp-sdk"
git fetch --all --tags --prune
git checkout "$REF"
git submodule update --init --recursive

FULL_SHA="$(git rev-parse HEAD)"
SHORT_SHA="$(git rev-parse --short=7 HEAD)"

mkdir -p "$SDK_DIR"
rsync -a --delete --exclude='.git' "$TMP/open.mp-sdk/" "$SDK_DIR/"
rm -f "$SDK_DIR/.git"

# Keep vendored SDK lean (docs/tests/CI metadata are not needed for component builds)
rm -rf \
  "$SDK_DIR/lib/glm/doc" \
  "$SDK_DIR/lib/glm/test" \
  "$SDK_DIR/lib/robin-hood-hashing/src/test" \
  "$SDK_DIR/lib/span-lite/scripts" \
  "$SDK_DIR/lib/span-lite/.github" \
  "$SDK_DIR/lib/span-lite/test" \
  "$SDK_DIR/lib/string-view-lite/.github" \
  "$SDK_DIR/lib/string-view-lite/test" \
  "$SDK_DIR/lib/glm/.github" || true
rm -f \
  "$SDK_DIR/.gitmodules" \
  "$SDK_DIR/lib/glm/.appveyor.yml" \
  "$SDK_DIR/lib/glm/.travis.yml" \
  "$SDK_DIR/lib/glm/.gitignore" || true

cat > "$SDK_DIR/VENDORED_COMMIT" <<META
$SHORT_SHA
META

cat > "$SDK_DIR/VENDORED_FROM" <<META
remote=$REMOTE
commit=$SHORT_SHA
mode=vendored-copy
META

echo "[ohmycmd] Vendored SDK updated to $SHORT_SHA ($FULL_SHA)"
