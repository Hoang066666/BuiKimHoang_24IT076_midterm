#ifndef FORMAT_H
#define FORMAT_H

#include "ls.h"

/*
 * human_size - convert byte count to human-readable string (e.g. "1.5K").
 * buf must be at least 8 bytes.
 */
void human_size(off_t size, char *buf, int buflen);

/*
 * block_count - return block count for -s option, respecting BLOCKSIZE env.
 * Unit is 512 bytes by default; -k makes it 1024; -h makes it human-readable
 * (handled by caller).
 */
long block_count(const FileEntry *e);

/*
 * print_name - print filename, replacing non-printable chars per -q/-w.
 */
void print_name(const char *name);

#endif /* FORMAT_H */
