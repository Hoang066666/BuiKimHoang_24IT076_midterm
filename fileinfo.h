#ifndef FILEINFO_H
#define FILEINFO_H

#include "ls.h"

/*
 * make_entry - allocate and fill a FileEntry from path.
 * Uses lstat so symlinks are not followed (unless -L were specified).
 * Returns NULL on error (errno set).
 */
FileEntry *make_entry(const char *dir, const char *name);

/* free_entry - release memory owned by a FileEntry */
void free_entry(FileEntry *e);

/*
 * get_time - return the relevant time_t for sorting/printing
 * based on -c (ctime), -u (atime), or default (mtime).
 */
time_t get_time(const FileEntry *e);

/*
 * mode_string - write a 10-char mode string (like "drwxr-xr-x") into buf.
 * buf must be at least 11 bytes.
 */
void mode_string(mode_t mode, char *buf);

/*
 * type_indicator - return the -F indicator character for a file, or '\0'.
 */
char type_indicator(const FileEntry *e);

#endif /* FILEINFO_H */
