#ifndef E2TOOLS_H
#define E2TOOLS_H

/* $Header: /home/ksheff/src/e2tools/RCS/e2tools.h,v 0.7 2004/04/07 01:15:55 ksheff Exp $ */

/* Copyright 2002 Keith W. Sheffield */

/* Description */
/*
 * $Log: e2tools.h,v $
 * Revision 0.7  2004/04/07 01:15:55  ksheff
 * Added the parameter struct stat *def_stat to put_file().
 *
 * Revision 0.6  2002/08/08 07:57:04  ksheff
 * Added new routine do_tail() from tail.c
 * Added new routine read_to_eof() from read.c
 * Made retrieve_data() from read.c a public routine
 *
 * Revision 0.5  2002/04/10 10:43:27  ksheff
 * Added e2rm().
 *
 * Revision 0.4  2002/04/10 09:33:26  ksheff
 * Modified prototypes for functions involved with setting directory
 * attributes.
 *
 * Revision 0.3  2002/03/21 09:05:16  ksheff
 * Added function prototypes from mv.c and altered do_ln() slightly.
 *
 * Revision 0.2  2002/03/07 07:26:02  ksheff
 * Added function prototypes and defined the macros E2T_FORCE and E2T_DO_MV
 *
 * Revision 0.1  2002/02/27 04:47:44  ksheff
 * initial revision
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <utime.h>
/*
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else 
#endif
*/

#ifdef HAVE_OPTRESET
extern int optreset;		/* defined by BSD, but not others */
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"

#define E2T_FORCE 1
#define E2T_DO_MV 2

#ifndef COPY_C
extern long copy(int argc, char *argv[]);
extern int my_strcmp(const void *n1, const void *n2);
#endif

#ifdef LN_C
extern long do_ln(int argc, char *argv[]);

extern long create_hard_link(ext2_filsys fs, ext2_ino_t cwd, ext2_ino_t
                             new_file_ino, char *newfile, int ln_flags);
#endif

#ifndef LS_C
extern long do_list_dir(int argc, char *argv[]);
#endif

#ifndef MKDIR_C
extern long e2mkdir(int argc, char *argv[]);
extern long create_dir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                       char *dirname, struct stat *def_stat);
#endif

#ifdef MV_C
extern long do_mv(int argc, char *argv[]);
extern long get_file_parts(ext2_filsys fs, ext2_ino_t root, char *pathname,
                           ext2_ino_t *dir_ino, char **dir_name,
                           char **base_name);

#endif


#ifndef READ_C
extern long get_file(ext2_filsys fs, ext2_ino_t root, ext2_ino_t cwd,
                     char *infile, char *outfile, int keep);
extern long retrieve_data(ext2_filsys fs, ext2_ino_t src, int dest_fd,
                          char *dest_name, int keep, ext2_off_t offset,
                          ext2_off_t *ret_pos);
extern long read_to_eof(ext2_file_t infile, int dest_fd, ext2_off_t offset,
                        ext2_off_t *ret_pos);
#endif

#ifndef RM_C
extern long e2rm(int argc, char *argv[]);
#endif

#ifndef TAIL_C
extern long do_tail(int argc, char *argv[]);
#endif

#ifndef UTIL_C
extern mode_t ext2_mode_xlate(__u16 lmode);
extern __u16 host_mode_xlate(mode_t hmode);
extern long open_filesystem(char *name, ext2_filsys *fs, ext2_ino_t *root, int
                            rw_mode);
extern long read_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode
                       *inode); 
extern long write_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode
                        *inode); 
extern long rm_file(ext2_filsys fs, ext2_ino_t cwd, char *outfile, ext2_ino_t
                    delfile); 
extern long delete_file(ext2_filsys fs, ext2_ino_t inode);
extern void init_stat_buf(struct stat *buf);
#endif

#ifndef WRITE_C
extern long
put_file(ext2_filsys fs, ext2_ino_t cwd, char *infile, char *outfile,
         ext2_ino_t *outfile_ino, int keep, struct stat *def_stat);
#endif

#endif


