#ifndef SORT_H
#define SORT_H

#include "ls.h"

/*
 * sort_entries - sort an array of FileEntry pointers in-place
 * according to current g_opts (S, t, r, f).
 */
void sort_entries(FileEntry **entries, int count);

#endif /* SORT_H */
