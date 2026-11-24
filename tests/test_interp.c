/*
 * HERMETIC-Ω (hermetic-array) · tests/test_interp.c
 * SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
 * License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
 * Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
 * Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
 * Hark, though this node be but a spark,
 * Its covenant endureth through the dark.
 */

/* HERMETIC ARRAY Ω (HAΩ)
 * Copyright (C) 2026 AHMAD ALI PARR
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../src/interp.h"
#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect(HEnv *g, const char *src, const char *want) {
    HV *r = NULL;
    int rc = hao_interp_run(g, src, strlen(src), "<test>", &r);
    const char *got = (rc == 0 && r) ? hb_repr(r) : hb_errmsg;
    if (rc != 0 || strcmp(got, want) != 0) {
        printf("FAIL %s\n  want: %s\n  got:  %s\n", src, want, got);
        failures++;
    }
}

static void expect_error(HEnv *g, const char *src, const char *fragment) {
    HV *r = NULL;
    int rc = hao_interp_run(g, src, strlen(src), "<test>", &r);
    if (rc == 0 || strstr(hb_errmsg, fragment) == NULL) {
        printf("FAIL %s\n  want error containing: %s\n  got: %s\n", src, fragment, rc ? hb_errmsg : hb_repr(r));
        failures++;
    }
}

int main(void) {
    HEnv *g = hao_interp_init();

    /* HAΩ dialect: math precedence, positional block arguments, bars, brackets */
    expect(g, "2 + 3 × 4", "14");
    expect(g, "2 × 3 + 4", "10");
    expect(g, "2 ⋆ 3 ⋆ 2", "512");
    expect(g, "|3 - 5|", "2");
    expect(g, "⌊1⌋ 1‿2‿3", "⟨ 2 3 1 ⟩");
    expect(g, "⌊¯1⌋ 1‿2‿3", "⟨ 3 1 2 ⟩");
    expect(g, "⌊2.7⌋", "2");
    expect(g, "Add ← {⊢ + ⊣}", "(function)");
    expect(g, "Add 2 5", "7");
    expect(g, "F ← {ℓ × 2} ⋄ F 1‿2", "⟨ 2 4 ⟩");
    expect(g, "Sel ← {⊤ = 0 ? ⊢ ◇ ⊣} ⋄ ⟨Sel 1 2 0, Sel 1 2 1⟩", "⟨ 1 2 ⟩");
    expect(g, "+´×˘ 3‿4", "25");
    expect(g, "Sin 0", "0");
    expect(g, "1‿2 ⊗ 3‿4", "2‿2⥊⟨ 3 4 6 8 ⟩");
    expect(g, "2‿3 ⍡ ↕6", "2‿3⥊⟨ 0 1 2 3 4 5 ⟩");
    expect(g, "•hao.Norm 3‿4", "5");
    expect(g, "•hao.SphereExp 1‿0‿0 0‿0‿0", "⟨ 1 0 0 ⟩");
    expect(g, "1‿0‿0 ⊙ 0‿1‿0", "⟨ 0 0 0 1 ⟩");
    expect(g, "Compose ← ∘ ⋄ (- Compose ⌊) 2.5", "¯2");

    /* standard BQN pieces that read the same in both dialects */
    expect(g, "+´ ↕10", "45");
    expect(g, "⍋ 3‿1‿2", "⟨ 1 2 0 ⟩");
    expect(g, "\"abc\" ∾ \"de\"", "\"abcde\"");
    expect(g, "(+´ ÷ ≠) 1‿2‿3‿4", "2.5");
    expect(g, "{𝕩 × 2}¨ 1‿2", "⟨ 2 4 ⟩");
    expect(g, "1e¯3", "0.001");
    expect(g, "¯1.5 × 2", "¯3");

    /* errors are reported, not crashes */
    expect_error(g, "1‿2 + 1‿2‿3", "length error");
    expect_error(g, "undefinedName", "undefined");
    expect_error(g, "1 ⟕ 2", "undefined primitive");
    expect_error(g, "2‿2 ⍡ ↕6", "length error");
    expect_error(g, "⍣\"boom\"", "boom");
    expect(g, "1 + 1", "2");   /* the session survives errors */

    /* strict BQN mode: right to left, ⊢ ⊣ are functions, ˘ is Cells */
    hao_interp_set_bqn(true);
    HEnv *b = hao_interp_init();
    expect(b, "2 × 3 + 4", "14");
    expect(b, "3 ⊣ 4", "3");
    expect(b, "10 {𝕨 - 𝕩} 3", "7");
    expect(b, "{𝕩 - 1} 10 ⊢ 3", "2");
    expect(b, "+´˘ 2‿2⥊1‿2‿3‿4", "⟨ 3 7 ⟩");
    expect(b, "•Repr ⟨1‿2, \"x\"⟩", "\"⟨1‿2,\"\"x\"\"⟩\"");
    hao_interp_set_bqn(false);

    hao_interp_shutdown();
    if (failures) { printf("interp tests: %d failure(s)\n", failures); return 1; }
    printf("interp tests passed\n");
    return 0;
}
