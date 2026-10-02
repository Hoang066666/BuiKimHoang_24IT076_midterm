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
        return 0;
    }

    /* separate non-directory and directory operands */
    char **files = malloc(noperands * sizeof(char *));
    char **dirs  = malloc(noperands * sizeof(char *));
    if (!files || !dirs) {
        perror("ls");
        return 1;
    }

    int nfiles = 0, ndirs = 0;
    int exit_status = 0;

    for (int i = first_arg; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) < 0) {
            fprintf(stderr, "ls: %s: %s\n", argv[i], strerror(errno));
            exit_status = 1;
            continue;
        }
        /* -d: treat dirs as plain files */
        if (S_ISDIR(st.st_mode) && !g_opts.opt_d)
            dirs[ndirs++] = argv[i];
        else
            files[nfiles++] = argv[i];
    }

    /* sort file and dir operand lists separately (lexicographic) */
    /* simple insertion sort — operand count is typically small */
    for (int i = 1; i < nfiles; i++) {
        char *tmp = files[i];
        int j = i - 1;
        while (j >= 0 && strcmp(files[j], tmp) > 0) {
            files[j + 1] = files[j];
            j--;
        }
        files[j + 1] = tmp;
    }
    for (int i = 1; i < ndirs; i++) {
        char *tmp = dirs[i];
        int j = i - 1;
        while (j >= 0 && strcmp(dirs[j], tmp) > 0) {
            dirs[j + 1] = dirs[j];
            j--;
        }
        dirs[j + 1] = tmp;
    }

    /* display non-directory operands first */
    for (int i = 0; i < nfiles; i++)
        list_single(files[i]);

    /* then directory operands */
    int print_header = (ndirs > 1 || nfiles > 0);
    for (int i = 0; i < ndirs; i++) {
        if (print_header && i > 0) putchar('\n');
        if (nfiles > 0 && i == 0) putchar('\n');
        list_directory(dirs[i], print_header);
    }

    free(files);
    free(dirs);
    return exit_status;
}
