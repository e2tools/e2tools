#ifndef E2TOOLS_READ_H
#define E2TOOLS_READ_H

/* from read.c */
extern long get_file(ext2_filsys fs, ext2_ino_t root, ext2_ino_t cwd,
                     char *infile, char *outfile, int keep);
extern long retrieve_data(ext2_filsys fs, ext2_ino_t src, int dest_fd,
                          char *dest_name, int keep, ext2_off_t offset,
                          ext2_off_t *ret_pos);
extern long read_to_eof(ext2_file_t infile, int dest_fd, ext2_off_t offset,
                        ext2_off_t *ret_pos);

#endif /* !E2TOOLS_READ_H */
