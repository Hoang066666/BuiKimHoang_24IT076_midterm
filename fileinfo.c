#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "fileinfo.h"

/* ───── make_entry ───── */
FileEntry *make_entry(const char *dir, const char *name)
{
    FileEntry *e = calloc(1, sizeof(FileEntry));
    if (!e) return NULL;

    e->name = strdup(name);
    if (!e->name) { free(e); return NULL; }

    /* build full path */
    size_t dlen = strlen(dir);
    size_t nlen = strlen(name);
    /* dir + '/' + name + '\0' */
    e->path = malloc(dlen + 1 + nlen + 1);
    if (!e->path) { free(e->name); free(e); return NULL; }

    if (dlen > 0 && dir[dlen - 1] == '/')
        snprintf(e->path, dlen + nlen + 1, "%s%s", dir, name);
    else
        snprintf(e->path, dlen + nlen + 2, "%s/%s", dir, name);

    if (lstat(e->path, &e->st) < 0) {
        /* try treating dir as empty (name is a full path itself) */
        if (lstat(name, &e->st) < 0) {
            free(e->path);
            free(e->name);
            free(e);
            return NULL;
        }
        free(e->path);
        e->path = strdup(name);
    }

    return e;
}

void free_entry(FileEntry *e)
{
    if (!e) return;
    free(e->name);
    free(e->path);
    free(e);
}

/* ───── get_time ───── */
time_t get_time(const FileEntry *e)
{
    if (g_opts.opt_u) return e->st.st_atime;
    if (g_opts.opt_c) return e->st.st_ctime;
    return e->st.st_mtime;
}

/* ───── mode_string ───── */
void mode_string(mode_t mode, char *buf)
{
    /* entry type */
    if      (S_ISREG(mode))  buf[0] = '-';
    else if (S_ISDIR(mode))  buf[0] = 'd';
    else if (S_ISLNK(mode))  buf[0] = 'l';
    else if (S_ISCHR(mode))  buf[0] = 'c';
    else if (S_ISBLK(mode))  buf[0] = 'b';
    else if (S_ISFIFO(mode)) buf[0] = 'p';
    else if (S_ISSOCK(mode)) buf[0] = 's';
    else                      buf[0] = '?';

    /* owner */
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID)
        buf[3] = (mode & S_IXUSR) ? 's' : 'S';
    else
        buf[3] = (mode & S_IXUSR) ? 'x' : '-';

    /* group */
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID)
        buf[6] = (mode & S_IXGRP) ? 's' : 'S';
    else
        buf[6] = (mode & S_IXGRP) ? 'x' : '-';

    /* other */
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX)
        buf[9] = (mode & S_IXOTH) ? 't' : 'T';
    else
        buf[9] = (mode & S_IXOTH) ? 'x' : '-';

    buf[10] = '\0';
}

/* ───── type_indicator ───── */
char type_indicator(const FileEntry *e)
{
    mode_t m = e->st.st_mode;
    if (S_ISDIR(m))  return '/';
    if (S_ISLNK(m))  return '@';
    if (S_ISFIFO(m)) return '|';
    if (S_ISSOCK(m)) return '=';
    if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) return '*';
    return '\0';
}
