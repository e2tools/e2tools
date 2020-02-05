#ifndef E2TOOLS_UTIL_H
#define E2TOOLS_UTIL_H

extern
mode_t ext2_mode_xlate(__u16 lmode);

extern
__u16 host_mode_xlate(mode_t hmode);

extern
long open_filesystem(char *name, ext2_filsys *fs, ext2_ino_t *root, int
                     rw_mode);

extern
long read_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode
                *inode);

extern
long write_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode
                 *inode);

extern
long rm_file(ext2_filsys fs, ext2_ino_t cwd, char *outfile, ext2_ino_t
             delfile);

extern
long delete_file(ext2_filsys fs, ext2_ino_t inode);

extern
void init_stat_buf(struct stat *buf);

extern
int  is_file_regexp(char *ptr);

extern
regex_t *make_regexp(char *shell);

#endif /* !defined(E2TOOLS_UTIL_H) */
