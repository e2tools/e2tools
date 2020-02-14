/** \file e2tool-e2mkdir.h
 * \brief Directory creation tool interface.
 */

#ifndef E2TOOL_E2MKDIR_H
#define E2TOOL_E2MKDIR_H

extern
int main_e2mkdir(int argc, char *argv[]);

extern
long create_dir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                char *dirname, struct stat *def_stat);

extern
long change_cwd(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                char *dirname);

#endif /* !defined(E2TOOL_E2MKDIR_H) */
