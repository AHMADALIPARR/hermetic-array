/*
 * HERMETIC-Ω (hermetic-array) · src/interp_prim.c
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

/* HERMETIC-Ω interpreter runtime: values, calls, primitive functions,
   primitive modifiers, formatting, system values and the bridge to the
   C11 array engine (array.c, geom.c, array_ext.c, geom_ext.c). */

#include "interp.h"
#include "array.h"
#include "geom.h"
#include "array_ext.h"
#include "geom_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* ---------- values ---------- */

HV *hv_num(double d) {
    HV *v = hb_alloc(sizeof(HV));
    v->k = HV_NUM;
    v->n = d;
    return v;
}

HV *hv_chr(uint32_t c) {
    HV *v = hb_alloc(sizeof(HV));
    v->k = HV_CHAR;
    v->c = c;
    return v;
}

HV *hv_arr(size_t rank, const size_t *shape) {
    HV *v = hb_alloc(sizeof(HV));
    v->k = HV_ARR;
    v->rank = rank;
    size_t count = 1;
    if (rank) {
        v->shape = hb_alloc(rank * sizeof(size_t));
        memcpy(v->shape, shape, rank * sizeof(size_t));
        for (size_t i = 0; i < rank; i++) count *= shape[i];
    }
    v->count = count;
    v->el = hb_alloc((count ? count : 1) * sizeof(HV *));
    return v;
}

HV *hv_list(size_t n) {
    size_t sh[1] = {n};
    return hv_arr(1, sh);
}

static uint32_t utf8_decode(const char *s, size_t len, size_t *i) {
    const unsigned char *u = (const unsigned char *)s;
    uint32_t c = u[*i];
    int extra = 0;
    if (c < 0x80) extra = 0;
    else if ((c & 0xE0) == 0xC0) { c &= 0x1F; extra = 1; }
    else if ((c & 0xF0) == 0xE0) { c &= 0x0F; extra = 2; }
    else if ((c & 0xF8) == 0xF0) { c &= 0x07; extra = 3; }
    (*i)++;
    for (int k = 0; k < extra && *i < len; k++) { c = (c << 6) | (u[*i] & 0x3F); (*i)++; }
    return c;
}

HV *hv_str(const char *utf8) {
    size_t len = strlen(utf8), i = 0, n = 0;
    while (i < len) { utf8_decode(utf8, len, &i); n++; }
    HV *l = hv_list(n);
    i = 0;
    for (size_t k = 0; k < n; k++) l->el[k] = hv_chr(utf8_decode(utf8, len, &i));
    return l;
}

HV *hv_prim_fn(uint32_t cp) {
    HV *f = hb_alloc(sizeof(HV));
    f->k = HV_FN;
    f->fk = FK_PRIM;
    f->prim = cp;
    return f;
}

bool hv_is_arr(const HV *v) { return v->k == HV_ARR; }
bool hv_is_fn(const HV *v) { return v->k == HV_FN; }

static size_t put_utf8(uint32_t c, char *b) {
    size_t n = 0;
    if (c < 0x80) b[n++] = (char)c;
    else if (c < 0x800) { b[n++] = (char)(0xC0 | (c >> 6)); b[n++] = (char)(0x80 | (c & 0x3F)); }
    else if (c < 0x10000) { b[n++] = (char)(0xE0 | (c >> 12)); b[n++] = (char)(0x80 | ((c >> 6) & 0x3F)); b[n++] = (char)(0x80 | (c & 0x3F)); }
    else { b[n++] = (char)(0xF0 | (c >> 18)); b[n++] = (char)(0x80 | ((c >> 12) & 0x3F)); b[n++] = (char)(0x80 | ((c >> 6) & 0x3F)); b[n++] = (char)(0x80 | (c & 0x3F)); }
    return n;
}

static char glyph_buf[8][8];
static int glyph_slot = 0;
static const char *glyph_str(uint32_t c) {
    glyph_slot = (glyph_slot + 1) & 7;
    char *b = glyph_buf[glyph_slot];
    if (c == HP_HAO_CEIL) c = 0x2308;
    b[put_utf8(c, b)] = 0;
    return b;
}

double hv_getnum(const HV *v, const char *who) {
    if (v->k == HV_ARR && v->rank == 0) v = v->el[0];
    if (v->k != HV_NUM) hb_error("domain error: %s expects a number", who);
    return v->n;
}

size_t hv_len(const HV *v) {
    if (v->k != HV_ARR || v->rank == 0) return 1;
    return v->shape[0];
}

static int64_t hv_getint(const HV *v, const char *who) {
    double d = hv_getnum(v, who);
    if (d != floor(d) || fabs(d) > 9e15) hb_error("domain error: %s expects an integer", who);
    return (int64_t)d;
}

static HV *enclose0(HV *x) {
    HV *r = hv_arr(0, NULL);
    r->el[0] = x;
    return r;
}

static HV *as_list(HV *x) {
    if (x->k == HV_ARR && x->rank == 1) return x;
    if (x->k == HV_ARR) {
        HV *l = hv_list(x->count);
        memcpy(l->el, x->el, x->count * sizeof(HV *));
        return l;
    }
    HV *l = hv_list(1);
    l->el[0] = x;
    return l;
}

/* ---------- cells ---------- */

static size_t cell_count(const HV *a) {
    size_t c = 1;
    for (size_t i = 1; i < a->rank; i++) c *= a->shape[i];
    return c;
}

static HV *major_cell(const HV *a, size_t i) {
    if (a->rank == 0) return a->el[0];
    if (a->rank == 1) return a->el[i];
    HV *c = hv_arr(a->rank - 1, a->shape + 1);
    memcpy(c->el, a->el + i * c->count, c->count * sizeof(HV *));
    return c;
}

/* Combine results into an array of shape frame ++ cellshape. */
static HV *merge_frame(size_t frank, const size_t *fshape, size_t n, HV **cells) {
    size_t crank = 0;
    const size_t *cshape = NULL;
    if (n > 0 && cells[0]->k == HV_ARR) { crank = cells[0]->rank; cshape = cells[0]->shape; }
    for (size_t i = 1; i < n; i++) {
        size_t r = cells[i]->k == HV_ARR ? cells[i]->rank : 0;
        if (r != crank) hb_error("rank error: cells of different ranks cannot be merged");
        for (size_t k = 0; k < r; k++) if (cells[i]->shape[k] != cshape[k]) hb_error("length error: cells of different shapes cannot be merged");
    }
    size_t rank = frank + crank;
    if (rank > 64) hb_error("rank error: rank too large");
    size_t sh[64];
    if (frank) memcpy(sh, fshape, frank * sizeof(size_t));
    if (crank) memcpy(sh + frank, cshape, crank * sizeof(size_t));
    HV *r = hv_arr(rank, sh);
    size_t csz = 1;
    for (size_t k = 0; k < crank; k++) csz *= cshape[k];
    for (size_t i = 0; i < n; i++) {
        if (cells[i]->k == HV_ARR) memcpy(r->el + i * csz, cells[i]->el, csz * sizeof(HV *));
        else r->el[i] = cells[i];
    }
    return r;
}

static HV *merge_list(size_t n, HV **cells) {
    size_t sh[1] = {n};
    return merge_frame(1, sh, n, cells);
}

/* ---------- calls ---------- */

static HV *apply_atop_fork(HV *f, size_t nargs, HV **args) {
    if (f->fk == FK_ATOP) {
        HV *r = hv_call(f->h, nargs, args);
        return hv_call1(f->g, r);
    }
    HV *right = hv_call(f->h, nargs, args);
    HV *left = hv_is_fn(f->f) ? hv_call(f->f, nargs, args) : f->f;
    return hv_call2(f->g, left, right);
}

HV *hv_call(HV *f, size_t nargs, HV **args) {
    if (f->k == HV_M1 || f->k == HV_M2) hb_error("a modifier cannot be called as a function");
    if (f->k != HV_FN) return f;
    switch (f->fk) {
        case FK_PRIM:
            if (nargs == 1) return hb_prim_monad(f->prim, args[0]);
            if (nargs == 2) return hb_prim_dyad(f->prim, args[0], args[1]);
            hb_error("valence error: primitive %s takes 1 or 2 arguments, got %d", glyph_str(f->prim), (int)nargs);
            return NULL;
        case FK_BLOCK: return hb_run_block(f->blk, f->env, nargs, args, f);
        case FK_BUILTIN: return f->bi(nargs, args);
        case FK_ATOP: case FK_FORK:
            if (nargs < 1 || nargs > 2) hb_error("valence error: trains take 1 or 2 arguments");
            return apply_atop_fork(f, nargs, args);
        case FK_D1:
            if (nargs < 1 || nargs > 2) hb_error("valence error: derived functions take 1 or 2 arguments");
            return hb_apply_d1(f, nargs, args);
        case FK_D2:
            if (nargs < 1 || nargs > 2) hb_error("valence error: derived functions take 1 or 2 arguments");
            return hb_apply_d2(f, nargs, args);
    }
    return NULL;
}

HV *hv_call1(HV *f, HV *x) { HV *a[1]; a[0] = x; return hv_call(f, 1, a); }
HV *hv_call2(HV *f, HV *w, HV *x) { HV *a[2]; a[0] = w; a[1] = x; return hv_call(f, 2, a); }

HV *hv_mod1(uint32_t m, HV *f) {
    HV *d = hb_alloc(sizeof(HV));
    d->k = HV_FN;
    d->fk = FK_D1;
    d->prim = m;
    d->f = f;
    return d;
}

HV *hv_mod2(uint32_t m, HV *f, HV *g) {
    HV *d = hb_alloc(sizeof(HV));
    d->k = HV_FN;
    d->fk = FK_D2;
    d->prim = m;
    d->f = f;
    d->g = g;
    return d;
}

/* ---------- comparison ---------- */

static int cmp_val(const HV *a, const HV *b);

static int cmp_atom(const HV *a, const HV *b) {
    if (a->k == HV_NUM && b->k == HV_NUM) return (a->n < b->n) ? -1 : (a->n > b->n) ? 1 : 0;
    if (a->k == HV_CHAR && b->k == HV_CHAR) return (a->c < b->c) ? -1 : (a->c > b->c) ? 1 : 0;
    if (a->k == HV_NUM && b->k == HV_CHAR) return -1;
    if (a->k == HV_CHAR && b->k == HV_NUM) return 1;
    hb_error("domain error: cannot compare functions");
    return 0;
}

static int cmp_val(const HV *a, const HV *b) {
    bool aa = a->k == HV_ARR, ba = b->k == HV_ARR;
    if (!aa && !ba) return cmp_atom(a, b);
    if (!aa) { if (b->count == 0) return 1; int c = cmp_val(a, b->el[0]); return c ? c : -1; }
    if (!ba) { if (a->count == 0) return -1; int c = cmp_val(a->el[0], b); return c ? c : 1; }
    size_t n = a->count < b->count ? a->count : b->count;
    for (size_t i = 0; i < n; i++) { int c = cmp_val(a->el[i], b->el[i]); if (c) return c; }
    if (a->count != b->count) return a->count < b->count ? -1 : 1;
    if (a->rank != b->rank) return a->rank < b->rank ? -1 : 1;
    return 0;
}

bool hv_match(const HV *a, const HV *b) {
    if (a->k != b->k) return false;
    switch (a->k) {
        case HV_NUM: return a->n == b->n || (a->n != a->n && b->n != b->n);
        case HV_CHAR: return a->c == b->c;
        case HV_ARR:
            if (a->rank != b->rank || a->count != b->count) return false;
            for (size_t i = 0; i < a->rank; i++) if (a->shape[i] != b->shape[i]) return false;
            for (size_t i = 0; i < a->count; i++) if (!hv_match(a->el[i], b->el[i])) return false;
            return true;
        default: return a == b;
    }
}

/* ---------- pervasive arithmetic ---------- */

static bool is_arith(uint32_t c) {
    switch (c) {
        case '+': case '-': case 0xD7: case 0xF7: case 0x22C6: case 0x221A: case 0x230A:
        case 0x2308: case HP_HAO_CEIL: case '|': case 0x2223: case 0xAC: case 0x2227: case 0x2228:
        case '<': case '>': case 0x2264: case 0x2265: case '=': case 0x2260:
            return true;
        default: return false;
    }
}

static HV *arith1(uint32_t op, HV *x) {
    if (x->k != HV_NUM) {
        if (x->k == HV_CHAR && op == '+') return x;
        hb_error("domain error: %s expects a number", glyph_str(op));
    }
    double v = x->n;
    switch (op) {
        case '+': return x;
        case '-': return hv_num(-v);
        case 0xD7: return hv_num(v > 0 ? 1.0 : v < 0 ? -1.0 : 0.0);
        case 0xF7: return hv_num(1.0 / v);
        case 0x22C6: return hv_num(exp(v));
        case 0x221A: return hv_num(sqrt(v));
        case 0x230A: return hv_num(floor(v));
        case 0x2308: case HP_HAO_CEIL: return hv_num(ceil(v));
        case '|': case 0x2223: return hv_num(fabs(v));
        case 0xAC: return hv_num(1.0 - v);
        default: hb_error("internal: arith1"); return NULL;
    }
}

static HV *arith2(uint32_t op, HV *a, HV *b) {
    switch (op) {
        case '<': return hv_num(cmp_atom(a, b) < 0);
        case '>': return hv_num(cmp_atom(a, b) > 0);
        case 0x2264: return hv_num(cmp_atom(a, b) <= 0);
        case 0x2265: return hv_num(cmp_atom(a, b) >= 0);
        case '=': return hv_num(a->k == b->k && cmp_atom(a, b) == 0);
        case 0x2260: return hv_num(!(a->k == b->k && cmp_atom(a, b) == 0));
        default: break;
    }
    if (op == '+') {
        if (a->k == HV_CHAR && b->k == HV_NUM) return hv_chr((uint32_t)((int64_t)a->c + (int64_t)b->n));
        if (a->k == HV_NUM && b->k == HV_CHAR) return hv_chr((uint32_t)((int64_t)b->c + (int64_t)a->n));
    }
    if (op == '-') {
        if (a->k == HV_CHAR && b->k == HV_NUM) return hv_chr((uint32_t)((int64_t)a->c - (int64_t)b->n));
        if (a->k == HV_CHAR && b->k == HV_CHAR) return hv_num((double)a->c - (double)b->c);
    }
    if (a->k != HV_NUM || b->k != HV_NUM) hb_error("domain error: %s expects numbers", glyph_str(op));
    double x = a->n, y = b->n;
    switch (op) {
        case '+': return hv_num(x + y);
        case '-': return hv_num(x - y);
        case 0xD7: case 0x2227: return hv_num(x * y);
        case 0xF7: return hv_num(x / y);
        case 0x22C6: return hv_num(pow(x, y));
        case 0x221A: return hv_num(pow(y, 1.0 / x));
        case 0x230A: return hv_num(x < y ? x : y);
        case 0x2308: return hv_num(x > y ? x : y);
        case '|': case 0x2223: return hv_num(x == 0 ? y : y - x * floor(y / x));
        case 0x2228: return hv_num(x + y - x * y);
        case 0xAC: return hv_num(1.0 + x - y);
        default: hb_error("internal: arith2"); return NULL;
    }
}

static HV *perv1(uint32_t op, HV *x) {
    if (x->k != HV_ARR) return arith1(op, x);
    HV *r = hv_arr(x->rank, x->shape);
    for (size_t i = 0; i < x->count; i++) r->el[i] = perv1(op, x->el[i]);
    return r;
}

typedef HV *(*Pair2)(void *ctx, HV *a, HV *b);

/* Leading-axis agreement shared by pervasive functions and each. */
static HV *pairwise(HV *w, HV *x, Pair2 fn, void *ctx, bool deep) {
    bool wa = w->k == HV_ARR, xa = x->k == HV_ARR;
    if (!wa && !xa) return fn(ctx, w, x);
    if (!wa || !xa) {
        HV *arr = wa ? w : x;
        HV *r = hv_arr(arr->rank, arr->shape);
        for (size_t i = 0; i < arr->count; i++) {
            HV *a = wa ? arr->el[i] : w, *b = wa ? x : arr->el[i];
            r->el[i] = deep ? pairwise(a, b, fn, ctx, true) : fn(ctx, a, b);
        }
        return r;
    }
    HV *hi = w->rank >= x->rank ? w : x;
    HV *lo = w->rank >= x->rank ? x : w;
    for (size_t i = 0; i < lo->rank; i++) {
        if (lo->shape[i] != hi->shape[i]) hb_error("length error: shapes do not agree");
    }
    HV *r = hv_arr(hi->rank, hi->shape);
    size_t c = lo->count ? hi->count / lo->count : 0;
    for (size_t i = 0; i < lo->count; i++) {
        for (size_t j = 0; j < c; j++) {
            HV *a = (hi == w) ? hi->el[i * c + j] : lo->el[i];
            HV *b = (hi == w) ? lo->el[i] : hi->el[i * c + j];
            r->el[i * c + j] = deep ? pairwise(a, b, fn, ctx, true) : fn(ctx, a, b);
        }
    }
    return r;
}

static HV *arith_pair(void *ctx, HV *a, HV *b) {
    uint32_t op = *(uint32_t *)ctx;
    return arith2(op, a, b);
}

static HV *perv2(uint32_t op, HV *w, HV *x) {
    return pairwise(w, x, arith_pair, &op, true);
}

/* ---------- structural helpers ---------- */

static HV *deshape(HV *x) {
    if (x->k != HV_ARR) { HV *l = hv_list(1); l->el[0] = x; return l; }
    HV *l = hv_list(x->count);
    memcpy(l->el, x->el, x->count * sizeof(HV *));
    return l;
}

static HV *fill_of(HV *x) {
    if (x->k == HV_ARR && x->count > 0) {
        HV *e = x->el[0];
        if (e->k == HV_CHAR) return hv_chr(' ');
        if (e->k == HV_ARR) {
            HV *r = hv_arr(e->rank, e->shape);
            for (size_t i = 0; i < r->count; i++) r->el[i] = fill_of(enclose0(e->el[i]));
            return r;
        }
    }
    return hv_num(0);
}

static HV *join2(HV *w, HV *x) {
    size_t wr = w->k == HV_ARR ? w->rank : 0, xr = x->k == HV_ARR ? x->rank : 0;
    if (wr == 0 && xr == 0) {
        HV *l = hv_list(2);
        l->el[0] = w->k == HV_ARR ? w->el[0] : w;
        l->el[1] = x->k == HV_ARR ? x->el[0] : x;
        return l;
    }
    size_t r = wr > xr ? wr : xr;
    if (r > 64) hb_error("rank error");
    if ((wr + 1 < r) || (xr + 1 < r)) hb_error("rank error: ∾ ranks differ by more than one");
    HV *hi = wr >= xr ? w : x;
    if (wr == r && xr == r) {
        for (size_t i = 1; i < r; i++) if (w->shape[i] != x->shape[i]) hb_error("length error: ∾ cell shapes differ");
    } else {
        HV *lo = wr < xr ? w : x;
        size_t lr = wr < xr ? wr : xr;
        for (size_t i = 0; i < lr; i++) if (lo->shape[i] != hi->shape[i + 1]) hb_error("length error: ∾ cell shapes differ");
    }
    size_t wl = (wr == r) ? w->shape[0] : 1, xl = (xr == r) ? x->shape[0] : 1;
    size_t sh[64];
    sh[0] = wl + xl;
    for (size_t i = 1; i < r; i++) sh[i] = hi->shape[i];
    HV *res = hv_arr(r, sh);
    size_t wc = (w->k == HV_ARR) ? w->count : 1;
    if (w->k == HV_ARR) memcpy(res->el, w->el, wc * sizeof(HV *)); else res->el[0] = w;
    if (x->k == HV_ARR) memcpy(res->el + wc, x->el, x->count * sizeof(HV *)); else res->el[wc] = x;
    return res;
}

static HV *take(int64_t n, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) x = as_list(x);
    size_t len = x->shape[0], k = (size_t)(n < 0 ? -n : n);
    size_t cs = cell_count(x);
    size_t sh[64];
    memcpy(sh, x->shape, x->rank * sizeof(size_t));
    sh[0] = k;
    HV *r = hv_arr(x->rank, sh);
    HV *fill = NULL;
    for (size_t i = 0; i < k; i++) {
        int64_t src = (n >= 0) ? (int64_t)i : (int64_t)len - (int64_t)k + (int64_t)i;
        for (size_t j = 0; j < cs; j++) {
            if (src >= 0 && (size_t)src < len) r->el[i * cs + j] = x->el[(size_t)src * cs + j];
            else { if (!fill) fill = fill_of(x); r->el[i * cs + j] = fill; }
        }
    }
    return r;
}

static HV *drop(int64_t n, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) x = as_list(x);
    int64_t len = (int64_t)x->shape[0];
    int64_t k = n < 0 ? -n : n;
    if (k > len) k = len;
    return take(n >= 0 ? -(len - k) : (len - k), x);
}

static HV *num_list(size_t n, const double *d) {
    HV *l = hv_list(n);
    for (size_t i = 0; i < n; i++) l->el[i] = hv_num(d[i]);
    return l;
}

static HV *range(HV *x) {
    if (x->k != HV_ARR) {
        int64_t n = hv_getint(x, "↕");
        if (n < 0) hb_error("domain error: ↕ of a negative number");
        HV *l = hv_list((size_t)n);
        for (int64_t i = 0; i < n; i++) l->el[i] = hv_num((double)i);
        return l;
    }
    if (x->rank != 1) hb_error("rank error: ↕ expects a number or a list");
    size_t r = x->count, sh[64];
    if (r > 64) hb_error("rank error");
    for (size_t i = 0; i < r; i++) sh[i] = (size_t)hv_getint(x->el[i], "↕");
    HV *res = hv_arr(r, sh);
    for (size_t lin = 0; lin < res->count; lin++) {
        HV *idx = hv_list(r);
        size_t rem = lin;
        for (size_t i = r; i-- > 0;) { idx->el[i] = hv_num((double)(rem % sh[i])); rem /= sh[i]; }
        res->el[lin] = idx;
    }
    return res;
}

static HV *reverse(HV *x) {
    if (x->k != HV_ARR || x->rank == 0) return x;
    size_t n = x->shape[0], cs = cell_count(x);
    HV *r = hv_arr(x->rank, x->shape);
    for (size_t i = 0; i < n; i++) memcpy(r->el + i * cs, x->el + (n - 1 - i) * cs, cs * sizeof(HV *));
    return r;
}

static bool all_numeric(const HV *x) {
    if (x->k == HV_NUM) return true;
    if (x->k != HV_ARR) return false;
    for (size_t i = 0; i < x->count; i++) if (x->el[i]->k != HV_NUM) return false;
    return true;
}

static HV *rotate(int64_t k, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) return x;
    size_t n = x->shape[0];
    if (n == 0) return x;
    if (all_numeric(x)) {
        /* engine path: hao_rotate moves index i to i+k, so BQN k⌽ is hao_rotate(-k) */
        HaoArray *a = hv_to_hao(x, "⌽"), *o = NULL;
        int64_t kk = k % (int64_t)n;
        int rc = hao_rotate(a, 0, (int)(-kk), &o);
        hao_array_free(a);
        if (rc) hb_error("⌽: engine error %d", rc);
        HV *r = hao_to_hv(o);
        hao_array_free(o);
        return r;
    }
    size_t cs = cell_count(x);
    HV *r = hv_arr(x->rank, x->shape);
    int64_t s = ((k % (int64_t)n) + (int64_t)n) % (int64_t)n;
    for (size_t i = 0; i < n; i++) memcpy(r->el + i * cs, x->el + ((i + (size_t)s) % n) * cs, cs * sizeof(HV *));
    return r;
}

/* Generic axis reorder: result axis j is source axis perm[j]. */
static HV *reorder(HV *x, const size_t *perm) {
    size_t r = x->rank, sh[64], idx[64], nidx[64];
    for (size_t j = 0; j < r; j++) sh[j] = x->shape[perm[j]];
    HV *res = hv_arr(r, sh);
    for (size_t lin = 0; lin < x->count; lin++) {
        size_t rem = lin;
        for (size_t i = r; i-- > 0;) { idx[i] = rem % x->shape[i]; rem /= x->shape[i]; }
        for (size_t j = 0; j < r; j++) nidx[j] = idx[perm[j]];
        size_t o = 0;
        for (size_t j = 0; j < r; j++) o = o * sh[j] + nidx[j];
        res->el[o] = x->el[lin];
    }
    return res;
}

static HV *transpose_bqn(HV *x, bool inverse) {
    if (x->k != HV_ARR || x->rank < 2) return x;
    size_t r = x->rank, perm[64];
    if (r > 64) hb_error("rank error");
    /* ⍉: first axis moves to the end; ⍉⁼ moves the last axis to the front */
    for (size_t j = 0; j < r; j++) perm[j] = inverse ? (j + r - 1) % r : (j + 1) % r;
    return reorder(x, perm);
}

static HV *grade(HV *x, bool down) {
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: grade needs an array of rank ≥ 1");
    size_t n = x->shape[0];
    HV **cells = hb_alloc((n ? n : 1) * sizeof(HV *));
    for (size_t i = 0; i < n; i++) cells[i] = major_cell(x, i);
    size_t *g = hb_alloc((n ? n : 1) * sizeof(size_t));
    for (size_t i = 0; i < n; i++) g[i] = i;
    for (size_t i = 1; i < n; i++) {
        size_t cur = g[i], j = i;
        while (j > 0) {
            int c = cmp_val(cells[g[j - 1]], cells[cur]);
            if (down ? (c >= 0) : (c <= 0)) break;
            g[j] = g[j - 1];
            j--;
        }
        g[j] = cur;
    }
    HV *r = hv_list(n);
    for (size_t i = 0; i < n; i++) r->el[i] = hv_num((double)g[i]);
    return r;
}

static HV *select_cells(HV *w, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: ⊏ needs an array to select from");
    size_t n = x->shape[0];
    HV *idx = (w->k == HV_ARR) ? w : enclose0(w);
    HV **cells = hb_alloc((idx->count ? idx->count : 1) * sizeof(HV *));
    for (size_t i = 0; i < idx->count; i++) {
        int64_t k = hv_getint(idx->el[i], "⊏");
        if (k < 0) k += (int64_t)n;
        if (k < 0 || (size_t)k >= n) hb_error("index error: ⊏ index out of range");
        cells[i] = major_cell(x, (size_t)k);
    }
    if (w->k != HV_ARR) return cells[0];
    if (idx->count == 0) {
        size_t sh[64];
        if (idx->rank + x->rank - 1 > 64) hb_error("rank error");
        memcpy(sh, idx->shape, idx->rank * sizeof(size_t));
        memcpy(sh + idx->rank, x->shape + 1, (x->rank - 1) * sizeof(size_t));
        return hv_arr(idx->rank + x->rank - 1, sh);
    }
    return merge_frame(idx->rank, idx->shape, idx->count, cells);
}

static HV *pick(HV *w, HV *x) {
    if (x->k != HV_ARR) hb_error("rank error: ⊑ needs an array");
    if (w->k != HV_ARR) {
        int64_t k = hv_getint(w, "⊑");
        if (x->rank != 1) hb_error("rank error: ⊑ with a number index needs a list");
        if (k < 0) k += (int64_t)x->count;
        if (k < 0 || (size_t)k >= x->count) hb_error("index error: ⊑ index out of range");
        return x->el[k];
    }
    if (w->count != x->rank) hb_error("rank error: ⊑ index length must equal rank");
    size_t o = 0;
    for (size_t i = 0; i < x->rank; i++) {
        int64_t k = hv_getint(w->el[i], "⊑");
        if (k < 0) k += (int64_t)x->shape[i];
        if (k < 0 || (size_t)k >= x->shape[i]) hb_error("index error: ⊑ index out of range");
        o = o * x->shape[i] + (size_t)k;
    }
    return x->el[o];
}

static HV *replicate(HV *w, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) x = as_list(x);
    size_t n = x->shape[0];
    if (w->k == HV_ARR && w->count != n) hb_error("length error: / lengths differ");
    size_t total = 0;
    for (size_t i = 0; i < n; i++) {
        int64_t k = hv_getint(w->k == HV_ARR ? w->el[i] : w, "/");
        if (k < 0) hb_error("domain error: / with a negative count");
        total += (size_t)k;
    }
    if (total == 0) {
        size_t sh[64];
        memcpy(sh, x->shape, x->rank * sizeof(size_t));
        sh[0] = 0;
        return hv_arr(x->rank, sh);
    }
    HV **cells = hb_alloc(total * sizeof(HV *));
    size_t q = 0;
    for (size_t i = 0; i < n; i++) {
        int64_t k = hv_getint(w->k == HV_ARR ? w->el[i] : w, "/");
        for (int64_t j = 0; j < k; j++) cells[q++] = major_cell(x, i);
    }
    return merge_list(total, cells);
}

static HV *indices(HV *x) {
    x = as_list(x);
    size_t total = 0;
    for (size_t i = 0; i < x->count; i++) {
        int64_t k = hv_getint(x->el[i], "/");
        if (k < 0) hb_error("domain error: / with a negative count");
        total += (size_t)k;
    }
    HV *r = hv_list(total);
    size_t q = 0;
    for (size_t i = 0; i < x->count; i++) {
        int64_t k = hv_getint(x->el[i], "/");
        for (int64_t j = 0; j < k; j++) r->el[q++] = hv_num((double)i);
    }
    return r;
}

static HV *reshape_cycle(HV *w, HV *x, bool exact) {
    HV *wl = as_list(w);
    size_t r = wl->count, sh[64], count = 1;
    if (r > 64) hb_error("rank error");
    for (size_t i = 0; i < r; i++) {
        int64_t k = hv_getint(wl->el[i], exact ? "⍡" : "⥊");
        if (k < 0) hb_error("domain error: negative shape");
        sh[i] = (size_t)k;
        count *= sh[i];
    }
    HV *src = deshape(x);
    if (exact) {
        if (count != src->count) hb_error("length error: ⍡ needs %d elements, got %d", (int)count, (int)src->count);
        if (all_numeric(src) && r > 0) {
            HaoArray *a = hv_to_hao(src, "⍡");
            HaoArray *o = hao_reshape(a, r, sh);
            hao_array_free(a);
            if (!o) hb_error("⍡: engine reshape failed");
            HV *res = hao_to_hv(o);
            hao_array_free(o);
            return res;
        }
    }
    if (count > 0 && src->count == 0) hb_error("length error: reshape of an empty array");
    HV *res = hv_arr(r, sh);
    for (size_t i = 0; i < count; i++) res->el[i] = src->el[i % src->count];
    return res;
}

static size_t depth(const HV *x) {
    if (x->k != HV_ARR) return 0;
    size_t m = 0;
    for (size_t i = 0; i < x->count; i++) { size_t d = depth(x->el[i]); if (d > m) m = d; }
    return m + 1;
}

static HV *merge(HV *x) {
    if (x->k != HV_ARR || x->count == 0) return x;
    return merge_frame(x->rank, x->shape, x->count, x->el);
}

static HV *join_all(HV *x) {
    if (x->k != HV_ARR) hb_error("rank error: ∾ needs an array of arrays");
    if (x->count == 0) return hv_list(0);
    HV *acc = x->el[0];
    if (acc->k != HV_ARR) acc = as_list(acc);
    for (size_t i = 1; i < x->count; i++) acc = join2(acc, x->el[i]);
    return acc;
}

static size_t index_in(HV *w, HV *cell, size_t n) {
    for (size_t i = 0; i < n; i++) if (hv_match(major_cell(w, i), cell)) return i;
    return n;
}

/* mode 0: ⊐ classify, 1: ∊ mark firsts, 2: ⍷ deduplicate, 3: ⊒ occurrence count */
static HV *classify(HV *x, int mode) {
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: needs an array of rank ≥ 1");
    size_t n = x->shape[0];
    HV **uniq = hb_alloc((n ? n : 1) * sizeof(HV *));
    size_t nu = 0;
    HV *r = hv_list(n);
    for (size_t i = 0; i < n; i++) {
        HV *c = major_cell(x, i);
        size_t k = 0;
        while (k < nu && !hv_match(uniq[k], c)) k++;
        size_t before = 0;
        for (size_t j = 0; j < i; j++) if (hv_match(major_cell(x, j), c)) before++;
        if (k == nu) uniq[nu++] = c;
        if (mode == 0) r->el[i] = hv_num((double)k);
        else if (mode == 1) r->el[i] = hv_num(before == 0);
        else if (mode == 3) r->el[i] = hv_num((double)before);
    }
    if (mode == 2) return nu ? merge_list(nu, uniq) : hv_list(0);
    return r;
}

static HV *group(HV *w, HV *x) {
    HV *wl = as_list(w);
    size_t n = wl->count;
    if (x && hv_len(x) != n) hb_error("length error: ⊔ lengths differ");
    int64_t mx = -1;
    for (size_t i = 0; i < n; i++) { int64_t g = hv_getint(wl->el[i], "⊔"); if (g > mx) mx = g; }
    HV *r = hv_list((size_t)(mx + 1));
    for (int64_t g = 0; g <= mx; g++) {
        size_t cnt = 0;
        for (size_t i = 0; i < n; i++) if (hv_getint(wl->el[i], "⊔") == g) cnt++;
        HV **cells = hb_alloc((cnt ? cnt : 1) * sizeof(HV *));
        size_t q = 0;
        for (size_t i = 0; i < n; i++) if (hv_getint(wl->el[i], "⊔") == g) cells[q++] = x ? major_cell(x, i) : hv_num((double)i);
        r->el[g] = cnt ? merge_list(cnt, cells) : hv_list(0);
    }
    return r;
}

static char *string_of(HV *x) {
    x = as_list(x);
    char *buf = hb_alloc(x->count * 4 + 1);
    size_t b = 0;
    for (size_t i = 0; i < x->count; i++) {
        if (x->el[i]->k != HV_CHAR) return hb_repr(x);
        b += put_utf8(x->el[i]->c, buf + b);
    }
    buf[b] = 0;
    return buf;
}

/* ---------- bridge to the C11 engine ---------- */

HaoArray *hv_to_hao(const HV *v, const char *who) {
    if (v->k == HV_NUM) {
        HaoArray *a = hao_array_create(HAO_SCALAR_F64, 0, NULL);
        if (!a) hb_error("%s: allocation failed", who);
        ((double *)a->data)[0] = v->n;
        return a;
    }
    if (!all_numeric(v)) hb_error("domain error: %s needs a numeric array", who);
    HaoArray *a = hao_array_create(HAO_SCALAR_F64, v->rank, v->shape);
    if (!a) hb_error("%s: allocation failed", who);
    double *d = a->data;
    for (size_t i = 0; i < v->count; i++) d[i] = v->el[i]->n;
    return a;
}

HV *hao_to_hv(const HaoArray *a) {
    if (a->rank == 0) return hv_num(hao_get_f64(a, 0));
    HV *r = hv_arr(a->rank, a->shape);
    for (size_t i = 0; i < a->element_count; i++) r->el[i] = hv_num(hao_get_f64(a, i));
    return r;
}

static void engine_check(int rc, const char *who) {
    if (rc == 0) return;
    const char *kind = rc == -2 ? "length error" : rc == -3 ? "domain error" : rc == -4 ? "rank/index error" : "error";
    hb_error("%s: %s (engine code %d)", who, kind, rc);
}

static HV *wrap(HaoArray *o) {
    HV *r = hao_to_hv(o);
    hao_array_free(o);
    return r;
}

static HV *engine_tensor(HV *w, HV *x) {
    HaoArray *a = hv_to_hao(w, "⊗"), *b = hv_to_hao(x, "⊗"), *o = NULL;
    int rc = hao_outer(a, b, &o);
    hao_array_free(a);
    hao_array_free(b);
    engine_check(rc, "⊗");
    return wrap(o);
}

static HV *engine_geoprod(HV *w, HV *x) {
    HaoArray *a = hv_to_hao(w, "⊙"), *b = hv_to_hao(x, "⊙"), *o = NULL;
    if (a->element_count == 3 && b->element_count == 3) {
        int rc = hao_geometric_product3(a, b, &o);
        hao_array_free(a);
        hao_array_free(b);
        engine_check(rc, "⊙");
        return wrap(o);
    }
    if (a->element_count == HAO_MV_DIM && b->element_count == HAO_MV_DIM) {
        double out[HAO_MV_DIM];
        hao_mv_product(a->data, b->data, out);
        hao_array_free(a);
        hao_array_free(b);
        return num_list(HAO_MV_DIM, out);
    }
    hao_array_free(a);
    hao_array_free(b);
    hb_error("length error: ⊙ needs two 3-vectors or two 8-component multivectors");
    return NULL;
}

static HV *engine_ortho(HV *w, HV *x) {
    HaoArray *a = hv_to_hao(w, "⟂"), *b = hv_to_hao(x, "⟂");
    double d = hao_inner(a, b);
    hao_array_free(a);
    hao_array_free(b);
    if (d != d) hb_error("length error: ⟂ needs equal-length numeric vectors");
    return hv_num(fabs(d) < 1e-8);
}

static HV *engine_geodesic(HV *x, HV *y, double t, const char *who) {
    HaoArray *a = hv_to_hao(x, who), *b = hv_to_hao(y, who), *o = NULL;
    int rc = hao_geodesic(a, b, t, &o);
    hao_array_free(a);
    hao_array_free(b);
    engine_check(rc, who);
    return wrap(o);
}

/* ---------- primitive dispatch ---------- */

static bool monadic_arith(uint32_t cp) {
    switch (cp) {
        case '+': case '-': case 0xD7: case 0xF7: case 0x22C6: case 0x221A: case 0x230A:
        case 0x2308: case HP_HAO_CEIL: case '|': case 0x2223: case 0xAC:
            return true;
        default: return false;
    }
}

static HV *nudge(uint32_t cp, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: nudge needs an array");
    HV *f = fill_of(x);
    size_t sh[64];
    memcpy(sh, x->shape, x->rank * sizeof(size_t));
    sh[0] = 1;
    HV *one = hv_arr(x->rank, sh);
    for (size_t i = 0; i < one->count; i++) one->el[i] = f;
    int64_t n = (int64_t)hv_len(x);
    return (cp == 0xBB) ? take(n, join2(one, x)) : take(-n, join2(x, one));
}

HV *hb_prim_monad(uint32_t cp, HV *x) {
    if (monadic_arith(cp)) return perv1(cp, x);
    switch (cp) {
        case '<': return enclose0(x);
        case '>': return merge(x);
        case 0x2260: return hv_num((double)hv_len(x));
        case '=': return hv_num(x->k == HV_ARR ? (double)x->rank : 0.0);
        case 0x2261: return hv_num((double)depth(x));
        case 0x2262: {
            if (x->k != HV_ARR) return hv_list(0);
            HV *l = hv_list(x->rank);
            for (size_t i = 0; i < x->rank; i++) l->el[i] = hv_num((double)x->shape[i]);
            return l;
        }
        case 0x22A3: case 0x22A2: return x;
        case 0x294A: case 0x2361: return deshape(x);
        case 0x223E: return join_all(x);
        case 0x224D: {
            if (x->k != HV_ARR) { HV *l = hv_list(1); l->el[0] = x; return l; }
            size_t sh[64];
            if (x->rank > 63) hb_error("rank error");
            sh[0] = 1;
            memcpy(sh + 1, x->shape, x->rank * sizeof(size_t));
            HV *r = hv_arr(x->rank + 1, sh);
            memcpy(r->el, x->el, x->count * sizeof(HV *));
            return r;
        }
        case 0x22C8: { HV *l = hv_list(1); l->el[0] = x; return l; }
        case 0x2191: case 0x2193: {
            size_t n = hv_len(x);
            HV *r = hv_list(n + 1);
            for (size_t i = 0; i <= n; i++) r->el[i] = (cp == 0x2191) ? take((int64_t)i, x) : drop((int64_t)i, x);
            return r;
        }
        case 0x2195: return range(x);
        case 0x233D: case 0x21BB: return reverse(x);
        case 0x2349: return transpose_bqn(x, false);
        case '/': return indices(x);
        case 0x234B: return grade(x, false);
        case 0x2352: return grade(x, true);
        case 0x2227: return select_cells(grade(x, false), x);
        case 0x2228: return select_cells(grade(x, true), x);
        case 0x228F:
            if (x->k != HV_ARR || x->rank == 0 || x->shape[0] == 0) hb_error("length error: ⊏ of an empty array");
            return major_cell(x, 0);
        case 0x2291:
            if (x->k != HV_ARR) return x;
            if (x->count == 0) return fill_of(x);
            return x->el[0];
        case 0x2290: return classify(x, 0);
        case 0x220A: return classify(x, 1);
        case 0x2377: return classify(x, 2);
        case 0x2292: return classify(x, 3);
        case 0x2294: return group(x, NULL);
        case 0xAB: case 0xBB: return nudge(cp, x);
        case '!': {
            HV *v = (x->k == HV_ARR && x->rank == 0) ? x->el[0] : x;
            if (v->k == HV_NUM && v->n == 1.0) return x;
            hb_error("assertion error");
            return NULL;
        }
        case 0x2363: hb_error("%s", string_of(x)); return NULL;
        case 0x2297: case 0x2299: case 0x27C2: case 0x2301:
            hb_error("valence error: %s has no monadic form", glyph_str(cp));
            return NULL;
        default:
            hb_error("undefined primitive %s (not defined by BQN or the HAΩ glyph registry)", glyph_str(cp));
            return NULL;
    }
}

static HV *reorder_axes(HV *w, HV *x) {
    if (x->k != HV_ARR) return x;
    HV *wl = as_list(w);
    size_t r = x->rank, perm[64];
    bool seen[64];
    if (r > 64) hb_error("rank error");
    memset(seen, 0, sizeof(seen));
    if (wl->count != r) hb_error("length error: ⍉ needs one axis per axis of 𝕩 (diagonals not supported)");
    for (size_t i = 0; i < r; i++) {
        int64_t a = hv_getint(wl->el[i], "⍉");
        if (a < 0 || (size_t)a >= r || seen[a]) hb_error("domain error: ⍉ needs a permutation");
        seen[a] = true;
        perm[(size_t)a] = i;
    }
    if (all_numeric(x)) {
        HaoArray *a = hv_to_hao(x, "⍉"), *o = NULL;
        int rc = hao_transpose_axes(a, perm, &o);
        hao_array_free(a);
        engine_check(rc, "⍉");
        return wrap(o);
    }
    return reorder(x, perm);
}

static HV *bins(uint32_t cp, HV *w, HV *x) {
    HV *wl = as_list(w);
    HV *xs = (x->k == HV_ARR) ? x : enclose0(x);
    HV *r = hv_arr(xs->rank, xs->shape);
    for (size_t i = 0; i < xs->count; i++) {
        size_t c = 0;
        for (size_t j = 0; j < wl->count; j++) {
            int k = cmp_val(wl->el[j], xs->el[i]);
            if ((cp == 0x234B) ? (k <= 0) : (k >= 0)) c++;
        }
        r->el[i] = hv_num((double)c);
    }
    return (x->k == HV_ARR) ? r : r->el[0];
}

static HV *index_of(HV *w, HV *x) {
    if (w->k != HV_ARR || w->rank == 0) hb_error("rank error: ⊐ needs a list on the left");
    size_t n = w->shape[0];
    HV *xs = (x->k == HV_ARR) ? x : enclose0(x);
    if (w->rank == 1) {
        HV *r = hv_arr(xs->rank, xs->shape);
        for (size_t i = 0; i < xs->count; i++) r->el[i] = hv_num((double)index_in(w, xs->el[i], n));
        return (x->k == HV_ARR) ? r : r->el[0];
    }
    size_t m = hv_len(x);
    HV *r = hv_list(m);
    for (size_t i = 0; i < m; i++) r->el[i] = hv_num((double)index_in(w, major_cell(x, i), n));
    return r;
}

static HV *member_of(HV *w, HV *x) {
    HV *ws = (w->k == HV_ARR) ? w : enclose0(w);
    HV *xs = (x->k == HV_ARR) ? x : enclose0(x);
    HV *r = hv_arr(ws->rank, ws->shape);
    for (size_t i = 0; i < ws->count; i++) {
        bool found = false;
        for (size_t j = 0; j < xs->count && !found; j++) found = hv_match(ws->el[i], xs->el[j]);
        r->el[i] = hv_num(found);
    }
    return (w->k == HV_ARR) ? r : r->el[0];
}

/* 𝕨⍷𝕩 Find for lists: 1 where the list 𝕨 starts a matching run in 𝕩 */
static HV *find_list(HV *w, HV *x) {
    if (w->k != HV_ARR || x->k != HV_ARR || w->rank != 1 || x->rank != 1)
        hb_error("rank error: dyadic ⍷ is implemented for lists only");
    size_t m = w->count, n = x->count;
    size_t len = (m > n) ? 0 : n - m + 1;
    HV *r = hv_list(len);
    for (size_t i = 0; i < len; i++) {
        bool ok = true;
        for (size_t j = 0; j < m && ok; j++) ok = hv_match(w->el[j], x->el[i + j]);
        r->el[i] = hv_num(ok);
    }
    return r;
}

static HV *windows(HV *w, HV *x) {
    int64_t k = hv_getint(w, "↕");
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: ↕ windows need an array");
    int64_t n = (int64_t)x->shape[0];
    if (k < 0 || k > n + 1) hb_error("length error: window too long");
    size_t m = (size_t)(n - k + 1);
    if (m == 0) return hv_list(0);
    HV **cells = hb_alloc(m * sizeof(HV *));
    for (size_t i = 0; i < m; i++) cells[i] = take(k, drop((int64_t)i, x));
    return merge_list(m, cells);
}

static HV *couple(HV *w, HV *x) {
    if (w->k == HV_ARR || x->k == HV_ARR) {
        if (w->k != HV_ARR || x->k != HV_ARR || w->rank != x->rank) hb_error("rank error: ≍ needs equal shapes");
        for (size_t i = 0; i < w->rank; i++) if (w->shape[i] != x->shape[i]) hb_error("length error: ≍ needs equal shapes");
    }
    HV *c[2];
    c[0] = w;
    c[1] = x;
    return merge_list(2, c);
}

HV *hb_prim_dyad(uint32_t cp, HV *w, HV *x) {
    if (cp == HP_HAO_CEIL) return join2(w, x);
    if (is_arith(cp)) return perv2(cp, w, x);
    switch (cp) {
        case 0x2261: return hv_num(hv_match(w, x));
        case 0x2262: return hv_num(!hv_match(w, x));
        case 0x22A3: return w;
        case 0x22A2: return x;
        case 0x294A: return reshape_cycle(w, x, false);
        case 0x2361: return reshape_cycle(w, x, true);
        case 0x223E: return join2(w, x);
        case 0x224D: return couple(w, x);
        case 0x22C8: { HV *l = hv_list(2); l->el[0] = w; l->el[1] = x; return l; }
        case 0x2191: return take(hv_getint(w, "↑"), x);
        case 0x2193: return drop(hv_getint(w, "↓"), x);
        case 0x2195: return windows(w, x);
        case 0x233D: case 0x21BB:
            if (w->k == HV_ARR && w->rank == 1 && w->count == 1) w = w->el[0];
            return rotate(hv_getint(w, "⌽"), x);
        case 0x2349: return reorder_axes(w, x);
        case '/': return replicate(w, x);
        case 0x234B: case 0x2352: return bins(cp, w, x);
        case 0x228F: return select_cells(w, x);
        case 0x2291: return pick(w, x);
        case 0x2290: return index_of(w, x);
        case 0x220A: return member_of(w, x);
        case 0x2377: return find_list(w, x);
        case 0x2294: return group(w, x);
        case 0xBB: case 0xAB: {
            if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: shift needs an array");
            int64_t n = (int64_t)hv_len(x);
            return (cp == 0xBB) ? take(n, join2(w, x)) : take(-n, join2(x, w));
        }
        case '!': {
            HV *v = (x->k == HV_ARR && x->rank == 0) ? x->el[0] : x;
            if (v->k == HV_NUM && v->n == 1.0) return x;
            hb_error("%s", string_of(w));
            return NULL;
        }
        case 0x2297: return engine_tensor(w, x);
        case 0x2299: return engine_geoprod(w, x);
        case 0x27C2: return engine_ortho(w, x);
        case 0x2301: return engine_geodesic(w, x, 0.5, "⌁");
        case 0x2363: hb_error("%s", string_of(x)); return NULL;
        default:
            hb_error("undefined primitive %s (not defined by BQN or the HAΩ glyph registry)", glyph_str(cp));
            return NULL;
    }
}

/* ---------- modifiers ---------- */

static HV *identity_of(HV *f) {
    if (f->k != HV_FN || f->fk != FK_PRIM) return NULL;
    switch (f->prim) {
        case '+': case '-': case 0x2228: case 0x2260: case '>': case '<': return hv_num(0);
        case 0xD7: case 0xF7: case 0x2227: case '=': case 0x2264: case 0x2265: case 0x22C6: return hv_num(1);
        case 0x230A: return hv_num(hb_infinity());
        case 0x2308: return hv_num(-hb_infinity());
        default: return NULL;
    }
}

static HV *log_each(HV *x) {
    if (x->k != HV_ARR) return hv_num(log(hv_getnum(x, "⋆⁼")));
    HV *r = hv_arr(x->rank, x->shape);
    for (size_t i = 0; i < x->count; i++) r->el[i] = log_each(x->el[i]);
    return r;
}

static HV *undo(HV *f, size_t nargs, HV **args) {
    HV *x = args[nargs - 1];
    if (f->k == HV_FN && f->fk == FK_PRIM) {
        switch (f->prim) {
            case '+': return nargs == 1 ? x : perv2('-', x, args[0]);
            case '-': return nargs == 1 ? perv1('-', x) : perv2('-', args[0], x);
            case 0xD7: if (nargs == 2) return perv2(0xF7, x, args[0]); break;
            case 0xF7: return nargs == 1 ? perv1(0xF7, x) : perv2(0xF7, args[0], x);
            case 0x22C6:
                if (nargs == 1) return log_each(x);
                return perv2(0xF7, log_each(x), log_each(args[0]));
            case 0x221A: return nargs == 1 ? perv2(0xD7, x, x) : perv2(0x22C6, x, args[0]);
            case 0xAC: if (nargs == 1) return perv1(0xAC, x); break;
            case 0x22A2: case 0x22A3: return x;
            case 0x233D: return nargs == 1 ? reverse(x) : rotate(-hv_getint(args[0], "⌽⁼"), x);
            case 0x2349: if (nargs == 1) return transpose_bqn(x, true); break;
            default: break;
        }
    }
    hb_error("undo (⁼) is not supported for this function");
    return NULL;
}

/* Structural Under: 𝔾 only moves elements around (selection, reshaping,
   reordering), so it is applied to an array of indices to learn which
   element of 𝕩 ends up where, and 𝔽's result is written back there. */
static bool is_structural(const HV *g) {
    if (g->k != HV_FN) return false;
    switch (g->fk) {
        case FK_PRIM:
            switch (g->prim) {
                case 0x2291: case 0x228F: case 0x2191: case 0x2193: case 0x233D: case 0x2349:
                case '/': case 0x294A: case 0x22A2: case 0x22A3: case 0x21BB: case 0x2361:
                case 0x2282: case 0x2283:
                    return true;
                default: return false;
            }
        case FK_D1:
            return (g->prim == 0xA8 || g->prim == 0x2D8) && is_structural(g->f);
        case FK_D2:
            if (g->prim == 0x22B8) return !hv_is_fn(g->f) && is_structural(g->g);
            if (g->prim == 0x27DC) return is_structural(g->f) && !hv_is_fn(g->g);
            if (g->prim == 0x2218) return is_structural(g->f) && is_structural(g->g);
            return false;
        case FK_ATOP:
            return is_structural(g->g) && is_structural(g->h);
        case FK_FORK:
            return (!hv_is_fn(g->f) || is_structural(g->f)) && is_structural(g->g) && is_structural(g->h);
        default:
            return false;
    }
}

static void under_put(HV *res, const HV *pos, HV *val) {
    if (pos->k == HV_NUM) {
        size_t i = (size_t)pos->n;
        if (pos->n < 0 || (double)i != pos->n || i >= res->count) hb_error("domain error: ⌾ position out of range");
        res->el[i] = val;
        return;
    }
    if (pos->k != HV_ARR) hb_error("domain error: ⌾ structural function produced a non-element");
    if (val->k != HV_ARR || val->rank != pos->rank) hb_error("length error: ⌾ 𝔽 changed the shape of 𝔾 𝕩");
    for (size_t d = 0; d < pos->rank; d++)
        if (val->shape[d] != pos->shape[d]) hb_error("length error: ⌾ 𝔽 changed the shape of 𝔾 𝕩");
    for (size_t k = 0; k < pos->count; k++) under_put(res, pos->el[k], val->el[k]);
}

static HV *under_structural(HV *f, HV *g, HV *w, HV *x) {
    HV *idx = hv_arr(x->rank, x->shape);
    for (size_t i = 0; i < x->count; i++) idx->el[i] = hv_num((double)i);
    HV *pos = hv_call1(g, idx);
    HV *gx = hv_call1(g, x);
    HV *fv = w ? hv_call2(f, hv_call1(g, w), gx) : hv_call1(f, gx);
    HV *res = hv_arr(x->rank, x->shape);
    for (size_t i = 0; i < x->count; i++) res->el[i] = x->el[i];
    under_put(res, pos, fv);
    return res;
}

static HV *each_pair(void *ctx, HV *a, HV *b) { return hv_call2((HV *)ctx, a, b); }

/* Apply f to cells of rank kx (and kw); frames must agree on their common prefix. */
static HV *cells_apply(HV *f, size_t nargs, HV **args, size_t kw, size_t kx) {
    HV *x = args[nargs - 1];
    HV *w = nargs == 2 ? args[0] : NULL;
    size_t xr = x->k == HV_ARR ? x->rank : 0, wr = (w && w->k == HV_ARR) ? w->rank : 0;
    if (kx > xr) kx = xr;
    if (kw > wr) kw = wr;
    size_t xf = xr - kx, wf = w ? wr - kw : 0;
    size_t frank = xf > wf ? xf : wf;
    const size_t *fshape = (xf >= wf) ? (x->k == HV_ARR ? x->shape : NULL) : w->shape;
    if (w && xf && wf) {
        size_t m = xf < wf ? xf : wf;
        for (size_t i = 0; i < m; i++) if (x->shape[i] != w->shape[i]) hb_error("length error: frames do not agree");
    }
    size_t nf = 1, xfc = 1, wfc = 1;
    for (size_t i = 0; i < frank; i++) nf *= fshape[i];
    for (size_t i = 0; i < xf; i++) xfc *= x->shape[i];
    for (size_t i = 0; i < wf; i++) wfc *= w->shape[i];
    HV **res = hb_alloc((nf ? nf : 1) * sizeof(HV *));
    for (size_t c = 0; c < nf; c++) {
        HV *xc = x, *wc = w;
        if (xf) {
            size_t xi = c / (nf / xfc);
            xc = hv_arr(kx, x->shape + xf);
            memcpy(xc->el, x->el + xi * xc->count, xc->count * sizeof(HV *));
            if (kx == 0) xc = xc->el[0];
        }
        if (w && wf) {
            size_t wi = c / (nf / wfc);
            wc = hv_arr(kw, w->shape + wf);
            memcpy(wc->el, w->el + wi * wc->count, wc->count * sizeof(HV *));
            if (kw == 0) wc = wc->el[0];
        }
        res[c] = w ? hv_call2(f, wc, xc) : hv_call1(f, xc);
    }
    if (frank == 0) return res[0];
    if (nf == 0) return hv_arr(frank, fshape);
    return merge_frame(frank, fshape, nf, res);
}

static HV *fold(HV *f, HV *w, HV *x) {
    if (x->k != HV_ARR || x->rank != 1) hb_error("rank error: ´ needs a list");
    size_t n = x->count;
    HV *acc;
    if (w) acc = w;
    else {
        if (n == 0) {
            HV *id = identity_of(f);
            if (!id) hb_error("domain error: fold of an empty list has no identity");
            return id;
        }
        acc = x->el[--n];
    }
    while (n-- > 0) acc = hv_call2(f, x->el[n], acc);
    return acc;
}

static HV *insert(HV *f, HV *w, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: ˝ needs an array of rank ≥ 1");
    size_t n = x->shape[0];
    HV *acc;
    if (w) acc = w;
    else {
        if (n == 0) hb_error("domain error: insert over an empty array");
        acc = major_cell(x, --n);
    }
    while (n-- > 0) acc = hv_call2(f, major_cell(x, n), acc);
    if (x->rank == 1 && acc->k != HV_ARR) return enclose0(acc);
    return acc;
}

static HV *scan(HV *f, HV *w, HV *x) {
    if (x->k != HV_ARR || x->rank == 0) hb_error("rank error: ` needs an array of rank ≥ 1");
    size_t n = x->shape[0];
    if (n == 0) return x;
    HV **cells = hb_alloc(n * sizeof(HV *));
    HV *acc = w ? hv_call2(f, w, major_cell(x, 0)) : major_cell(x, 0);
    cells[0] = acc;
    for (size_t i = 1; i < n; i++) { acc = hv_call2(f, acc, major_cell(x, i)); cells[i] = acc; }
    return merge_list(n, cells);
}

static HV *table(HV *f, HV *w, HV *x) {
    HV *ws = (w->k == HV_ARR) ? w : enclose0(w);
    HV *xs = (x->k == HV_ARR) ? x : enclose0(x);
    size_t sh[64];
    if (ws->rank + xs->rank > 64) hb_error("rank error");
    if (ws->rank) memcpy(sh, ws->shape, ws->rank * sizeof(size_t));
    if (xs->rank) memcpy(sh + ws->rank, xs->shape, xs->rank * sizeof(size_t));
    HV *r = hv_arr(ws->rank + xs->rank, sh);
    for (size_t i = 0; i < ws->count; i++)
        for (size_t j = 0; j < xs->count; j++) r->el[i * xs->count + j] = hv_call2(f, ws->el[i], xs->el[j]);
    return r;
}

static HV *each(HV *f, HV *w, HV *x) {
    if (!w) {
        if (x->k != HV_ARR) return enclose0(hv_call1(f, x));
        HV *r = hv_arr(x->rank, x->shape);
        for (size_t i = 0; i < x->count; i++) r->el[i] = hv_call1(f, x->el[i]);
        return r;
    }
    if (w->k != HV_ARR && x->k != HV_ARR) return enclose0(hv_call2(f, w, x));
    return pairwise(w, x, each_pair, f, false);
}

HV *hb_apply_d1(HV *d, size_t nargs, HV **args) {
    HV *f = d->f;
    HV *x = args[nargs - 1];
    HV *w = nargs == 2 ? args[0] : NULL;
    switch (d->prim) {
        case 0x2D9: return f;
        case 0x2DC: return w ? hv_call2(f, x, w) : hv_call2(f, x, x);
        case 0x2D8: {
            /* HAΩ dialect: monadic F˘ x is x F x (Norm ← +´×˘); BQN dialect: Cells */
            if (!w && !hb_bqn_mode) return hv_call2(f, x, x);
            size_t kx = (x->k == HV_ARR && x->rank) ? x->rank - 1 : 0;
            size_t kw = (w && w->k == HV_ARR && w->rank) ? w->rank - 1 : 0;
            return cells_apply(f, nargs, args, kw, kx);
        }
        case 0xA8: return each(f, w, x);
        case 0x231C: return w ? table(f, w, x) : each(f, NULL, x);
        case 0xB4: return fold(f, w, x);
        case 0x2DD: return insert(f, w, x);
        case '`': return scan(f, w, x);
        case 0x207C: return undo(f, nargs, args);
        default:
            hb_error("unsupported 1-modifier %s", glyph_str(d->prim));
            return NULL;
    }
}

static size_t rank_arg(HV *g, size_t which, size_t r) {
    /* which: 0 monadic, 1 left, 2 right; BQN ⎉ operand forms: k, w‿x, m‿w‿x */
    HV *gl = as_list(g);
    int64_t k;
    if (gl->count == 1) k = hv_getint(gl->el[0], "⎉");
    else if (gl->count == 2) k = hv_getint(gl->el[which == 1 ? 0 : 1], "⎉");
    else if (gl->count == 3) k = hv_getint(gl->el[which], "⎉");
    else { hb_error("length error: ⎉ rank list must have 1 to 3 numbers"); return 0; }
    if (k < 0) k = (int64_t)r + k;
    if (k < 0) k = 0;
    return (size_t)k;
}

static HV *catch_call(HV *f, HV *g, size_t nargs, HV **args) {
    HbHandler h;
    h.prev = hb_handler;
    hb_handler = &h;
    if (setjmp(h.jb) == 0) {
        HV *res = hv_call(f, nargs, args);
        hb_handler = h.prev;
        return res;
    }
    hb_handler = h.prev;
    return hv_call(g, nargs, args);
}

HV *hb_apply_d2(HV *d, size_t nargs, HV **args) {
    HV *f = d->f, *g = d->g;
    HV *x = args[nargs - 1];
    HV *w = nargs == 2 ? args[0] : NULL;
    switch (d->prim) {
        case 0x2218: return hv_call1(f, hv_call(g, nargs, args));
        case 0x25CB: {
            if (!w) return hv_call1(f, hv_call1(g, x));
            HV *gx = hv_call1(g, x);
            HV *gw = hv_call1(g, w);
            return hv_call2(f, gw, gx);
        }
        case 0x22B8: {
            HV *left = hv_is_fn(f) ? hv_call1(f, w ? w : x) : f;
            return hv_call2(g, left, x);
        }
        case 0x27DC: {
            HV *right = hv_is_fn(g) ? hv_call1(g, x) : g;
            return hv_call2(f, w ? w : x, right);
        }
        case 0x2298: return w ? hv_call(g, nargs, args) : hv_call(f, nargs, args);
        case 0x25F6: {
            HV *iv = hv_is_fn(f) ? hv_call(f, nargs, args) : f;
            int64_t i = hv_getint(iv, "◶");
            HV *gl = as_list(g);
            if (i < 0) i += (int64_t)gl->count;
            if (i < 0 || (size_t)i >= gl->count) hb_error("index error: ◶ index out of range");
            return hv_call(gl->el[i], nargs, args);
        }
        case 0x2389: case 0x2364: {
            if (hv_is_fn(g)) hb_error("⎉ with a function right operand is not supported");
            size_t xr = x->k == HV_ARR ? x->rank : 0, wr = (w && w->k == HV_ARR) ? w->rank : 0;
            size_t kx = rank_arg(g, w ? 2 : 0, xr);
            size_t kw = w ? rank_arg(g, 1, wr) : 0;
            return cells_apply(f, nargs, args, kw, kx);
        }
        case 0x235F: {
            HV *nv = hv_is_fn(g) ? hv_call(g, nargs, args) : g;
            int64_t n = hv_getint(nv, "⍟");
            if (n < 0) hb_error("domain error: ⍟ with a negative count is not supported");
            HV *cur = x;
            for (int64_t i = 0; i < n; i++) cur = w ? hv_call2(f, w, cur) : hv_call1(f, cur);
            return cur;
        }
        case 0x238A: return catch_call(f, g, nargs, args);
        case 0x233E: {
            if (x->k == HV_ARR && is_structural(g)) return under_structural(f, g, w, x);
            HV *a1[1];
            HV *gx = hv_call1(g, x);
            a1[0] = w ? hv_call2(f, hv_call1(g, w), gx) : hv_call1(f, gx);
            return undo(g, 1, a1);
        }
        default:
            hb_error("unsupported 2-modifier %s", glyph_str(d->prim));
            return NULL;
    }
}

/* ---------- formatting ---------- */

static void put(char **buf, size_t *len, size_t *cap, const char *s) {
    size_t n = strlen(s);
    if (*len + n + 1 > *cap) {
        size_t nc = (*cap ? *cap : 64);
        while (*len + n + 1 > nc) nc *= 2;
        char *nb = hb_alloc(nc);
        if (*len) memcpy(nb, *buf, *len);
        *buf = nb;
        *cap = nc;
    }
    memcpy(*buf + *len, s, n);
    *len += n;
    (*buf)[*len] = 0;
}

/* BQN number notation: ¯ for minus, e¯ for negative exponents, ∞, NaN. */
static void fmt_num(double d, char *out, size_t outsz) {
    if (d != d) { snprintf(out, outsz, "NaN"); return; }
    if (d > DBL_MAX) { snprintf(out, outsz, "∞"); return; }
    if (d < -DBL_MAX) { snprintf(out, outsz, "¯∞"); return; }
    char tmp[64];
    if (d == floor(d) && fabs(d) < 1e15) snprintf(tmp, sizeof(tmp), "%.0f", d);
    else {
        for (int prec = 15; prec <= 17; prec++) {
            snprintf(tmp, sizeof(tmp), "%.*g", prec, d);
            if (strtod(tmp, NULL) == d) break;
        }
    }
    if (strcmp(tmp, "-0") == 0) { snprintf(out, outsz, "0"); return; }
    size_t o = 0;
    for (size_t i = 0; tmp[i] && o + 4 < outsz; i++) {
        char ch = tmp[i];
        if (ch == '-') { out[o++] = (char)0xC2; out[o++] = (char)0xAF; }
        else if (ch == '+') continue;
        else if (ch == 'e') {
            out[o++] = 'e';
            if (tmp[i + 1] == '-') { out[o++] = (char)0xC2; out[o++] = (char)0xAF; i++; }
            else if (tmp[i + 1] == '+') i++;
            while (tmp[i + 1] == '0' && tmp[i + 2]) i++;
        }
        else out[o++] = ch;
    }
    out[o] = 0;
}

void hb_format(const HV *v, char **buf, size_t *len, size_t *cap) {
    char tmp[64];
    switch (v->k) {
        case HV_NUM: fmt_num(v->n, tmp, sizeof(tmp)); put(buf, len, cap, tmp); return;
        case HV_CHAR:
            put(buf, len, cap, "'");
            put(buf, len, cap, glyph_str(v->c));
            put(buf, len, cap, "'");
            return;
        case HV_FN:
            if (v->fk == FK_PRIM) { put(buf, len, cap, glyph_str(v->prim)); return; }
            if (v->fk == FK_BUILTIN && v->name) { put(buf, len, cap, v->name); return; }
            put(buf, len, cap, "(function)");
            return;
        case HV_M1: put(buf, len, cap, glyph_str(v->prim)); return;
        case HV_M2: put(buf, len, cap, glyph_str(v->prim)); return;
        case HV_ARR: break;
    }
    if (v->rank == 0) { put(buf, len, cap, "<"); hb_format(v->el[0], buf, len, cap); return; }
    if (v->rank == 1) {
        bool str = v->count > 0;
        for (size_t i = 0; i < v->count; i++) if (v->el[i]->k != HV_CHAR) str = false;
        if (str) {
            put(buf, len, cap, "\"");
            for (size_t i = 0; i < v->count; i++) put(buf, len, cap, v->el[i]->c == '"' ? "\"\"" : glyph_str(v->el[i]->c));
            put(buf, len, cap, "\"");
            return;
        }
        if (v->count == 0) { put(buf, len, cap, "⟨⟩"); return; }
        put(buf, len, cap, "⟨ ");
        for (size_t i = 0; i < v->count; i++) {
            if (i) put(buf, len, cap, " ");
            hb_format(v->el[i], buf, len, cap);
        }
        put(buf, len, cap, " ⟩");
        return;
    }
    for (size_t i = 0; i < v->rank; i++) {
        if (i) put(buf, len, cap, "‿");
        snprintf(tmp, sizeof(tmp), "%d", (int)v->shape[i]);
        put(buf, len, cap, tmp);
    }
    put(buf, len, cap, "⥊");
    if (v->count == 0) { put(buf, len, cap, "⟨⟩"); return; }
    HV flat = *v;
    size_t sh[1];
    sh[0] = v->count;
    flat.rank = 1;
    flat.shape = sh;
    hb_format(&flat, buf, len, cap);
}

char *hb_repr(const HV *v) {
    char *buf = NULL;
    size_t len = 0, cap = 0;
    put(&buf, &len, &cap, "");
    hb_format(v, &buf, &len, &cap);
    return buf;
}

/* •Repr: BQN source form, as CBQN prints it (1‿2‿3, ⟨1‿2,"x"⟩, (2‿3⥊0‿1‿2‿3‿4‿5), (<1)) */
static void hb_source(const HV *v, char **buf, size_t *len, size_t *cap);

static bool is_string(const HV *v) {
    if (v->k != HV_ARR || v->rank != 1 || v->count == 0) return false;
    for (size_t i = 0; i < v->count; i++) if (v->el[i]->k != HV_CHAR) return false;
    return true;
}

static void hb_source_list(const HV *v, char **buf, size_t *len, size_t *cap) {
    if (v->count == 0) { put(buf, len, cap, "⟨⟩"); return; }
    if (is_string(v)) { hb_format(v, buf, len, cap); return; }
    bool strand = v->count >= 2;
    for (size_t i = 0; i < v->count; i++)
        if (v->el[i]->k == HV_ARR || v->el[i]->k == HV_M1 || v->el[i]->k == HV_M2) strand = false;
    if (strand) {
        for (size_t i = 0; i < v->count; i++) {
            if (i) put(buf, len, cap, "‿");
            hb_source(v->el[i], buf, len, cap);
        }
        return;
    }
    put(buf, len, cap, "⟨");
    for (size_t i = 0; i < v->count; i++) {
        if (i) put(buf, len, cap, ",");
        hb_source(v->el[i], buf, len, cap);
    }
    put(buf, len, cap, "⟩");
}

static void hb_source(const HV *v, char **buf, size_t *len, size_t *cap) {
    char tmp[32];
    if (v->k != HV_ARR) { hb_format(v, buf, len, cap); return; }
    if (v->rank == 0) {
        put(buf, len, cap, "(<");
        hb_source(v->el[0], buf, len, cap);
        put(buf, len, cap, ")");
        return;
    }
    if (v->rank == 1) { hb_source_list(v, buf, len, cap); return; }
    put(buf, len, cap, "(");
    for (size_t i = 0; i < v->rank; i++) {
        if (i) put(buf, len, cap, "‿");
        snprintf(tmp, sizeof(tmp), "%d", (int)v->shape[i]);
        put(buf, len, cap, tmp);
    }
    put(buf, len, cap, "⥊");
    HV flat = *v;
    size_t sh[1];
    sh[0] = v->count;
    flat.rank = 1;
    flat.shape = sh;
    hb_source_list(&flat, buf, len, cap);
    put(buf, len, cap, ")");
}

static char *hb_source_repr(const HV *v) {
    char *buf = NULL;
    size_t len = 0, cap = 0;
    put(&buf, &len, &cap, "");
    hb_source(v, &buf, &len, &cap);
    return buf;
}

/* ---------- builtins ---------- */

static void need_args(size_t nargs, size_t lo, size_t hi, const char *who) {
    if (nargs < lo || nargs > hi) hb_error("valence error: %s takes %d to %d arguments, got %d", who, (int)lo, (int)hi, (int)nargs);
}

typedef double (*Math1)(double);

static HV *map_math(Math1 fn, HV *x, const char *who) {
    if (x->k == HV_ARR) {
        HV *r = hv_arr(x->rank, x->shape);
        for (size_t i = 0; i < x->count; i++) r->el[i] = map_math(fn, x->el[i], who);
        return r;
    }
    return hv_num(fn(hv_getnum(x, who)));
}

#define MATH_BUILTIN(NAME, FN) \
    static HV *bi_##NAME(size_t nargs, HV **args) { need_args(nargs, 1, 1, #NAME); return map_math(FN, args[0], #NAME); }

MATH_BUILTIN(Sin, sin)
MATH_BUILTIN(Cos, cos)
MATH_BUILTIN(Tan, tan)
MATH_BUILTIN(Arcsin, asin)
MATH_BUILTIN(Arccos, acos)
MATH_BUILTIN(Arctan, atan)
MATH_BUILTIN(Sinh, sinh)
MATH_BUILTIN(Cosh, cosh)
MATH_BUILTIN(Tanh, tanh)
MATH_BUILTIN(Exp, exp)
MATH_BUILTIN(Log, log)
MATH_BUILTIN(Sqrt, sqrt)
MATH_BUILTIN(Abs, fabs)

static HV *bi_Atan2(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "Atan2");
    return hv_num(atan2(hv_getnum(args[0], "Atan2"), hv_getnum(args[1], "Atan2")));
}

/* Quaternion conjugate (w x y z) -> (w -x -y -z); real numbers are self-conjugate. */
static HV *bi_Conj(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "Conj");
    HV *x = args[0];
    if (x->k == HV_NUM) return x;
    if (!all_numeric(x) || x->rank != 1 || x->count != 4) hb_error("domain error: Conj needs a number or a 4-component quaternion");
    HV *r = hv_list(4);
    r->el[0] = x->el[0];
    for (size_t i = 1; i < 4; i++) r->el[i] = hv_num(-x->el[i]->n);
    return r;
}

static HV *bi_Show(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•Show");
    printf("%s\n", hb_repr(args[0]));
    return args[0];
}

static HV *bi_Out(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•Out");
    printf("%s\n", string_of(args[0]));
    return args[0];
}

static HV *bi_Repr(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•Repr");
    return hv_str(hb_source_repr(args[0]));
}

static HV *bi_Fmt(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•Fmt");
    return hv_str(hb_repr(args[0]));
}

/* •CurrentError: message of the error caught by the innermost ⎊ handler */
static HV *bi_CurrentError(size_t nargs, HV **args) {
    (void)nargs; (void)args;
    return hv_str(hb_errtext);
}

static HV *bi_Type(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•Type");
    switch (args[0]->k) {
        case HV_ARR: return hv_num(0);
        case HV_NUM: return hv_num(1);
        case HV_CHAR: return hv_num(2);
        case HV_FN: return hv_num(3);
        case HV_M1: return hv_num(4);
        default: return hv_num(5);
    }
}

/* ----- engine-backed system functions: •hao.Name ----- */

static HV *bi_hao_Norm(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•hao.Norm");
    HaoArray *a = hv_to_hao(args[0], "•hao.Norm");
    HaoArray *n = hao_norm(a);
    hao_array_free(a);
    if (!n) hb_error("•hao.Norm failed");
    double v = ((double *)n->data)[0];
    hao_array_free(n);
    return hv_num(v);
}

static HV *bi_hao_Inner(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Inner");
    HaoArray *a = hv_to_hao(args[0], "•hao.Inner"), *b = hv_to_hao(args[1], "•hao.Inner");
    double d = hao_inner(a, b);
    hao_array_free(a);
    hao_array_free(b);
    if (d != d) hb_error("length error: •hao.Inner needs equal element counts");
    return hv_num(d);
}

static HV *bi_hao_Normalize(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•hao.Normalize");
    HaoArray *a = hv_to_hao(args[0], "•hao.Normalize");
    HaoArray *o = hao_normalize(a);
    hao_array_free(a);
    if (!o) hb_error("domain error: •hao.Normalize of a zero vector");
    return wrap(o);
}

static HV *bi_hao_SphereExp(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.SphereExp");
    HaoArray *x = hv_to_hao(args[0], "•hao.SphereExp"), *v = hv_to_hao(args[1], "•hao.SphereExp");
    if (x->rank == 0) { hao_array_free(x); hao_array_free(v); hb_error("rank error: •hao.SphereExp needs vectors"); }
    HaoArray *o = hao_array_create(HAO_SCALAR_F64, x->rank, x->shape);
    int rc = o ? hao_array_sphere_exp(x, v, o) : -1;
    hao_array_free(x);
    hao_array_free(v);
    if (rc) {
        hao_array_free(o);
        hb_error("•hao.SphereExp: %s (engine code %d)", rc == -3 ? "x is not a unit vector" : rc == -4 ? "v is not tangent at x" : "shape error", rc);
    }
    return wrap(o);
}

static HV *bi_hao_Evolve(size_t nargs, HV **args) {
    need_args(nargs, 4, 4, "•hao.Evolve");
    double eta = hv_getnum(args[2], "•hao.Evolve");
    int64_t steps = hv_getint(args[3], "•hao.Evolve");
    if (steps < 0) hb_error("domain error: negative step count");
    HaoArray *x = hv_to_hao(args[0], "•hao.Evolve"), *v = hv_to_hao(args[1], "•hao.Evolve"), *o = NULL;
    int rc = hao_evolve_sphere(x, v, eta, (size_t)steps, &o);
    hao_array_free(x);
    hao_array_free(v);
    if (rc) hb_error("•hao.Evolve: %s (engine code %d)", rc == -3 ? "preconditions |x|=1 and x·v=0 violated" : "shape error", rc);
    return wrap(o);
}

static HV *bi_hao_Geodesic(size_t nargs, HV **args) {
    need_args(nargs, 3, 3, "•hao.Geodesic");
    return engine_geodesic(args[0], args[1], hv_getnum(args[2], "•hao.Geodesic"), "•hao.Geodesic");
}

static HV *bi_hao_GeoProd(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.GeoProd");
    return engine_geoprod(args[0], args[1]);
}

static HV *bi_hao_Wedge(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Wedge");
    HaoArray *a = hv_to_hao(args[0], "•hao.Wedge"), *b = hv_to_hao(args[1], "•hao.Wedge");
    if (a->element_count != 3 || b->element_count != 3) {
        hao_array_free(a);
        hao_array_free(b);
        hb_error("length error: •hao.Wedge needs 3-vectors");
    }
    double out[3];
    hao_wedge3(a->data, b->data, out);
    hao_array_free(a);
    hao_array_free(b);
    return num_list(3, out);
}

static void mv_arg(HV *v, double *out, const char *who) {
    HaoArray *a = hv_to_hao(v, who);
    if (a->element_count != HAO_MV_DIM) { hao_array_free(a); hb_error("length error: %s needs 8-component multivectors", who); }
    memcpy(out, a->data, HAO_MV_DIM * sizeof(double));
    hao_array_free(a);
}

static HV *bi_hao_MvOuter(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.MvOuter");
    double a[HAO_MV_DIM], b[HAO_MV_DIM], o[HAO_MV_DIM];
    mv_arg(args[0], a, "•hao.MvOuter");
    mv_arg(args[1], b, "•hao.MvOuter");
    hao_mv_outer(a, b, o);
    return num_list(HAO_MV_DIM, o);
}

static HV *bi_hao_MvContract(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.MvContract");
    double a[HAO_MV_DIM], b[HAO_MV_DIM], o[HAO_MV_DIM];
    mv_arg(args[0], a, "•hao.MvContract");
    mv_arg(args[1], b, "•hao.MvContract");
    hao_mv_left_contract(a, b, o);
    return num_list(HAO_MV_DIM, o);
}

static HV *bi_hao_MvReverse(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•hao.MvReverse");
    double a[HAO_MV_DIM], o[HAO_MV_DIM];
    mv_arg(args[0], a, "•hao.MvReverse");
    hao_mv_reverse(a, o);
    return num_list(HAO_MV_DIM, o);
}

static HV *bi_hao_MvGrade(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.MvGrade");
    double a[HAO_MV_DIM], o[HAO_MV_DIM];
    int64_t k = hv_getint(args[0], "•hao.MvGrade");
    mv_arg(args[1], a, "•hao.MvGrade");
    if (hao_mv_grade(a, (int)k, o)) hb_error("domain error: grade must be 0..3");
    return num_list(HAO_MV_DIM, o);
}

static HV *bi_hao_Rotor(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Rotor");
    double angle = hv_getnum(args[1], "•hao.Rotor");
    HaoArray *ax = hv_to_hao(args[0], "•hao.Rotor");
    if (ax->element_count != 3) { hao_array_free(ax); hb_error("length error: •hao.Rotor axis must be a 3-vector"); }
    double o[HAO_MV_DIM];
    hao_rotor_from_axis_angle(ax->data, angle, o);
    hao_array_free(ax);
    return num_list(HAO_MV_DIM, o);
}

static HV *bi_hao_Sandwich(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Sandwich");
    HaoArray *r = hv_to_hao(args[0], "•hao.Sandwich"), *v = hv_to_hao(args[1], "•hao.Sandwich"), *o = NULL;
    int rc = hao_rotor_sandwich(r, v, &o);
    hao_array_free(r);
    hao_array_free(v);
    engine_check(rc, "•hao.Sandwich");
    return wrap(o);
}

static HV *bi_hao_Act(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Act");
    HaoArray *g = hv_to_hao(args[0], "•hao.Act"), *x = hv_to_hao(args[1], "•hao.Act"), *o = NULL;
    int rc = hao_group_act_matrix(g, x, &o);
    hao_array_free(g);
    hao_array_free(x);
    engine_check(rc, "•hao.Act");
    return wrap(o);
}

static HV *bi_hao_Orbit(size_t nargs, HV **args) {
    need_args(nargs, 3, 3, "•hao.Orbit");
    int64_t n = hv_getint(args[2], "•hao.Orbit");
    if (n < 0) hb_error("domain error: negative orbit length");
    HaoArray *g = hv_to_hao(args[0], "•hao.Orbit"), *x = hv_to_hao(args[1], "•hao.Orbit"), *o = NULL;
    int rc = hao_orbit(g, x, (size_t)n, &o);
    hao_array_free(g);
    hao_array_free(x);
    engine_check(rc, "•hao.Orbit");
    return wrap(o);
}

static HV *bi_hao_Stabilizes(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Stabilizes");
    HaoArray *g = hv_to_hao(args[0], "•hao.Stabilizes"), *x = hv_to_hao(args[1], "•hao.Stabilizes");
    bool s = hao_stabilizes(g, x, 1e-9);
    hao_array_free(g);
    hao_array_free(x);
    return hv_num(s);
}

typedef struct { HaoArray *a, *b, *c; size_t n; } Vecs;

static void vecs(size_t nargs, HV **args, const char *who, Vecs *v) {
    v->a = hv_to_hao(args[0], who);
    v->b = nargs > 1 ? hv_to_hao(args[1], who) : NULL;
    v->c = nargs > 2 ? hv_to_hao(args[2], who) : NULL;
    v->n = v->a->element_count;
    if ((v->b && v->b->element_count != v->n) || (v->c && v->c->element_count != v->n)) {
        hao_array_free(v->a);
        hao_array_free(v->b);
        hao_array_free(v->c);
        hb_error("length error: %s needs vectors of equal length", who);
    }
}

static void vecs_free(Vecs *v) {
    hao_array_free(v->a);
    hao_array_free(v->b);
    hao_array_free(v->c);
}

static HV *vec_result(Vecs *v, int rc, const double *out, const char *who) {
    HV *r = (rc == 0) ? num_list(v->n, out) : NULL;
    vecs_free(v);
    engine_check(rc, who);
    return r;
}

static HV *bi_hao_Log(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Log");
    Vecs v;
    vecs(nargs, args, "•hao.Log", &v);
    double *out = hb_alloc((v.n ? v.n : 1) * sizeof(double));
    int rc = hao_sphere_log(v.a->data, v.b->data, out, v.n);
    return vec_result(&v, rc, out, "•hao.Log");
}

static HV *bi_hao_Distance(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Distance");
    Vecs v;
    vecs(nargs, args, "•hao.Distance", &v);
    double d = hao_sphere_distance(v.a->data, v.b->data, v.n);
    vecs_free(&v);
    return hv_num(d);
}

static HV *bi_hao_Transport(size_t nargs, HV **args) {
    need_args(nargs, 3, 3, "•hao.Transport");
    Vecs v;
    vecs(nargs, args, "•hao.Transport", &v);
    double *out = hb_alloc((v.n ? v.n : 1) * sizeof(double));
    int rc = hao_parallel_transport(v.a->data, v.b->data, v.c->data, out, v.n);
    return vec_result(&v, rc, out, "•hao.Transport");
}

static HV *bi_hao_Reflect(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Reflect");
    Vecs v;
    vecs(nargs, args, "•hao.Reflect", &v);
    double *out = hb_alloc((v.n ? v.n : 1) * sizeof(double));
    int rc = hao_reflect(v.a->data, v.b->data, out, v.n);
    return vec_result(&v, rc, out, "•hao.Reflect");
}

static HV *bi_hao_Project(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Project");
    Vecs v;
    vecs(nargs, args, "•hao.Project", &v);
    double *out = hb_alloc((v.n ? v.n : 1) * sizeof(double));
    hao_project_tangent(v.a->data, v.b->data, out, v.n);
    return vec_result(&v, 0, out, "•hao.Project");
}

static HaoArray *square_arg(HV *m, size_t *n, const char *who) {
    HaoArray *a = hv_to_hao(m, who);
    if (a->rank != 2 || a->shape[0] != a->shape[1]) { hao_array_free(a); hb_error("rank error: %s needs a square matrix", who); }
    *n = a->shape[0];
    return a;
}

static HV *bi_hao_Det(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•hao.Det");
    size_t n;
    HaoArray *a = square_arg(args[0], &n, "•hao.Det");
    double d = hao_det(a->data, n);
    hao_array_free(a);
    return hv_num(d);
}

static HV *bi_hao_IsSO(size_t nargs, HV **args) {
    need_args(nargs, 1, 1, "•hao.IsSO");
    size_t n;
    HaoArray *a = square_arg(args[0], &n, "•hao.IsSO");
    bool s = hao_is_special_orthogonal(a->data, n, 1e-9);
    hao_array_free(a);
    return hv_num(s);
}

static HV *bi_hao_MatMul(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.MatMul");
    HaoArray *a = hv_to_hao(args[0], "•hao.MatMul"), *b = hv_to_hao(args[1], "•hao.MatMul"), *o = NULL;
    int rc = hao_matmul(a, b, &o);
    hao_array_free(a);
    hao_array_free(b);
    engine_check(rc, "•hao.MatMul");
    return wrap(o);
}

static HV *bi_hao_Contract(size_t nargs, HV **args) {
    need_args(nargs, 3, 3, "•hao.Contract");
    int64_t i = hv_getint(args[1], "•hao.Contract"), j = hv_getint(args[2], "•hao.Contract");
    if (i < 0 || j < 0) hb_error("domain error: negative axis");
    HaoArray *a = hv_to_hao(args[0], "•hao.Contract"), *o = NULL;
    int rc = hao_contract(a, (size_t)i, (size_t)j, &o);
    hao_array_free(a);
    engine_check(rc, "•hao.Contract");
    return wrap(o);
}

static HV *bi_hao_Broadcast(size_t nargs, HV **args) {
    need_args(nargs, 2, 2, "•hao.Broadcast");
    HV *sl = as_list(args[0]);
    size_t sh[HAO_MAX_RANK];
    if (sl->count > HAO_MAX_RANK) hb_error("rank error");
    for (size_t i = 0; i < sl->count; i++) sh[i] = (size_t)hv_getint(sl->el[i], "•hao.Broadcast");
    HaoArray *a = hv_to_hao(args[1], "•hao.Broadcast"), *o = NULL;
    int rc = hao_broadcast(a, sl->count, sh, &o);
    hao_array_free(a);
    engine_check(rc, "•hao.Broadcast");
    return wrap(o);
}

static HV *bi_hao_Memory(size_t nargs, HV **args) {
    (void)args;
    need_args(nargs, 1, 1, "•hao.Memory");
    HV *r = hv_list(3);
    r->el[0] = hv_num((double)hao_total_bytes);
    r->el[1] = hv_num((double)hao_peak_bytes);
    r->el[2] = hv_num((double)hao_alloc_count);
    return r;
}

typedef struct { const char *name; HBuiltin fn; } BuiltinEntry;

static const BuiltinEntry global_builtins[] = {
    {"Sin", bi_Sin}, {"Cos", bi_Cos}, {"Tan", bi_Tan}, {"Arcsin", bi_Arcsin}, {"Arccos", bi_Arccos},
    {"Arctan", bi_Arctan}, {"Sinh", bi_Sinh}, {"Cosh", bi_Cosh}, {"Tanh", bi_Tanh}, {"Exp", bi_Exp},
    {"Log", bi_Log}, {"Sqrt", bi_Sqrt}, {"Abs", bi_Abs}, {"Atan2", bi_Atan2}, {"Conj", bi_Conj},
    {NULL, NULL}
};

static const BuiltinEntry sys_builtins[] = {
    {"Show", bi_Show}, {"Out", bi_Out}, {"Repr", bi_Repr}, {"Fmt", bi_Fmt}, {"Type", bi_Type},
    {"CurrentError", bi_CurrentError},
    {"math.Sin", bi_Sin}, {"math.Cos", bi_Cos}, {"math.Tan", bi_Tan}, {"math.Asin", bi_Arcsin},
    {"math.Acos", bi_Arccos}, {"math.Atan", bi_Arctan}, {"math.Atan2", bi_Atan2}, {"math.Sinh", bi_Sinh},
    {"math.Cosh", bi_Cosh}, {"math.Tanh", bi_Tanh}, {"math.Log", bi_Log}, {"math.Exp", bi_Exp},
    {"math.Sqrt", bi_Sqrt},
    {"hao.Norm", bi_hao_Norm}, {"hao.Inner", bi_hao_Inner}, {"hao.Normalize", bi_hao_Normalize},
    {"hao.SphereExp", bi_hao_SphereExp}, {"hao.Evolve", bi_hao_Evolve}, {"hao.Geodesic", bi_hao_Geodesic},
    {"hao.GeoProd", bi_hao_GeoProd}, {"hao.Wedge", bi_hao_Wedge}, {"hao.MvOuter", bi_hao_MvOuter},
    {"hao.MvContract", bi_hao_MvContract}, {"hao.MvReverse", bi_hao_MvReverse}, {"hao.MvGrade", bi_hao_MvGrade},
    {"hao.Rotor", bi_hao_Rotor}, {"hao.Sandwich", bi_hao_Sandwich}, {"hao.Act", bi_hao_Act},
    {"hao.Orbit", bi_hao_Orbit}, {"hao.Stabilizes", bi_hao_Stabilizes}, {"hao.Log", bi_hao_Log},
    {"hao.Distance", bi_hao_Distance}, {"hao.Transport", bi_hao_Transport}, {"hao.Reflect", bi_hao_Reflect},
    {"hao.Project", bi_hao_Project}, {"hao.Det", bi_hao_Det}, {"hao.IsSO", bi_hao_IsSO},
    {"hao.MatMul", bi_hao_MatMul}, {"hao.Contract", bi_hao_Contract}, {"hao.Broadcast", bi_hao_Broadcast},
    {"hao.Memory", bi_hao_Memory},
    {NULL, NULL}
};

static HV *mk_builtin(const char *name, HBuiltin fn) {
    HV *f = hb_alloc(sizeof(HV));
    f->k = HV_FN;
    f->fk = FK_BUILTIN;
    f->bi = fn;
    f->name = name;
    return f;
}

void hb_install_builtins(HEnv *g) {
    for (size_t i = 0; global_builtins[i].name; i++)
        henv_def(g, global_builtins[i].name, mk_builtin(global_builtins[i].name, global_builtins[i].fn));
}

HV *hb_sys_lookup(const char *name) {
    for (size_t i = 0; sys_builtins[i].name; i++)
        if (strcmp(sys_builtins[i].name, name) == 0) return mk_builtin(sys_builtins[i].name, sys_builtins[i].fn);
    return NULL;
}
