#ifndef E2TOOLS_MKDIR_H
#define E2TOOLS_MKDIR_H

int main_e2mkdir(int argc, char *argv[]);
long create_dir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                char *dirname, struct stat *def_stat);
long change_cwd(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                char *dirname);

#endif /* !E2TOOLS_MKDIR_H */
