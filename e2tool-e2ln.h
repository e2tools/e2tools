/** \file e2tool-e2ln.h
 * \brief File link tool interface.
 */

#ifndef E2TOOL_E2LN_H
#define E2TOOL_E2LN_H

/* Apparently (and weirdly), ext2fs.h wants some macros like
 * HAVE_SYS_TYPES_H defined before it is included, so we include
 * "e2tools-autoconfig.h" before <ext2fs.h>.
 *
 * See https://github.com/e2tools/e2tools/issues/1#issuecomment-582913807
 */
#include "e2tools-autoconfig.h"

#include <ext2fs.h>

extern
int main_e2ln(int argc, char *argv[]);

extern
long create_hard_link(ext2_filsys fs, ext2_ino_t cwd, ext2_ino_t
                      new_file_ino, char *newfile, int ln_flags);

#endif /* !defined(E2TOOL_E2LN_H) */
