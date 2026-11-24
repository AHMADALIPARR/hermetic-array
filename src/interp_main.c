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

/* HERMETIC-Ω interpreter command line: hao-bqn
   usage: hao-bqn [-b] [-k] [-e expr | -x expr | file]...
     file     run a source file (all files share one global environment)
     -e expr  evaluate expr and print its value
     -x expr  evaluate expr silently
     -b       strict BQN dialect (right-to-left, ˘ is Cells, ⊢ ⊣ are functions)
     -k       keep going after an error (exit status is still 1)
   Arguments are processed left to right, so -b must come first. */

#include "interp.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(void) {
    fprintf(stderr, "usage: hao-bqn [-b] [-k] [-e expr | -x expr | file]...\n");
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(); return 2; }
    HEnv *g = NULL;
    bool keep_going = false;
    int status = 0;
    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (strcmp(a, "-b") == 0) { hao_interp_set_bqn(true); continue; }
        if (strcmp(a, "-k") == 0) { keep_going = true; continue; }
        if (!g) g = hao_interp_init();
        int rc;
        if (strcmp(a, "-e") == 0 || strcmp(a, "-x") == 0) {
            if (i + 1 >= argc) { usage(); hao_interp_shutdown(); return 2; }
            const char *src = argv[++i];
            HV *res = NULL;
            rc = hao_interp_run(g, src, strlen(src), "<expr>", &res);
            if (rc == 0 && a[1] == 'e' && res) printf("%s\n", hb_repr(res));
        } else {
            rc = hao_interp_run_file(g, a);
        }
        if (rc != 0) {
            fflush(stdout);
            fprintf(stderr, "Error: %s\n", hb_errmsg);
            status = 1;
            if (!keep_going) break;
        }
    }
    fflush(stdout);
    hao_interp_shutdown();
    return status;
}
