#ifndef E2TOOLS_UTIL_H
#define E2TOOLS_UTIL_H

mode_t ext2_mode_xlate(__u16 lmode);
__u16 host_mode_xlate(mode_t hmode);
long open_filesystem(char *name, ext2_filsys *fs, ext2_ino_t *root, int
                     rw_mode);
long read_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode
                *inode);
long write_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode
                 *inode);
long rm_file(ext2_filsys fs, ext2_ino_t cwd, char *outfile, ext2_ino_t
             delfile);
long delete_file(ext2_filsys fs, ext2_ino_t inode);
void init_stat_buf(struct stat *buf);
int  is_file_regexp(char *ptr);
regex_t *make_regexp(char *shell);

#endif /* !E2TOOLS_UTIL_H */
