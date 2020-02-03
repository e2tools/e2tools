#ifndef E2TOOLS_LN_H
#define E2TOOLS_LN_H

long do_ln(int argc, char *argv[]);

long create_hard_link(ext2_filsys fs, ext2_ino_t cwd, ext2_ino_t
                      new_file_ino, char *newfile, int ln_flags);

#endif /* !E2TOOLS_LN_H */
