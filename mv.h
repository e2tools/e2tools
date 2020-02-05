#ifndef E2TOOLS_MV_H
#define E2TOOLS_MV_H

int main_e2mv(int argc, char *argv[]);
long get_file_parts(ext2_filsys fs, ext2_ino_t root, char *pathname,
                    ext2_ino_t *dir_ino, char **dir_name,
                    char **base_name);

#endif /* !E2TOOLS_MV_H */
