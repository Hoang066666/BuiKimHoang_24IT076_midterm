#ifndef LS_H
#define LS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

/* ───── Option flags ───── */
typedef struct {
    int opt_A;   /* -A: list all except . and .. */
    int opt_a;   /* -a: include dotfiles */
    int opt_c;   /* -c: use ctime for sort/print */
    int opt_d;   /* -d: list dirs as plain files */
    int opt_F;   /* -F: append type indicators */
    int opt_f;   /* -f: no sort */
    int opt_h;   /* -h: human-readable sizes */
    int opt_i;   /* -i: print inode number */
    int opt_k;   /* -k: sizes in kilobytes */
    int opt_l;   /* -l: long format */
    int opt_n;   /* -n: like -l but numeric uid/gid */
    int opt_q;   /* -q: replace non-printable with '?' */
    int opt_R;   /* -R: recursive */
    int opt_r;   /* -r: reverse sort */
    int opt_S;   /* -S: sort by size */
    int opt_s;   /* -s: show block usage */
    int opt_t;   /* -t: sort by time */
    int opt_u;   /* -u: use atime */
    int opt_w;   /* -w: raw non-printable chars */
} Options;

/* ───── File entry ───── */
typedef struct {
    char        *name;      /* display name */
    char        *path;      /* full path */
    struct stat  st;        /* stat info */
} FileEntry;

/* ───── Global options instance ───── */
extern Options g_opts;

#endif /* LS_H */
