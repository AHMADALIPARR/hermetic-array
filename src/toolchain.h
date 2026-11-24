/*
 * HERMETIC-Ω (hermetic-array) · src/toolchain.h
 * SPDX-License-Identifier: LicenseRef-Sovereign-Leviathan-SL-AGPL3-001
 * License-ID: SL-AGPL3-001 | Covenant-Version: 1.0
 * Copyright (C) 2026 SnapKittyWest. Ahmad Ali Parr, Bel Esprit D'Accord Irrevocable Trust.
 * Dual-licensed: GNU AGPL-3.0 (LICENSE) or the Sovereign Leviathan additional terms (LICENSE-SOVEREIGN-LEVIATHAN).
 * Hark, though this node be but a spark,
 * Its covenant endureth through the dark.
 */

#ifndef HAO_TOOLCHAIN_H
#define HAO_TOOLCHAIN_H

#include "array.h"

int hao_toolchain_eval(const char *opname, HaoArray *x, HaoArray *v, double eta, size_t steps, HaoArray **result);
int toolchain_main(int argc, char **argv);

#endif
