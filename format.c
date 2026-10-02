#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "format.h"

/* ───── human_size ───── */
void human_size(off_t size, char *buf, int buflen)
{
    const char *units[] = {"B", "K", "M", "G", "T", "P"};
    double val = (double)size;
    int u = 0;
    while (val >= 1024.0 && u < 5) {
        val /= 1024.0;
        u++;
    }
    if (u == 0)
        snprintf(buf, buflen, "%lld%s", (long long)size, units[u]);
    else if (val < 10.0)
        snprintf(buf, buflen, "%.1f%s", val, units[u]);
    else
        snprintf(buf, buflen, "%.0f%s", val, units[u]);
}

/* ───── block_count ───── */
long block_count(const FileEntry *e)
{
    long blksz = 512;

    if (g_opts.opt_k)
        blksz = 1024;
    else {
        char *bs = getenv("BLOCKSIZE");
        if (bs && *bs)
            blksz = atol(bs);
    }

    if (blksz <= 0) blksz = 512;

    /* st_blocks is in 512-byte units */
    long blocks512 = (long)e->st.st_blocks;
    /* round up */
    return (blocks512 * 512 + blksz - 1) / blksz;
}

/* ───── print_name ───── */
void print_name(const char *name)
{
    /* -q replaces non-printable with '?'
       -w prints raw; that's the default when not a terminal */
    int use_q = g_opts.opt_q || (!g_opts.opt_w && isatty(STDOUT_FILENO));

    for (; *name; name++) {
        unsigned char c = (unsigned char)*name;
        if (use_q && !isprint(c))
            putchar('?');
        else
            putchar(c);
    }
}
