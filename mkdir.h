#ifndef E2TOOLS_MKDIR_H
#define E2TOOLS_MKDIR_H

/* from mkdir.c */
extern long e2mkdir(int argc, char *argv[]);
extern long create_dir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                       char *dirname, struct stat *def_stat);
extern long change_cwd(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
		       char *dirname);

#endif /* !E2TOOLS_MKDIR_H */
