#ifndef OPTIONS_H
#define OPTIONS_H

#include "ls.h"

/*
 * parse_options - parse argv, fill g_opts, return index of first non-option arg.
 * Exits with error message on unknown option.
 */
int parse_options(int argc, char *argv[]);

#endif /* OPTIONS_H */
