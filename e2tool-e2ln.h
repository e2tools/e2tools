#ifndef E2TOOL_E2LN_H
#define E2TOOL_E2LN_H

#include <ext2fs.h>

extern
int main_e2ln(int argc, char *argv[]);

extern
long create_hard_link(ext2_filsys fs, ext2_ino_t cwd, ext2_ino_t
                      new_file_ino, char *newfile, int ln_flags);

#endif /* !defined(E2TOOL_E2LN_H) */
