#include <stdlib.h>
#include <string.h>
#include "sort.h"
#include "fileinfo.h"

/* ───── comparators ───── */

static int cmp_name(const void *a, const void *b)
{
    const FileEntry *ea = *(const FileEntry **)a;
    const FileEntry *eb = *(const FileEntry **)b;
    return strcmp(ea->name, eb->name);
}

static int cmp_size(const void *a, const void *b)
{
    const FileEntry *ea = *(const FileEntry **)a;
    const FileEntry *eb = *(const FileEntry **)b;
    /* largest first */
    if (ea->st.st_size > eb->st.st_size) return -1;
    if (ea->st.st_size < eb->st.st_size) return  1;
    return strcmp(ea->name, eb->name); /* tie-break by name */
}

static int cmp_time(const void *a, const void *b)
{
    const FileEntry *ea = *(const FileEntry **)a;
    const FileEntry *eb = *(const FileEntry **)b;
    time_t ta = get_time(ea);
    time_t tb = get_time(eb);
    /* most recently modified first */
    if (ta > tb) return -1;
    if (ta < tb) return  1;
    return strcmp(ea->name, eb->name);
}

/* ───── sort_entries ───── */
void sort_entries(FileEntry **entries, int count)
{
    if (g_opts.opt_f || count <= 1)
        return; /* -f: no sort */

    if (g_opts.opt_S)
        qsort(entries, count, sizeof(FileEntry *), cmp_size);
    else if (g_opts.opt_t)
        qsort(entries, count, sizeof(FileEntry *), cmp_time);
    else
        qsort(entries, count, sizeof(FileEntry *), cmp_name);

    /* -r: reverse */
    if (g_opts.opt_r) {
        int lo = 0, hi = count - 1;
        while (lo < hi) {
            FileEntry *tmp = entries[lo];
            entries[lo]    = entries[hi];
            entries[hi]    = tmp;
            lo++; hi--;
        }
    }
}
