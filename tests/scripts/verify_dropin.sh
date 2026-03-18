#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

PAWNCC_BIN="${PAWNCC_BIN:-/home/blefnk/repos/blefonix/blefonix/apps/omp/qawno/pawncc}"
QAWNO_INCLUDE="${QAWNO_INCLUDE:-/home/blefnk/repos/blefonix/blefonix/apps/omp/qawno/include}"
QAWNO_LIB_DIR="${QAWNO_LIB_DIR:-/home/blefnk/repos/blefonix/blefonix/apps/omp/qawno}"

if [[ ! -x "$PAWNCC_BIN" ]]; then
  echo "PAWNCC_BIN not executable: $PAWNCC_BIN" >&2
  exit 1
fi

mkdir -p "$ROOT_DIR/build/dropin"

export LD_LIBRARY_PATH="$QAWNO_LIB_DIR:${LD_LIBRARY_PATH:-}"

"$PAWNCC_BIN" "$ROOT_DIR/tests/fixtures/dropin_gamemode.pwn" \
  -i"$QAWNO_INCLUDE" \
  -i"$ROOT_DIR/include" \
  -o"$ROOT_DIR/build/dropin/dropin_gamemode.amx"

"$PAWNCC_BIN" "$ROOT_DIR/tests/fixtures/dropin_filterscript.pwn" \
  -i"$QAWNO_INCLUDE" \
  -i"$ROOT_DIR/include" \
  -o"$ROOT_DIR/build/dropin/dropin_filterscript.amx"

echo "drop-in fixtures compiled: OK"
