#!/bin/bash
# Cross-validate the C calendar library against the Java library.
# Produces two output files and diffs them; exits with 0 iff they match.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
JAVA_PROJ=/Users/itz/IdeaProjects/hebrewcalendar

OUT_JAVA=/tmp/cv_java.txt
OUT_C=/tmp/cv_c.txt
DIFF_OUT=/tmp/cv_diff.txt

HC_BUILD=/tmp/hcbuild
mkdir -p "$HC_BUILD"

echo "=== Building Java library from source ==="
find "$JAVA_PROJ/src" -name "*.java" | xargs javac -d "$HC_BUILD"

echo "=== Building Java oracle ==="
javac -cp "$HC_BUILD" -d /tmp "$SCRIPT_DIR/CrossValidate.java"

echo "=== Running Java oracle (1950-2050, ~292k lines) ==="
time java -cp "/tmp:$HC_BUILD" CrossValidate > "$OUT_JAVA"
echo "Java: $(wc -l < "$OUT_JAVA") lines"

echo ""
echo "=== Building C oracle ==="
gcc -std=c11 -O2 \
    "$PROJ_DIR/src/common.c" \
    "$PROJ_DIR/src/gregorian.c" \
    "$PROJ_DIR/src/julian.c" \
    "$PROJ_DIR/src/hebrew.c" \
    "$PROJ_DIR/src/hconverter.c" \
    "$PROJ_DIR/src/tekufot.c" \
    "$PROJ_DIR/src/parshiot.c" \
    "$PROJ_DIR/src/hc_jewish_dates.c" \
    "$PROJ_DIR/src/noaa.c" \
    "$PROJ_DIR/src/zmanim.c" \
    "$SCRIPT_DIR/cross_validate.c" \
    -I "$PROJ_DIR/src" -lm -o /tmp/cv_c

echo "=== Running C oracle ==="
time /tmp/cv_c > "$OUT_C"
echo "C:    $(wc -l < "$OUT_C") lines"

echo ""
echo "=== Diffing ==="
if diff "$OUT_JAVA" "$OUT_C" > "$DIFF_OUT" 2>&1; then
    echo "PASS — outputs are identical"
    exit 0
else
    NDIFF=$(grep -c '^[<>]' "$DIFF_OUT" || true)
    echo "FAIL — $NDIFF differing lines (see $DIFF_OUT)"
    head -80 "$DIFF_OUT"
    exit 1
fi
