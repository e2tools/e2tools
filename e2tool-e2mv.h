#ifndef E2TOOL_E2MV_H
#define E2TOOL_E2MV_H

extern
int main_e2mv(int argc, char *argv[]);

extern
long get_file_parts(ext2_filsys fs, ext2_ino_t root, char *pathname,
                    ext2_ino_t *dir_ino, char **dir_name,
                    char **base_name);

#endif /* !defined(E2TOOL_E2MV_H) */
