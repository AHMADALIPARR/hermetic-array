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

/* HERMETIC-Ω interpreter front end: arena, errors, lexer, parser, evaluator.

   Precedence in the HAΩ dialect (default), highest first:
     atoms, ( ), ⟨ ⟩, [ ], strands a‿b, |x| ∣x∣ bars, ⌊k⌋ brackets
     modifiers (bind left to right, BQN style)
     named calls  Name a b c   (Name is a capitalised identifier or •name;
                               it takes every directly following subject)
     tight monadic arithmetic  - + × ÷ ⋆ √ ⌊ ⌈ | ¬
     ⋆ √ (dyadic, right assoc)
     × ÷ ∧ | ⊗ ⊙               (left assoc)
     + - ∨                     (left assoc)
     every other dyadic function, named infix calls  (right assoc)
     < > ≤ ≥ = ≠ ≡ ≢ ⟂         (left assoc)
     loose monadic functions (structural, derived, blocks): take the rest
     cond ? a ◇ b   (also  cond ? a ; b)
   The strict BQN dialect (-b) evaluates every function right to left. */

#include "interp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

/* ---------- arena ---------- */

typedef struct HbChunk {
    struct HbChunk *next;
    size_t used, cap;
    double align_data;
} HbChunk;

static HbChunk *hb_chunks = NULL;

void *hb_alloc(size_t bytes) {
    bytes = (bytes + 15) & ~(size_t)15;
    if (!hb_chunks || hb_chunks->used + bytes > hb_chunks->cap) {
        size_t cap = bytes > (1u << 20) ? bytes : (1u << 20);
        HbChunk *c = malloc(sizeof(HbChunk) + cap + 16);
        if (!c) { fprintf(stderr, "hao-bqn: out of memory\n"); exit(2); }
        c->next = hb_chunks;
        c->used = 0;
        c->cap = cap;
        hb_chunks = c;
    }
    char *base = (char *)(hb_chunks + 1);
    base += (16 - ((uintptr_t)base & 15)) & 15;
    void *p = base + hb_chunks->used;
    hb_chunks->used += bytes;
    memset(p, 0, bytes);
    return p;
}

char *hb_strdup(const char *s, size_t len) {
    char *r = hb_alloc(len + 1);
    memcpy(r, s, len);
    r[len] = 0;
    return r;
}

void hao_interp_shutdown(void) {
    while (hb_chunks) {
        HbChunk *n = hb_chunks->next;
        free(hb_chunks);
        hb_chunks = n;
    }
}

/* ---------- errors ---------- */

HbHandler *hb_handler = NULL;
char hb_errmsg[512];
char hb_errtext[400];
bool hb_bqn_mode = false;
static int hb_cur_line = 0;
static const char *hb_cur_file = "";

void hao_interp_set_bqn(bool strict_bqn) { hb_bqn_mode = strict_bqn; }

/* Computed at run time: some C libraries spell HUGE_VAL as an out-of-range literal. */
double hb_zero = 0.0;
double hb_infinity(void) { return 1.0 / hb_zero; }

void hb_error(const char *fmt, ...) {
    char msg[400];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    snprintf(hb_errtext, sizeof(hb_errtext), "%s", msg);
    snprintf(hb_errmsg, sizeof(hb_errmsg), "%s:%d: %s", hb_cur_file, hb_cur_line, msg);
    if (!hb_handler) {
        fprintf(stderr, "hao-bqn: unhandled error: %s\n", hb_errmsg);
        exit(1);
    }
    longjmp(hb_handler->jb, 1);
}

/* ---------- UTF-8 ---------- */

static uint32_t utf8_next(const char *s, size_t len, size_t *i) {
    const unsigned char *u = (const unsigned char *)s;
    uint32_t c = u[*i];
    int extra = 0;
    if (c < 0x80) extra = 0;
    else if ((c & 0xE0) == 0xC0) { c &= 0x1F; extra = 1; }
    else if ((c & 0xF0) == 0xE0) { c &= 0x0F; extra = 2; }
    else if ((c & 0xF8) == 0xF0) { c &= 0x07; extra = 3; }
    else { (*i)++; return 0xFFFD; }
    (*i)++;
    for (int k = 0; k < extra; k++) {
        if (*i >= len || (u[*i] & 0xC0) != 0x80) return 0xFFFD;
        c = (c << 6) | (u[*i] & 0x3F);
        (*i)++;
    }
    return c;
}

static size_t utf8_put(uint32_t c, char *out) {
    if (c < 0x80) { out[0] = (char)c; return 1; }
    if (c < 0x800) { out[0] = (char)(0xC0 | (c >> 6)); out[1] = (char)(0x80 | (c & 0x3F)); return 2; }
    if (c < 0x10000) {
        out[0] = (char)(0xE0 | (c >> 12)); out[1] = (char)(0x80 | ((c >> 6) & 0x3F));
        out[2] = (char)(0x80 | (c & 0x3F)); return 3;
    }
    out[0] = (char)(0xF0 | (c >> 18)); out[1] = (char)(0x80 | ((c >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((c >> 6) & 0x3F)); out[3] = (char)(0x80 | (c & 0x3F));
    return 4;
}

/* ---------- lexer ---------- */

typedef enum {
    T_NUM, T_STR, T_CHR, T_NAME, T_SYS, T_PRIMF, T_PRIM1, T_PRIM2, T_ARG,
    T_LPAR, T_RPAR, T_LBRACE, T_RBRACE, T_LANGLE, T_RANGLE, T_LBRACK, T_RBRACK,
    T_STRAND, T_ASSIGN, T_MODIFY, T_EXPORT, T_SEP, T_NL, T_QUERY, T_ELSE, T_BAR,
    T_FLCLOSE, T_NOTHING, T_COLON, T_EOF
} TokKind;

typedef struct {
    TokKind k;
    double num;
    char *str;
    uint32_t cp;
    int arg;
    int line;
} Tok;

static const uint32_t PRIM_M1[] = { 0x2D9, 0x2DC, 0x2D8, 0xA8, 0x231C, 0x207C, 0xB4, 0x2DD, '`', 0 };
static const uint32_t PRIM_M2[] = { 0x2218, 0x25CB, 0x22B8, 0x27DC, 0x233E, 0x2298, 0x25F6, 0x2389, 0x2387, 0x235F, 0x238A, 0x2364, 0 };

static bool in_set(const uint32_t *set, uint32_t c) {
    for (size_t i = 0; set[i]; i++) if (set[i] == c) return true;
    return false;
}

static bool is_greek(uint32_t c) { return (c >= 0x391 && c <= 0x3C9 && c != 0x3C0); }
static bool is_ident_start(uint32_t c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || is_greek(c);
}
static bool is_ident_char(uint32_t c) { return is_ident_start(c) || (c >= '0' && c <= '9'); }

typedef struct {
    Tok *t;
    size_t n, cap;
} TokVec;

static void push_tok(TokVec *v, const Tok *t) {
    if (v->n == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 256;
        Tok *nt = hb_alloc(nc * sizeof(Tok));
        if (v->n) memcpy(nt, v->t, v->n * sizeof(Tok));
        v->t = nt;
        v->cap = nc;
    }
    v->t[v->n++] = *t;
}

static double lex_number(const char *s, size_t len, size_t *i) {
    char buf[128];
    size_t b = 0;
    size_t j = *i;
    bool neg = false;
    size_t save = j;
    uint32_t c = utf8_next(s, len, &j);
    if (c == 0xAF) { neg = true; save = j; c = utf8_next(s, len, &j); }
    if (c == 0x3C0) { *i = j; return neg ? -acos(-1.0) : acos(-1.0); }
    if (c == 0x221E) { *i = j; return neg ? -hb_infinity() : hb_infinity(); }
    j = save;
    while (j < len && b < sizeof(buf) - 2) {
        size_t k = j;
        c = utf8_next(s, len, &k);
        if ((c >= '0' && c <= '9') || c == '.') { buf[b++] = (char)c; j = k; continue; }
        if ((c == 'e' || c == 'E') && b > 0) {
            size_t k2 = k;
            uint32_t d = k2 < len ? utf8_next(s, len, &k2) : 0;
            if (d == 0xAF) {
                size_t k3 = k2;
                uint32_t d2 = k3 < len ? utf8_next(s, len, &k3) : 0;
                if (d2 >= '0' && d2 <= '9') { buf[b++] = 'e'; buf[b++] = '-'; j = k2; continue; }
            } else if (d >= '0' && d <= '9') { buf[b++] = 'e'; j = k; continue; }
        }
        break;
    }
    buf[b] = 0;
    *i = j;
    double v = strtod(buf, NULL);
    return neg ? -v : v;
}

static void lex(const char *s, size_t len, TokVec *v) {
    memset(v, 0, sizeof(*v));
    size_t i = 0;
    int line = 1;
    while (i < len) {
        size_t start = i;
        uint32_t c = utf8_next(s, len, &i);
        Tok t;
        memset(&t, 0, sizeof(t));
        t.line = line;
        if (c == '\n') { t.k = T_NL; push_tok(v, &t); line++; continue; }
        if (c == ' ' || c == '\t' || c == '\r') continue;
        if (c == '#') { while (i < len && s[i] != '\n') i++; continue; }
        if ((c >= '0' && c <= '9') || c == 0xAF || c == 0x3C0 || c == 0x221E ||
            (c == '.' && i < len && s[i] >= '0' && s[i] <= '9')) {
            i = start;
            t.k = T_NUM;
            t.num = lex_number(s, len, &i);
            push_tok(v, &t);
            continue;
        }
        if (c == '"') {
            char *buf = hb_alloc(len - i + 1);
            size_t b = 0;
            while (i < len) {
                if (s[i] == '"') {
                    if (i + 1 < len && s[i + 1] == '"') { buf[b++] = '"'; i += 2; continue; }
                    break;
                }
                if (s[i] == '\n') line++;
                buf[b++] = s[i++];
            }
            if (i >= len) { hb_cur_line = t.line; hb_error("unterminated string"); }
            i++;
            buf[b] = 0;
            t.k = T_STR;
            t.str = buf;
            push_tok(v, &t);
            continue;
        }
        if (c == '\'') {
            size_t j = i;
            uint32_t ch = utf8_next(s, len, &j);
            if (j < len && s[j] == '\'') {
                t.k = T_CHR; t.cp = ch; i = j + 1; push_tok(v, &t); continue;
            }
            hb_cur_line = t.line;
            hb_error("bad character literal");
        }
        if (c == '@') { t.k = T_CHR; t.cp = 0; push_tok(v, &t); continue; }
        if (is_ident_start(c)) {
            size_t j = i;
            while (j < len) {
                size_t k = j;
                uint32_t d = utf8_next(s, len, &k);
                if (!is_ident_char(d)) break;
                j = k;
            }
            t.k = T_NAME;
            t.str = hb_strdup(s + start, j - start);
            i = j;
            push_tok(v, &t);
            continue;
        }
        if (c == 0x2022) {
            size_t j = i;
            while (j < len) {
                size_t k = j;
                uint32_t d = utf8_next(s, len, &k);
                if (!is_ident_char(d) && d != '.') break;
                j = k;
            }
            t.k = T_SYS;
            t.str = hb_strdup(s + i, j - i);
            i = j;
            push_tok(v, &t);
            continue;
        }
        switch (c) {
            case '(': t.k = T_LPAR; break;
            case ')': t.k = T_RPAR; break;
            case '{': t.k = T_LBRACE; break;
            case '}': t.k = T_RBRACE; break;
            case 0x27E8: t.k = T_LANGLE; break;
            case 0x27E9: t.k = T_RANGLE; break;
            case '[': t.k = T_LBRACK; break;
            case ']': t.k = T_RBRACK; break;
            case 0x203F: t.k = T_STRAND; break;
            case 0x2190: t.k = T_ASSIGN; break;
            case 0x21A9: t.k = T_MODIFY; break;
            case 0x21D0: t.k = T_EXPORT; break;
            case 0x22C4: case ',': t.k = T_SEP; break;
            case '?': t.k = T_QUERY; break;
            case 0x25C7: case ';': t.k = T_ELSE; break;
            case '|': case 0x2223: t.k = T_BAR; t.cp = c; break;
            case 0x230B: case 0x2309: t.k = T_FLCLOSE; t.cp = c; break;
            case 0xB7: t.k = T_NOTHING; break;
            case ':': t.k = T_COLON; break;
            case 0x1D569: t.k = T_ARG; t.arg = ARG_X; break;
            case 0x1D568: t.k = T_ARG; t.arg = ARG_W; break;
            case 0x1D54A: t.k = T_ARG; t.arg = ARG_SELF_FN; break;
            case 0x1D564: t.k = T_ARG; t.arg = ARG_SELF_SUB; break;
            case 0x2113: t.k = T_ARG; t.arg = ARG_ELL; break;
            case 0x22A4: t.k = T_ARG; t.arg = ARG_P3; break;
            case 0x22A5: t.k = T_ARG; t.arg = ARG_P4; break;
            case 0x1D557: t.k = T_ARG; t.arg = ARG_F; break;
            case 0x1D558: t.k = T_ARG; t.arg = ARG_G; break;
            default:
                if (in_set(PRIM_M1, c)) { t.k = T_PRIM1; t.cp = c; }
                else if (in_set(PRIM_M2, c)) { t.k = T_PRIM2; t.cp = c; }
                else { t.k = T_PRIMF; t.cp = c; }
        }
        push_tok(v, &t);
    }
    Tok e;
    memset(&e, 0, sizeof(e));
    e.k = T_EOF;
    e.line = line;
    push_tok(v, &e);
}

/* ---------- parser ---------- */

typedef struct {
    Tok *t;
    size_t n, pos;
    const char *file;
    bool hao_block;   /* inside an HAΩ-style block: ⊢ ⊣ are positional arguments */
    int paren;        /* newline is insignificant inside ( ) and [ ] */
} Parser;

static HNode *node(Parser *p, HNodeKind k, HRole r) {
    HNode *n = hb_alloc(sizeof(HNode));
    n->k = k;
    n->role = r;
    n->line = p->t[p->pos < p->n ? p->pos : p->n - 1].line;
    n->file = p->file;
    return n;
}

static void add_kid(HNode *n, HNode *kid) {
    HNode **nk = hb_alloc((n->nk + 1) * sizeof(HNode *));
    if (n->nk) memcpy(nk, n->kids, n->nk * sizeof(HNode *));
    nk[n->nk++] = kid;
    n->kids = nk;
}

static Tok *peek(Parser *p) {
    while (p->paren > 0 && p->t[p->pos].k == T_NL) p->pos++;
    return &p->t[p->pos];
}

static Tok *advance(Parser *p) {
    Tok *t = peek(p);
    if (t->k != T_EOF) p->pos++;
    return t;
}

static void perr(Parser *p, const char *msg) {
    hb_cur_line = p->t[p->pos].line;
    hb_cur_file = p->file;
    hb_error("syntax error: %s", msg);
}

static void expect(Parser *p, TokKind k, const char *what) {
    if (peek(p)->k != k) perr(p, what);
    advance(p);
}

/* HAΩ: a capitalized name bound to a modifier (Compose ← ∘) keeps the
   modifier's syntactic role in every later statement and file. */
#define HB_MAX_MOD_ALIASES 256
static const char *mod_alias_name[HB_MAX_MOD_ALIASES];
static HRole mod_alias_role[HB_MAX_MOD_ALIASES];
static size_t mod_alias_count = 0;

static void note_mod_alias(const char *s, HRole r) {
    for (size_t i = 0; i < mod_alias_count; i++)
        if (strcmp(mod_alias_name[i], s) == 0) { mod_alias_role[i] = r; return; }
    if (mod_alias_count < HB_MAX_MOD_ALIASES) {
        mod_alias_name[mod_alias_count] = s;
        mod_alias_role[mod_alias_count++] = r;
    }
}

static HRole name_role(const char *s) {
    for (size_t i = 0; i < mod_alias_count; i++)
        if (strcmp(mod_alias_name[i], s) == 0) return mod_alias_role[i];
    const char *dot = strrchr(s, '.');
    if (dot && dot[1]) s = dot + 1;
    size_t i = 0;
    uint32_t c = utf8_next(s, strlen(s), &i);
    if (c == '_') {
        size_t L = strlen(s);
        return (L > 1 && s[L - 1] == '_') ? R_M2 : R_M1;
    }
    if ((c >= 'A' && c <= 'Z') || (c >= 0x391 && c <= 0x3A9)) return R_FN;
    return R_SUB;
}

static HNode *parse_stmt(Parser *p);
static HNode *parse_expr(Parser *p, uint32_t closing_bar);

static bool is_stop(TokKind k) {
    return k == T_EOF || k == T_SEP || k == T_NL || k == T_RPAR || k == T_RBRACE ||
           k == T_RANGLE || k == T_RBRACK || k == T_QUERY || k == T_ELSE || k == T_COLON;
}

/* Scan forward for a ⌋ / ⌉ closing a bracket opened at the current token. */
static bool bracket_closes(Parser *p) {
    int depth = 0;
    for (size_t i = p->pos + 1; i < p->n; i++) {
        TokKind k = p->t[i].k;
        if (k == T_LPAR || k == T_LANGLE || k == T_LBRACK || k == T_LBRACE) depth++;
        else if (k == T_RPAR || k == T_RANGLE || k == T_RBRACK || k == T_RBRACE) {
            if (depth == 0) return false;
            depth--;
        } else if (depth == 0 && (k == T_SEP || k == T_NL || k == T_EOF || k == T_QUERY || k == T_ELSE)) return false;
        else if (depth == 0 && k == T_FLCLOSE) return true;
    }
    return false;
}

static bool starts_subject(Parser *p, Tok *t) {
    switch (t->k) {
        case T_NUM: case T_STR: case T_CHR: case T_LPAR: case T_LANGLE: case T_LBRACK:
            return true;
        case T_NAME: return name_role(t->str) == R_SUB;
        case T_SYS: return name_role(t->str) == R_SUB;
        case T_ARG: return t->arg != ARG_SELF_FN;
        case T_PRIMF: return p->hao_block && (t->cp == 0x22A2 || t->cp == 0x22A3);
        default: return false;
    }
}

static HNode *parse_block(Parser *p) {
    size_t open = p->pos;
    advance(p);
    /* classify: BQN-style if 𝕩 𝕨 𝕊 𝕤 appear at depth 0; function if any argument appears */
    int depth = 0;
    bool bqn_style = false, has_arg = false;
    for (size_t i = p->pos; i < p->n; i++) {
        TokKind k = p->t[i].k;
        if (k == T_LBRACE) { depth++; continue; }
        if (k == T_RBRACE) { if (depth == 0) break; depth--; continue; }
        if (depth != 0) continue;
        if (k == T_ARG) {
            int a = p->t[i].arg;
            if (a == ARG_X || a == ARG_W || a == ARG_SELF_FN || a == ARG_SELF_SUB) bqn_style = true;
            if (a != ARG_F && a != ARG_G) has_arg = true;
        }
        if (k == T_PRIMF && (p->t[i].cp == 0x22A2 || p->t[i].cp == 0x22A3)) has_arg = true;
    }
    bool hao = !bqn_style && !hb_bqn_mode;
    if (!hao && bqn_style) {
        has_arg = false;
        depth = 0;
        for (size_t i = p->pos; i < p->n; i++) {
            TokKind k = p->t[i].k;
            if (k == T_LBRACE) { depth++; continue; }
            if (k == T_RBRACE) { if (depth == 0) break; depth--; continue; }
            if (depth == 0 && k == T_ARG && p->t[i].arg != ARG_F && p->t[i].arg != ARG_G) has_arg = true;
        }
    }
    bool saved_hao = p->hao_block;
    int saved_paren = p->paren;
    p->hao_block = hao;
    p->paren = 0;
    HNode *b = node(p, N_BLOCK, has_arg ? R_FN : R_SUB);
    b->line = p->t[open].line;
    b->fn_block = has_arg;
    b->arg = hao ? 1 : 0;
    for (;;) {
        Tok *t = peek(p);
        if (t->k == T_RBRACE) break;
        if (t->k == T_EOF) perr(p, "unclosed {");
        if (t->k == T_SEP || t->k == T_NL) { advance(p); continue; }
        add_kid(b, parse_stmt(p));
    }
    advance(p);
    p->hao_block = saved_hao;
    p->paren = saved_paren;
    return b;
}

static HNode *parse_list(Parser *p, TokKind close, HNodeKind kind) {
    advance(p);
    HNode *l = node(p, kind, R_SUB);
    int saved_paren = p->paren;
    p->paren = 0;
    for (;;) {
        Tok *t = peek(p);
        if (t->k == close) break;
        if (t->k == T_EOF) perr(p, "unclosed list");
        if (t->k == T_SEP || t->k == T_NL) { advance(p); continue; }
        add_kid(l, parse_stmt(p));
    }
    advance(p);
    p->paren = saved_paren;
    /* [1 0 0]: a single juxtaposed strand inside [ ] lists its items */
    if (kind == N_ARRLIT && l->nk == 1 && l->kids[0]->k == N_STRAND) {
        HNode *s = l->kids[0];
        l->kids = s->kids;
        l->nk = s->nk;
    }
    return l;
}

static HNode *parse_primary(Parser *p, bool operand_expected) {
    Tok *t = peek(p);
    HNode *n;
    switch (t->k) {
        case T_NUM: advance(p); n = node(p, N_NUM, R_SUB); n->num = t->num; return n;
        case T_STR: advance(p); n = node(p, N_STR, R_SUB); n->str = t->str; return n;
        case T_CHR: advance(p); n = node(p, N_CHR, R_SUB); n->cp = t->cp; return n;
        case T_NAME: advance(p); n = node(p, N_NAME, name_role(t->str)); n->str = t->str; return n;
        case T_SYS: advance(p); n = node(p, N_SYS, name_role(t->str)); n->str = t->str; return n;
        case T_NOTHING: advance(p); return node(p, N_NOTHING, R_SUB);
        case T_ARG:
            advance(p);
            n = node(p, N_ARG, t->arg == ARG_SELF_FN ? R_FN : R_SUB);
            n->arg = t->arg;
            return n;
        case T_LPAR: {
            advance(p);
            p->paren++;
            HNode *inner = parse_stmt(p);
            p->paren--;
            expect(p, T_RPAR, "expected )");
            return inner;
        }
        case T_LBRACE: return parse_block(p);
        case T_LANGLE: return parse_list(p, T_RANGLE, N_LIST);
        case T_LBRACK: return parse_list(p, T_RBRACK, N_ARRLIT);
        case T_BAR:
            if (operand_expected && !hb_bqn_mode) {
                uint32_t bar = t->cp;
                advance(p);
                HNode *inner = parse_expr(p, bar);
                if (peek(p)->k != T_BAR || peek(p)->cp != bar) perr(p, "unclosed magnitude bar");
                advance(p);
                n = node(p, N_ABS, R_SUB);
                add_kid(n, inner);
                return n;
            }
            advance(p);
            n = node(p, N_PRIM, R_FN);
            n->cp = '|';
            return n;
        case T_PRIMF:
            if (p->hao_block && (t->cp == 0x22A2 || t->cp == 0x22A3)) {
                advance(p);
                n = node(p, N_ARG, R_SUB);
                n->arg = (t->cp == 0x22A2) ? ARG_P1 : ARG_P2;
                return n;
            }
            if (operand_expected && !hb_bqn_mode && (t->cp == 0x230A || t->cp == 0x2308) && bracket_closes(p)) {
                bool floor_b = (t->cp == 0x230A);
                advance(p);
                p->paren++;
                HNode *inner = parse_stmt(p);
                p->paren--;
                if (peek(p)->k != T_FLCLOSE) perr(p, "unclosed bracket");
                advance(p);
                n = node(p, floor_b ? N_FLOORB : N_CEILB, R_SUB);
                add_kid(n, inner);
                if (starts_subject(p, peek(p))) {
                    HNode *r = node(p, N_ROTB, R_SUB);
                    add_kid(r, n);
                    HNode *y = parse_primary(p, true);
                    if (peek(p)->k == T_STRAND) {
                        HNode *st = node(p, N_LIST, R_SUB);
                        add_kid(st, y);
                        while (peek(p)->k == T_STRAND) {
                            advance(p);
                            add_kid(st, parse_primary(p, true));
                        }
                        y = st;
                    }
                    add_kid(r, y);
                    return r;
                }
                return n;
            }
            advance(p);
            n = node(p, N_PRIM, R_FN);
            n->cp = (p->hao_block && t->cp == 0x2308) ? HP_HAO_CEIL : t->cp;
            return n;
        case T_PRIM1: advance(p); n = node(p, N_M1, R_M1); n->cp = t->cp; return n;
        case T_PRIM2: advance(p); n = node(p, N_M2, R_M2); n->cp = t->cp; return n;
        default:
            perr(p, "unexpected token");
            return NULL;
    }
}

/* Primary plus explicit strands and modifiers. */
static HNode *parse_part(Parser *p, bool operand_expected) {
    HNode *x = parse_primary(p, operand_expected);
    if (peek(p)->k == T_STRAND) {
        HNode *s = node(p, N_LIST, R_SUB);
        add_kid(s, x);
        while (peek(p)->k == T_STRAND) {
            advance(p);
            add_kid(s, parse_primary(p, true));
        }
        x = s;
    }
    if (x->role == R_M1 || x->role == R_M2) return x;
    for (;;) {
        Tok *t = peek(p);
        if (t->k == T_PRIM1) {
            advance(p);
            HNode *d = node(p, N_D1, R_FN);
            d->cp = t->cp;
            add_kid(d, x);
            x = d;
        } else if (t->k == T_PRIM2) {
            advance(p);
            HNode *d = node(p, N_D2, R_FN);
            d->cp = t->cp;
            add_kid(d, x);
            HNode *g = parse_primary(p, true);
            if (g->role == R_M1 || g->role == R_M2) perr(p, "modifier operand expected");
            add_kid(d, g);
            x = d;
        } else if ((t->k == T_NAME || t->k == T_SYS) && name_role(t->str) == R_M1) {
            HNode *m = parse_primary(p, false);
            HNode *d = node(p, N_D1, R_FN);
            d->cp = 0;
            add_kid(d, x);
            add_kid(d, m);
            x = d;
        } else if ((t->k == T_NAME || t->k == T_SYS) && name_role(t->str) == R_M2) {
            HNode *m = parse_primary(p, false);
            HNode *d = node(p, N_D2, R_FN);
            d->cp = 0;
            add_kid(d, x);
            HNode *g = parse_primary(p, true);
            if (g->role == R_M1 || g->role == R_M2) perr(p, "modifier operand expected");
            add_kid(d, g);
            add_kid(d, m);
            x = d;
        } else break;
    }
    return x;
}

/* ----- precedence over a flat list of parts ----- */

typedef struct {
    HNode **v;
    size_t n, pos;
    Parser *p;
} Parts;

enum { PREC_CMP = 1, PREC_LOW = 2, PREC_ADD = 3, PREC_MUL = 4, PREC_POW = 5 };

static bool is_named_fn(const HNode *f) {
    return (f->k == N_NAME || f->k == N_SYS) && f->role == R_FN;
}

static int bin_prec(const HNode *f) {
    if (hb_bqn_mode) return PREC_CMP;
    if (f->k != N_PRIM) return PREC_LOW;
    switch (f->cp) {
        case 0x22C6: case 0x221A: return PREC_POW;
        case 0xD7: case 0xF7: case 0x2227: case '|': case 0x2223: case 0x2297: case 0x2299: return PREC_MUL;
        case '+': case '-': case 0x2228: return PREC_ADD;
        case '<': case '>': case 0x2264: case 0x2265: case '=': case 0x2260:
        case 0x2261: case 0x2262: case 0x27C2: return PREC_CMP;
        default: return PREC_LOW;
    }
}

static bool right_assoc(int prec) { return hb_bqn_mode || prec == PREC_LOW || prec == PREC_POW; }

static bool tight_monadic(const HNode *f) {
    if (hb_bqn_mode || f->k != N_PRIM) return false;
    switch (f->cp) {
        case '-': case '+': case 0xD7: case 0xF7: case 0x22C6: case 0x221A:
        case 0x230A: case 0x2308: case HP_HAO_CEIL: case '|': case 0x2223: case 0xAC:
            return true;
        default: return false;
    }
}

static HNode *pp_bin(Parts *ps, int minprec);

static HNode *mk2(Parser *p, HNodeKind k, HNode *a, HNode *b) {
    HNode *n = node(p, k, R_SUB);
    add_kid(n, a);
    add_kid(n, b);
    return n;
}

static HNode *pp_bin_from(Parts *ps, HNode *left, int minprec);

static HNode *pp_unary(Parts *ps) {
    Parser *p = ps->p;
    if (ps->pos >= ps->n) perr(p, "missing operand");
    HNode *x = ps->v[ps->pos];
    if (x->role == R_SUB) {
        ps->pos++;
        if (ps->pos < ps->n && ps->v[ps->pos]->role == R_SUB) {
            HNode *s = node(p, N_STRAND, R_SUB);
            add_kid(s, x);
            while (ps->pos < ps->n && ps->v[ps->pos]->role == R_SUB) add_kid(s, ps->v[ps->pos++]);
            return s;
        }
        return x;
    }
    if (x->role != R_FN) perr(p, "modifier without operand");
    ps->pos++;
    if (is_named_fn(x)) {
        HNode *call = node(p, N_CALL, R_SUB);
        add_kid(call, x);
        while (ps->pos < ps->n && ps->v[ps->pos]->role == R_SUB) add_kid(call, ps->v[ps->pos++]);
        if (call->nk == 1) {
            if (ps->pos < ps->n && ps->v[ps->pos]->role == R_FN) add_kid(call, hb_bqn_mode ? pp_bin(ps, PREC_CMP) : pp_unary(ps));
            else perr(p, "function called without an argument");
        } else if (hb_bqn_mode && ps->pos < ps->n && ps->v[ps->pos]->role == R_FN) {
            HNode *last = call->kids[call->nk - 1];
            call->kids[call->nk - 1] = pp_bin_from(ps, last, PREC_CMP);
        }
        return call;
    }
    HNode *operand = tight_monadic(x) ? pp_unary(ps) : pp_bin(ps, PREC_CMP);
    return mk2(p, N_MONAD, x, operand);
}

static HNode *pp_bin_from(Parts *ps, HNode *left, int minprec) {
    Parser *p = ps->p;
    while (ps->pos < ps->n) {
        HNode *f = ps->v[ps->pos];
        if (f->role != R_FN) perr(p, "expected a function");
        int prec = bin_prec(f);
        if (prec < minprec) break;
        ps->pos++;
        HNode *right = pp_bin(ps, right_assoc(prec) ? prec : prec + 1);
        HNode *d = node(p, N_DYAD, R_SUB);
        add_kid(d, f);
        add_kid(d, left);
        add_kid(d, right);
        left = d;
    }
    return left;
}

static HNode *pp_bin(Parts *ps, int minprec) {
    HNode *left = pp_unary(ps);
    return pp_bin_from(ps, left, minprec);
}

static HNode *build_train(Parser *p, HNode **v, size_t n) {
    for (size_t i = 0; i < n; i++) if (v[i]->role == R_M1 || v[i]->role == R_M2) perr(p, "stray modifier in train");
    HNode *cur = v[n - 1];
    size_t i = n - 1;
    while (i > 0) {
        if (i >= 2) {
            HNode *fk = node(p, N_FORK, R_FN);
            add_kid(fk, v[i - 2]);
            add_kid(fk, v[i - 1]);
            add_kid(fk, cur);
            if (v[i - 1]->role != R_FN) perr(p, "train needs a function in the middle");
            cur = fk;
            i -= 2;
        } else {
            HNode *at = node(p, N_ATOP, R_FN);
            if (v[i - 1]->role != R_FN) perr(p, "train needs a function on the left");
            add_kid(at, v[i - 1]);
            add_kid(at, cur);
            cur = at;
            i -= 1;
        }
    }
    return cur;
}

static HNode *parse_expr(Parser *p, uint32_t closing_bar) {
    Tok *t = peek(p);
    /* assignment: name ← value, also glyph ← value inside registries */
    if ((t->k == T_NAME || t->k == T_PRIMF || t->k == T_PRIM1 || t->k == T_PRIM2) &&
        p->pos + 1 < p->n && (p->t[p->pos + 1].k == T_ASSIGN || p->t[p->pos + 1].k == T_MODIFY || p->t[p->pos + 1].k == T_EXPORT)) {
        char buf[8];
        const char *nm;
        if (t->k == T_NAME) nm = t->str;
        else { size_t L = utf8_put(t->cp, buf); nm = hb_strdup(buf, L); }
        advance(p);
        Tok *a = advance(p);
        HNode *as = node(p, N_ASSIGN, R_SUB);
        as->str = (char *)nm;
        as->modify = (a->k == T_MODIFY);
        HNode *val = parse_expr(p, closing_bar);
        if (val->k == N_BLOCK && t->k == T_NAME && name_role(nm) == R_FN && !val->fn_block) {
            val->fn_block = true;
            val->role = R_FN;
        }
        if (t->k == T_NAME && !hb_bqn_mode && name_role(nm) == R_FN &&
            (val->role == R_M1 || val->role == R_M2))
            note_mod_alias(nm, val->role);
        add_kid(as, val);
        as->role = val->role;
        return as;
    }
    HNode **v = NULL;
    size_t n = 0, cap = 0;
    for (;;) {
        Tok *c = peek(p);
        if (is_stop(c->k) || c->k == T_FLCLOSE) break;
        if (c->k == T_BAR && closing_bar && c->cp == closing_bar && n > 0 && v[n - 1]->role == R_SUB) break;
        bool operand_expected = (n == 0) || v[n - 1]->role == R_FN;
        HNode *part = parse_part(p, operand_expected);
        if (n == cap) {
            size_t nc = cap ? cap * 2 : 8;
            HNode **nv = hb_alloc(nc * sizeof(HNode *));
            if (n) memcpy(nv, v, n * sizeof(HNode *));
            v = nv;
            cap = nc;
        }
        v[n++] = part;
    }
    if (n == 0) perr(p, "empty expression");
    if (n == 1) return v[0];
    if (v[n - 1]->role == R_FN) return build_train(p, v, n);
    for (size_t i = 0; i < n; i++) if (v[i]->role == R_M1 || v[i]->role == R_M2) perr(p, "stray modifier");
    Parts ps = { v, n, 0, p };
    HNode *e = pp_bin(&ps, PREC_CMP);
    if (ps.pos != n) perr(p, "could not parse expression");
    return e;
}

/* statement = expression [ ? statement ( ◇ | ; ) statement ] */
static HNode *parse_stmt(Parser *p) {
    HNode *e = parse_expr(p, 0);
    if (peek(p)->k == T_QUERY) {
        advance(p);
        HNode *tern = node(p, N_TERN, R_SUB);
        add_kid(tern, e);
        HNode *a = parse_stmt(p);
        add_kid(tern, a);
        if (peek(p)->k == T_ELSE) {
            advance(p);
            while (peek(p)->k == T_NL) advance(p);
            add_kid(tern, parse_stmt(p));
        }
        tern->role = a->role;
        return tern;
    }
    return e;
}

HNode *hb_parse(const char *src, size_t len, const char *file) {
    hb_cur_file = file;
    TokVec tv;
    lex(src, len, &tv);
    Parser p = { tv.t, tv.n, 0, file, false, 0 };
    HNode *seq = node(&p, N_SEQ, R_SUB);
    for (;;) {
        Tok *t = peek(&p);
        if (t->k == T_EOF) break;
        if (t->k == T_SEP || t->k == T_NL) { advance(&p); continue; }
        add_kid(seq, parse_stmt(&p));
        Tok *after = peek(&p);
        if (after->k != T_EOF && after->k != T_SEP && after->k != T_NL) perr(&p, "unexpected token after statement");
    }
    return seq;
}

/* ---------- environments ---------- */

HEnv *henv_new(HEnv *parent) {
    HEnv *e = hb_alloc(sizeof(HEnv));
    e->parent = parent;
    return e;
}

static HV **henv_slot(HEnv *e, const char *name) {
    for (size_t i = 0; i < e->n; i++) if (strcmp(e->names[i], name) == 0) return &e->vals[i];
    return NULL;
}

void henv_def(HEnv *e, const char *name, HV *v) {
    HV **s = henv_slot(e, name);
    if (s) { *s = v; return; }
    if (e->n == e->cap) {
        size_t nc = e->cap ? e->cap * 2 : 8;
        const char **nn = hb_alloc(nc * sizeof(char *));
        HV **nv = hb_alloc(nc * sizeof(HV *));
        if (e->n) { memcpy(nn, e->names, e->n * sizeof(char *)); memcpy(nv, e->vals, e->n * sizeof(HV *)); }
        e->names = nn;
        e->vals = nv;
        e->cap = nc;
    }
    e->names[e->n] = name;
    e->vals[e->n] = v;
    e->n++;
}

HV *henv_get(HEnv *e, const char *name) {
    for (; e; e = e->parent) {
        HV **s = henv_slot(e, name);
        if (s) return *s;
    }
    return NULL;
}

static HEnv *arg_env(HEnv *e) {
    for (; e; e = e->parent) if (e->has_args) return e;
    return NULL;
}

/* ---------- evaluator ---------- */

static void set_loc(const HNode *n) {
    hb_cur_line = n->line;
    if (n->file) hb_cur_file = n->file;
}

static HV *make_fn_block(const HNode *n, HEnv *e) {
    HV *f = hb_alloc(sizeof(HV));
    f->k = HV_FN;
    f->fk = FK_BLOCK;
    f->blk = n;
    f->env = e;
    return f;
}

HV *hb_run_block(const HNode *blk, HEnv *closure, size_t nargs, HV **args, HV *self) {
    HEnv *e = henv_new(closure);
    if (self) {
        e->has_args = true;
        e->self = self;
        e->nargs = nargs;
        for (size_t i = 0; i < nargs && i < 4; i++) e->args[i] = args[i];
        if (nargs == 1) { e->ell = args[0]; e->x = args[0]; e->w = NULL; }
        else if (nargs == 2) {
            HV *l = hv_list(2); l->el[0] = args[0]; l->el[1] = args[1];
            e->ell = l; e->w = args[0]; e->x = args[1];
        } else {
            HV *l = hv_list(nargs);
            for (size_t i = 0; i < nargs; i++) l->el[i] = args[i];
            e->ell = l; e->x = l; e->w = NULL;
        }
    }
    HV *r = NULL;
    for (size_t i = 0; i < blk->nk; i++) {
        const HNode *s = blk->kids[i];
        if (s->k == N_TERN && s->nk == 2) {
            /* BQN guard without alternative: a false predicate skips to the next statement */
            HV *c = hb_eval(s->kids[0], e);
            double cv = hv_getnum(c->k == HV_ARR && c->count == 1 ? c->el[0] : c, "?");
            if (cv == 1.0) return hb_eval(s->kids[1], e);
            if (cv != 0.0) hb_error("domain error: ? needs a boolean condition");
            continue;
        }
        r = hb_eval(s, e);
    }
    if (!r) r = hv_list(0);
    return r;
}

static HV *eval_list(const HNode *n, HEnv *e) {
    HV *l = hv_list(n->nk);
    for (size_t i = n->nk; i-- > 0;) l->el[i] = hb_eval(n->kids[i], e);
    return l;
}

HV *hb_eval(const HNode *n, HEnv *e) {
    set_loc(n);
    switch (n->k) {
        case N_NUM: return hv_num(n->num);
        case N_CHR: return hv_chr(n->cp);
        case N_STR: return hv_str(n->str);
        case N_NOTHING: hb_error("· (nothing) is not supported as a value"); return NULL;
        case N_NAME: {
            HV *v = henv_get(e, n->str);
            if (!v) hb_error("undefined name: %s", n->str);
            return v;
        }
        case N_SYS: {
            HV *v = hb_sys_lookup(n->str);
            if (!v) hb_error("unknown system value: •%s", n->str);
            return v;
        }
        case N_PRIM: return hv_prim_fn(n->cp);
        case N_M1: case N_M2: {
            HV *m = hb_alloc(sizeof(HV));
            m->k = (n->k == N_M1) ? HV_M1 : HV_M2;
            m->prim = n->cp;
            return m;
        }
        case N_ARG: {
            HEnv *ae = arg_env(e);
            if (!ae) hb_error("argument used outside a function block");
            HV *v = NULL;
            switch (n->arg) {
                case ARG_X: v = ae->x; break;
                case ARG_W: v = ae->w; if (!v) hb_error("𝕨 used in a monadic call"); break;
                case ARG_SELF_FN: case ARG_SELF_SUB: v = ae->self; break;
                case ARG_ELL: v = ae->ell; break;
                case ARG_P1: case ARG_P2: case ARG_P3: case ARG_P4: {
                    size_t k = (size_t)(n->arg - ARG_P1);
                    if (k >= ae->nargs) hb_error("positional argument %d not supplied (call has %d)", (int)k + 1, (int)ae->nargs);
                    v = ae->args[k];
                    break;
                }
                default: hb_error("user-defined modifiers (𝕗 𝕘) are not supported");
            }
            return v;
        }
        case N_LIST: case N_STRAND: return eval_list(n, e);
        case N_ARRLIT: {
            HV *l = eval_list(n, e);
            if (l->count == 0) return l;
            bool all_atoms = true;
            for (size_t i = 0; i < l->count; i++) if (l->el[i]->k == HV_ARR) all_atoms = false;
            if (all_atoms) return l;
            return hb_prim_monad('>', l);
        }
        case N_BLOCK:
            if (n->fn_block) return make_fn_block(n, e);
            return hb_run_block(n, e, 0, NULL, NULL);
        case N_ASSIGN: {
            HV *v = hb_eval(n->kids[0], e);
            set_loc(n);
            if (n->modify) {
                HEnv *s = e;
                for (; s; s = s->parent) if (henv_slot(s, n->str)) break;
                if (!s) hb_error("↩ to undefined name: %s", n->str);
                henv_def(s, n->str, v);
            } else {
                henv_def(e, n->str, v);
            }
            return v;
        }
        case N_MONAD: {
            HV *x = hb_eval(n->kids[1], e);
            HV *f = hb_eval(n->kids[0], e);
            set_loc(n);
            return hv_call1(f, x);
        }
        case N_DYAD: {
            HV *x = hb_eval(n->kids[2], e);
            HV *f = hb_eval(n->kids[0], e);
            HV *w = hb_eval(n->kids[1], e);
            set_loc(n);
            return hv_call2(f, w, x);
        }
        case N_CALL: {
            size_t na = n->nk - 1;
            HV **args = hb_alloc((na ? na : 1) * sizeof(HV *));
            for (size_t i = na; i-- > 0;) args[i] = hb_eval(n->kids[i + 1], e);
            HV *f = hb_eval(n->kids[0], e);
            set_loc(n);
            return hv_call(f, na, args);
        }
        case N_D1: {
            uint32_t cp = n->cp;
            if (n->nk == 2) {
                HV *m = hb_eval(n->kids[1], e);
                if (m->k != HV_M1) hb_error("%s is not a 1-modifier", n->kids[1]->str);
                cp = m->prim;
            }
            HV *f = hb_eval(n->kids[0], e);
            return hv_mod1(cp, f);
        }
        case N_D2: {
            uint32_t cp = n->cp;
            if (n->nk == 3) {
                HV *m = hb_eval(n->kids[2], e);
                if (m->k != HV_M2) hb_error("%s is not a 2-modifier", n->kids[2]->str);
                cp = m->prim;
            }
            HV *g = hb_eval(n->kids[1], e);
            HV *f = hb_eval(n->kids[0], e);
            return hv_mod2(cp, f, g);
        }
        case N_ATOP: case N_FORK: {
            HV *t = hb_alloc(sizeof(HV));
            t->k = HV_FN;
            if (n->k == N_ATOP) {
                t->fk = FK_ATOP;
                t->h = hb_eval(n->kids[1], e);
                t->g = hb_eval(n->kids[0], e);
            } else {
                t->fk = FK_FORK;
                t->h = hb_eval(n->kids[2], e);
                t->g = hb_eval(n->kids[1], e);
                t->f = hb_eval(n->kids[0], e);
            }
            return t;
        }
        case N_TERN: {
            HV *c = hb_eval(n->kids[0], e);
            double cv = hv_getnum(c->k == HV_ARR && c->count == 1 ? c->el[0] : c, "?");
            if (cv == 1.0) return hb_eval(n->kids[1], e);
            if (cv != 0.0) hb_error("domain error: ? needs a boolean condition, got %g", cv);
            if (n->nk < 3) hb_error("? predicate failed and no alternative (◇ or ;) was given");
            return hb_eval(n->kids[2], e);
        }
        case N_ABS: return hb_prim_monad('|', hb_eval(n->kids[0], e));
        case N_FLOORB: return hb_prim_monad(0x230A, hb_eval(n->kids[0], e));
        case N_CEILB: return hb_prim_monad(0x2308, hb_eval(n->kids[0], e));
        case N_ROTB: {
            HV *x = hb_eval(n->kids[1], e);
            HV *k = hb_eval(n->kids[0], e);
            return hb_prim_dyad(0x233D, k, x);
        }
        case N_SEQ: {
            HV *r = NULL;
            for (size_t i = 0; i < n->nk; i++) r = hb_eval(n->kids[i], e);
            return r ? r : hv_list(0);
        }
        default:
            hb_error("internal: bad node");
    }
    return NULL;
}

/* ---------- embedding ---------- */

HEnv *hao_interp_init(void) {
    HEnv *g = henv_new(NULL);
    hb_install_builtins(g);
    return g;
}

int hao_interp_run(HEnv *g, const char *src, size_t len, const char *file, HV **result) {
    HbHandler h;
    h.prev = hb_handler;
    hb_handler = &h;
    if (setjmp(h.jb)) {
        hb_handler = h.prev;
        return -1;
    }
    HNode *prog = hb_parse(src, len, file);
    HV *r = hb_eval(prog, g);
    if (result) *result = r;
    hb_handler = h.prev;
    return 0;
}

int hao_interp_run_file(HEnv *g, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(hb_errmsg, sizeof(hb_errmsg), "%s: cannot open file", path);
        return -1;
    }
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) { fclose(f); return -1; }
    size_t got;
    while ((got = fread(buf + len, 1, cap - len, f)) > 0) {
        len += got;
        if (len == cap) {
            char *nb = realloc(buf, cap * 2);
            if (!nb) { free(buf); fclose(f); return -1; }
            buf = nb;
            cap *= 2;
        }
    }
    fclose(f);
    char *src = hb_strdup(buf, len);
    free(buf);
    return hao_interp_run(g, src, len, hb_strdup(path, strlen(path)), NULL);
}
