/*
 * HERMETIC-Ω (hermetic-array) · src/interp.h
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

/* HERMETIC-Ω array-language interpreter (additions only).
   A self-contained C11 interpreter for the BQN-family source under lang/
   (the HAΩ dialect: positional block arguments ⊢ ⊣ ⊤ ⊥ and ℓ, `cond ? a ◇ b`,
   |x| and ∣x∣ magnitude bars, ⌊k⌋ brackets, multi-argument calls, math
   precedence) plus a strict BQN dialect selected with hao_interp_set_bqn.
   Numeric arrays are bridged to the C11 engine in array.c / geom.c and the
   extension kernels in array_ext.c / geom_ext.c. No CBQN dependency. */

#ifndef HAO_INTERP_H
#define HAO_INTERP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>
#include "array.h"

typedef enum {
    HV_NUM,
    HV_CHAR,
    HV_ARR,
    HV_FN,
    HV_M1,
    HV_M2
} HvKind;

typedef enum {
    FK_PRIM,
    FK_BLOCK,
    FK_ATOP,
    FK_FORK,
    FK_D1,
    FK_D2,
    FK_BUILTIN
} HvFnKind;

typedef struct HV HV;
typedef struct HEnv HEnv;
typedef struct HNode HNode;
typedef HV *(*HBuiltin)(size_t nargs, HV **args);

struct HV {
    HvKind k;
    double n;
    uint32_t c;
    size_t rank;
    size_t *shape;
    size_t count;
    HV **el;
    HvFnKind fk;
    uint32_t prim;
    HV *f, *g, *h;
    const HNode *blk;
    HEnv *env;
    HBuiltin bi;
    const char *name;
};

struct HEnv {
    HEnv *parent;
    size_t n, cap;
    const char **names;
    HV **vals;
    bool has_args;
    size_t nargs;
    HV *args[4];
    HV *ell;
    HV *x, *w;
    HV *self;
};

typedef enum {
    N_NUM, N_CHR, N_STR, N_NAME, N_SYS, N_PRIM, N_M1, N_M2, N_ARG,
    N_LIST, N_STRAND, N_ARRLIT, N_BLOCK, N_ASSIGN, N_MONAD, N_DYAD,
    N_D1, N_D2, N_ATOP, N_FORK, N_TERN, N_ABS, N_FLOORB, N_CEILB,
    N_ROTB, N_CALL, N_SEQ, N_NOTHING
} HNodeKind;

typedef enum { R_SUB, R_FN, R_M1, R_M2 } HRole;

struct HNode {
    HNodeKind k;
    HRole role;
    double num;
    char *str;
    uint32_t cp;
    HNode **kids;
    size_t nk;
    int arg;
    bool fn_block;
    bool modify;
    int line;
    const char *file;
};

/* Argument slots for N_ARG */
enum { ARG_X, ARG_W, ARG_SELF_FN, ARG_SELF_SUB, ARG_ELL, ARG_P1, ARG_P2, ARG_P3, ARG_P4, ARG_F, ARG_G };

/* Synthetic primitive code: ⌈ inside an HAΩ block (monadic ceiling, dyadic join). */
#define HP_HAO_CEIL 0x110001u

/* Memory arena: every interpreter object lives until hao_interp_shutdown. */
void *hb_alloc(size_t bytes);
char *hb_strdup(const char *s, size_t len);

/* Errors unwind to the innermost handler. */
typedef struct HbHandler {
    jmp_buf jb;
    struct HbHandler *prev;
} HbHandler;
extern HbHandler *hb_handler;
extern char hb_errmsg[512];
extern char hb_errtext[400];   /* message of the most recent error, without location */
void hb_error(const char *fmt, ...);

/* Dialect */
extern bool hb_bqn_mode;
extern double hb_zero;
double hb_infinity(void);
void hao_interp_set_bqn(bool strict_bqn);

/* Values */
HV *hv_num(double d);
HV *hv_chr(uint32_t c);
HV *hv_arr(size_t rank, const size_t *shape);
HV *hv_list(size_t n);
HV *hv_str(const char *utf8);
HV *hv_prim_fn(uint32_t cp);
bool hv_is_arr(const HV *v);
bool hv_is_fn(const HV *v);
double hv_getnum(const HV *v, const char *who);
size_t hv_len(const HV *v);

/* Calls (args are left to right: monadic {x}, dyadic {w, x}) */
HV *hv_call(HV *f, size_t nargs, HV **args);
HV *hv_call1(HV *f, HV *x);
HV *hv_call2(HV *f, HV *w, HV *x);
HV *hv_mod1(uint32_t m, HV *f);
HV *hv_mod2(uint32_t m, HV *f, HV *g);
HV *hb_prim_monad(uint32_t cp, HV *x);
HV *hb_prim_dyad(uint32_t cp, HV *w, HV *x);
HV *hb_apply_d1(HV *d, size_t nargs, HV **args);
HV *hb_apply_d2(HV *d, size_t nargs, HV **args);

/* Bridging to the C11 engine */
HaoArray *hv_to_hao(const HV *v, const char *who);
HV *hao_to_hv(const HaoArray *a);

/* Formatting */
void hb_format(const HV *v, char **buf, size_t *len, size_t *cap);
char *hb_repr(const HV *v);
bool hv_match(const HV *a, const HV *b);

/* Environment */
HEnv *henv_new(HEnv *parent);
void henv_def(HEnv *e, const char *name, HV *v);
HV *henv_get(HEnv *e, const char *name);
void hb_install_builtins(HEnv *g);
HV *hb_sys_lookup(const char *name);

/* Front end */
HNode *hb_parse(const char *src, size_t len, const char *file);
HV *hb_eval(const HNode *n, HEnv *e);
HV *hb_run_block(const HNode *blk, HEnv *closure, size_t nargs, HV **args, HV *self);

/* Embedding API */
HEnv *hao_interp_init(void);
int hao_interp_run(HEnv *g, const char *src, size_t len, const char *file, HV **result);
int hao_interp_run_file(HEnv *g, const char *path);
void hao_interp_shutdown(void);

#endif
