# HERMETIC ARRAY Ω (HAΩ)
# Copyright (C) 2026 AHMAD ALI PARR
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# HERMETIC-Ω BQN test runner (additions only).
#   sh tests/run_bqn_tests.sh path/to/hao-bqn
# Each tests/bqn/*.bqn names the original lang files it needs on a
# "# load:" line and optional interpreter flags on a "# flags:" line.
# Output must match tests/bqn/NAME.expected when that file exists.
# If CBQN is set to a CBQN binary, files with "# flags: -b" (plain BQN)
# are also run under CBQN as the reference implementation.

HB=${1:-bin-ccomp/hao-bqn}
fail=0
pass=0
tmp=${TMPDIR:-/tmp}/hao-bqn-test.$$
for t in tests/bqn/*.bqn; do
  name=$(basename "$t" .bqn)
  load=$(sed -n 's/^# load:[ ]*//p' "$t" | head -n 1)
  flags=$(sed -n 's/^# flags:[ ]*//p' "$t" | head -n 1)
  if "$HB" $flags $load "$t" > "$tmp" 2>&1; then
    if [ -f "tests/bqn/$name.expected" ] && ! cmp -s "$tmp" "tests/bqn/$name.expected"; then
      echo "FAIL $name (output differs)"
      diff "tests/bqn/$name.expected" "$tmp"
      fail=$((fail + 1))
    else
      echo "PASS $name"
      pass=$((pass + 1))
    fi
  else
    echo "FAIL $name"
    cat "$tmp"
    fail=$((fail + 1))
  fi
  if [ -n "$CBQN" ] && [ "$flags" = "-b" ]; then
    if "$CBQN" "$t" > "$tmp" 2>&1 && cmp -s "$tmp" "tests/bqn/$name.expected"; then
      echo "PASS $name (CBQN reference)"
    else
      echo "FAIL $name (CBQN reference)"
      cat "$tmp"
      fail=$((fail + 1))
    fi
  fi
done
rm -f "$tmp"
echo "bqn tests: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
