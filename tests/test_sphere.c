/*
 * HERMETIC-Ω (hermetic-array) · tests/test_sphere.c
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

#include "../src/array.h"
#include "../src/geom.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

int main(void) {
    size_t sh[] = {3};
    HaoArray *x = hao_array_create(HAO_SCALAR_F64, 1, sh);
    HaoArray *v = hao_array_create(HAO_SCALAR_F64, 1, sh);
    HaoArray *out = hao_array_create(HAO_SCALAR_F64, 1, sh);
    double *xd = x->data;
    double *vd = v->data;
    xd[0] = 1.0; xd[1] = 0.0; xd[2] = 0.0;
    vd[0] = 0.0; vd[1] = 0.1; vd[2] = 0.0;
    assert(hao_is_unit(xd, 3, 1e-10));
    assert(hao_is_tangent(xd, vd, 3, 1e-10));
    int rc = hao_array_sphere_exp(x, v, out);
    assert(rc == 0);
    double *od = out->data;
    double nrm = sqrt(hao_norm2(od, 3));
    assert(fabs(nrm - 1.0) < 1e-8);
    printf("sphere exp test passed\n");
    hao_array_free(x);
    hao_array_free(v);
    hao_array_free(out);
    return 0;
}
