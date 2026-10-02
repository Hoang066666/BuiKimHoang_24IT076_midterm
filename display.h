#ifndef DISPLAY_H
#define DISPLAY_H

#include "ls.h"

/*
 * list_directory - read and display the contents of a directory path.
 * If print_header is non-zero, print "path:" before the listing
 * (used for multiple args or -R).
 */
void list_directory(const char *path, int print_header);

/*
 * list_entries - display an array of FileEntry pointers.
 * Handles -i, -s, -l/-n, -F prefixes/suffixes.
 */
void list_entries(FileEntry **entries, int count);

/*
 * list_single - stat and display a single non-directory operand.
 */
void list_single(const char *path);

#endif /* DISPLAY_H */
