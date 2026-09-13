#!/usr/bin/env bash
# Compiles the autowindowrefresh plugin against the stub Win32 layer and runs
# the behaviour tests. Works on Linux/macOS (g++) and on Windows via MSYS/Git
# Bash (where it is mainly a sanity check; the real build uses build.bat).
set -u

here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/.." && pwd)"

# MSVC reads BOM-less sources using the system ANSI codepage (CP936 on Chinese
# Windows), where the last UTF-8 byte of a comment can swallow the next line.
# /utf-8 in CMakeLists covers the build; this guard catches the case early.
python3 - "$root" <<'PY'
import pathlib, subprocess, sys
root = pathlib.Path(sys.argv[1])
files = subprocess.check_output(
    ["git", "ls-files", "*.cpp", "*.hpp", "*.h"], cwd=root, text=True).split()
bad = []
for name in files:
    data = (root / name).read_bytes()
    if not data.startswith(b"\xef\xbb\xbf") and any(b > 127 for b in data):
        bad.append(name)
if bad:
    print("FAIL: these sources contain non-ASCII but no UTF-8 BOM:")
    print("\n".join("  " + n for n in bad))
    sys.exit(1)
print("[PASS] every non-ASCII source has a UTF-8 BOM")
PY
[ $? -ne 0 ] && exit 1

cxx="${CXX:-g++}"
out="${OUT:-$here/harness}"

"$cxx" -std=c++17 -Wall -Wextra -Wno-unused-parameter \
    -I"$here" \
    -I"$root/WarcraftHelper/plugin" \
    "$here/harness.cpp" \
    "$here/stub_warcraft.cpp" \
    "$root/WarcraftHelper/plugin/autowindowrefresh.cpp" \
    -o "$out" || exit 1

"$out"
