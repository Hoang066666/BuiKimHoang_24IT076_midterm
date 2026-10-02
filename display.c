#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include "display.h"
#include "fileinfo.h"
#include "format.h"
#include "sort.h"

/* ───── helpers ───── */

/* Print inode + block prefix if -i / -s are set */
static void print_prefix(const FileEntry *e)
{
    if (g_opts.opt_i)
        printf("%lu ", (unsigned long)e->st.st_ino);

    if (g_opts.opt_s) {
        if (g_opts.opt_h) {
            char buf[16];
            human_size((off_t)e->st.st_blocks * 512, buf, sizeof(buf));
            printf("%s ", buf);
        } else {
            printf("%ld ", block_count(e));
        }
    }
}

/* Print long-format line for one entry */
static void print_long(const FileEntry *e)
{
    char modebuf[11];
    mode_string(e->st.st_mode, modebuf);

    /* owner / group */
    char owner[32], group[32];
    if (g_opts.opt_n) {
        snprintf(owner, sizeof(owner), "%u", (unsigned)e->st.st_uid);
        snprintf(group, sizeof(group), "%u", (unsigned)e->st.st_gid);
    } else {
        struct passwd *pw = getpwuid(e->st.st_uid);
        struct group  *gr = getgrgid(e->st.st_gid);
        if (pw) snprintf(owner, sizeof(owner), "%s", pw->pw_name);
        else    snprintf(owner, sizeof(owner), "%u", (unsigned)e->st.st_uid);
        if (gr) snprintf(group, sizeof(group), "%s", gr->gr_name);
        else    snprintf(group, sizeof(group), "%u", (unsigned)e->st.st_gid);
    }

    /* size or device numbers */
    char sizebuf[32];
    if (S_ISCHR(e->st.st_mode) || S_ISBLK(e->st.st_mode)) {
        snprintf(sizebuf, sizeof(sizebuf), "%3u, %3u",
                 (unsigned)major(e->st.st_rdev),
                 (unsigned)minor(e->st.st_rdev));
    } else if (g_opts.opt_h) {
        human_size(e->st.st_size, sizebuf, sizeof(sizebuf));
    } else {
        snprintf(sizebuf, sizeof(sizebuf), "%lld", (long long)e->st.st_size);
    }

    /* time */
    time_t t = get_time(e);
    struct tm *tm = localtime(&t);
    char timebuf[16];
    time_t now = time(NULL);
    /* files older than ~6 months show year instead of time */
    if (now - t > 15552000 || t > now)
        strftime(timebuf, sizeof(timebuf), "%b %e  %Y", tm);
    else
        strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

    printf("%s %2lu %-8s %-8s %8s %s ",
           modebuf,
           (unsigned long)e->st.st_nlink,
           owner, group,
           sizebuf,
           timebuf);

    print_name(e->name);

    /* symlink target */
    if (S_ISLNK(e->st.st_mode)) {
        char target[1024];
        ssize_t len = readlink(e->path, target, sizeof(target) - 1);
        if (len >= 0) {
            target[len] = '\0';
            printf(" -> %s", target);
        }
    }
    putchar('\n');
}

/* ───── list_entries ───── */
void list_entries(FileEntry **entries, int count)
{
    /* total blocks line for -l/-s when output is listing a directory */
    if ((g_opts.opt_l || g_opts.opt_n || g_opts.opt_s) && count > 0) {
        long total = 0;
        for (int i = 0; i < count; i++)
            total += (long)entries[i]->st.st_blocks;
        /* st_blocks in 512-byte units; display in 512-byte units */
        printf("total %ld\n", total / (g_opts.opt_k ? 2 : 1));
    }

    for (int i = 0; i < count; i++) {
        FileEntry *e = entries[i];
        print_prefix(e);

        if (g_opts.opt_l || g_opts.opt_n) {
            print_long(e);
        } else {
            print_name(e->name);
            if (g_opts.opt_F) {
                char ind = type_indicator(e);
                if (ind) putchar(ind);
            }
            putchar('\n');
        }
    }
}

/* ───── list_directory ───── */
void list_directory(const char *path, int print_header)
{
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return;
    }

    /* collect entries */
    int capacity = 64, count = 0;
    FileEntry **entries = malloc(capacity * sizeof(FileEntry *));
    if (!entries) { closedir(dir); return; }

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        const char *name = de->d_name;

        /* filter dotfiles */
        if (name[0] == '.') {
            if (!g_opts.opt_a && !g_opts.opt_A)
                continue;
            if (g_opts.opt_A && (strcmp(name, ".") == 0 || strcmp(name, "..") == 0))
                continue;
        }

        FileEntry *e = make_entry(path, name);
        if (!e) {
            fprintf(stderr, "ls: %s/%s: %s\n", path, name, strerror(errno));
            continue;
        }

        if (count == capacity) {
            capacity *= 2;
            FileEntry **tmp = realloc(entries, capacity * sizeof(FileEntry *));
            if (!tmp) { free_entry(e); break; }
            entries = tmp;
        }
        entries[count++] = e;
    }
    closedir(dir);

    sort_entries(entries, count);

    if (print_header)
        printf("%s:\n", path);

    list_entries(entries, count);

    /* -R: recurse into subdirectories */
    if (g_opts.opt_R) {
        for (int i = 0; i < count; i++) {
            FileEntry *e = entries[i];
            if (S_ISDIR(e->st.st_mode)
                && strcmp(e->name, ".") != 0
                && strcmp(e->name, "..") != 0)
            {
                putchar('\n');
                list_directory(e->path, 1);
            }
        }
    }

    for (int i = 0; i < count; i++)
        free_entry(entries[i]);
    free(entries);
}

/* ───── list_single ───── */
void list_single(const char *path)
{
    FileEntry *e = make_entry("", path);
    if (!e) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return;
    }
    /* fix up name to basename */
    char *slash = strrchr(path, '/');
    free(e->name);
    e->name = strdup(slash ? slash + 1 : path);

    print_prefix(e);
    if (g_opts.opt_l || g_opts.opt_n)
        print_long(e);
    else {
        print_name(e->name);
        if (g_opts.opt_F) {
            char ind = type_indicator(e);
            if (ind) putchar(ind);
        }
        putchar('\n');
    }
    free_entry(e);
}
