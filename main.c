#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "ls.h"
#include "options.h"
#include "fileinfo.h"
#include "display.h"
#include "sort.h"

int main(int argc, char *argv[])
{
    int first_arg = parse_options(argc, argv);

    int noperands = argc - first_arg;

    /* no operands: list current directory */
    if (noperands == 0) {
        list_directory(".", 0);
        return g_exit_status;
    }

    /* separate non-directory and directory operands */
    char **files = malloc(noperands * sizeof(char *));
    char **dirs  = malloc(noperands * sizeof(char *));
    if (!files || !dirs) {
        perror("ls");
        return 1;
    }

    int nfiles = 0, ndirs = 0;

    for (int i = first_arg; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) < 0) {
            fprintf(stderr, "ls: %s: %s\n", argv[i], strerror(errno));
            g_exit_status = 1;
            continue;
        }
        /* -d: treat dirs as plain files */
        if (S_ISDIR(st.st_mode) && !g_opts.opt_d)
            dirs[ndirs++] = argv[i];
        else
            files[nfiles++] = argv[i];
    }

    /* display non-directory operands first */
    if (nfiles > 0) {
        FileEntry **file_entries = malloc(nfiles * sizeof(FileEntry *));
        if (file_entries) {
            int valid_files = 0;
            for (int i = 0; i < nfiles; i++) {
                FileEntry *e = make_entry("", files[i]);
                if (!e) {
                    fprintf(stderr, "ls: %s: %s\n", files[i], strerror(errno));
                    g_exit_status = 1;
                    continue;
                }
                char *slash = strrchr(files[i], '/');
                free(e->name);
                e->name = strdup(slash ? slash + 1 : files[i]);
                file_entries[valid_files++] = e;
            }

            sort_entries(file_entries, valid_files);
            list_entries(file_entries, valid_files, 0);

            for (int i = 0; i < valid_files; i++)
                free_entry(file_entries[i]);
            free(file_entries);
        }
    }

    /* sort directory operands */
    if (!g_opts.opt_f && ndirs > 1) {
        for (int i = 1; i < ndirs; i++) {
            char *tmp = dirs[i];
            int j = i - 1;
            while (j >= 0 && strcmp(dirs[j], tmp) > 0) {
                dirs[j + 1] = dirs[j];
                j--;
            }
            dirs[j + 1] = tmp;
        }
        if (g_opts.opt_r) {
            int lo = 0, hi = ndirs - 1;
            while (lo < hi) {
                char *tmp = dirs[lo];
                dirs[lo] = dirs[hi];
                dirs[hi] = tmp;
                lo++; hi--;
            }
        }
    }

    /* then directory operands */
    int print_header = (ndirs > 1 || nfiles > 0);
    for (int i = 0; i < ndirs; i++) {
        if (print_header && i > 0) putchar('\n');
        if (nfiles > 0 && i == 0) putchar('\n');
        list_directory(dirs[i], print_header);
    }

    free(files);
    free(dirs);
    return g_exit_status;
}
