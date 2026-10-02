#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

Options g_opts;

int parse_options(int argc, char *argv[])
{
    int i;

    memset(&g_opts, 0, sizeof(g_opts));

    /* default: -q when stdout is a terminal (handled in display.c) */

    for (i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][1] == '\0')
            break; /* first non-option argument */

        if (argv[i][1] == '-' && argv[i][2] == '\0') {
            i++; /* "--" ends option parsing */
            break;
        }

        const char *p = argv[i] + 1;
        while (*p) {
            switch (*p) {
            case 'A': g_opts.opt_A = 1; break;
            case 'a': g_opts.opt_a = 1; break;
            case 'c': g_opts.opt_c = 1; g_opts.opt_u = 0; break;
            case 'd': g_opts.opt_d = 1; g_opts.opt_R = 0; break;
            case 'F': g_opts.opt_F = 1; break;
            case 'f': g_opts.opt_f = 1; break;
            case 'h': g_opts.opt_h = 1; g_opts.opt_k = 0; break;
            case 'i': g_opts.opt_i = 1; break;
            case 'k': g_opts.opt_k = 1; g_opts.opt_h = 0; break;
            case 'l': g_opts.opt_l = 1; g_opts.opt_n = 0; break;
            case 'n': g_opts.opt_n = 1; g_opts.opt_l = 0; break;
            case 'q': g_opts.opt_q = 1; g_opts.opt_w = 0; break;
            case 'R': g_opts.opt_R = 1; g_opts.opt_d = 0; break;
            case 'r': g_opts.opt_r = 1; break;
            case 'S': g_opts.opt_S = 1; break;
            case 's': g_opts.opt_s = 1; break;
            case 't': g_opts.opt_t = 1; break;
            case 'u': g_opts.opt_u = 1; g_opts.opt_c = 0; break;
            case 'w': g_opts.opt_w = 1; g_opts.opt_q = 0; break;
            default:
                fprintf(stderr, "ls: illegal option -- %c\n", *p);
                fprintf(stderr, "usage: ls [-AacdFfhiklnqRrSstuw] [file ...]\n");
                exit(1);
            }
            p++;
        }
    }

    /* -f implies -a, disables sort */
    if (g_opts.opt_f)
        g_opts.opt_a = 1;

    return i; /* index of first non-option arg */
}
