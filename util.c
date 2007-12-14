/* $Header: /home/ksheff/src/e2tools/RCS/util.c,v 0.4 2002/06/05 20:38:16 ksheff Exp $ */
/*
 * util.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Derived from debugfs.c Copyright (C) 1993 Theodore Ts'o <tytso@mit.edu>
 * and modified by Robert Sanders <gt8134b@prism.gatech.edu>
 */

/* Description */
/*
 * Misc. utility functions
 *
 */
/*
 * $Log: util.c,v $
 * Revision 0.4  2002/06/05 20:38:16  ksheff
 * Added regular expression routines.
 *
 * Revision 0.3  2002/04/10 09:31:21  ksheff
 * Added function init_stat_buf().
 *
 * Revision 0.2  2002/03/07 07:27:45  ksheff
 * checked for return of ext2fs_unlink in rm_file().
 *
 * Revision 0.1  2002/02/27 04:49:11  ksheff
 * initial revision
 *
 */

/*  Headers */
#include "e2tools.h"
#include <regex.h>

/* Macros */

/* Structures and Unions */
typedef struct
{
    __u16 lmask;
    mode_t mask;
} MODE_XLAT_T;

static MODE_XLAT_T xlat_tbl[] =
{
    { LINUX_S_IFREG, S_IFREG },
    { LINUX_S_IFDIR, S_IFDIR },
    { LINUX_S_IFCHR, S_IFCHR },
    { LINUX_S_IFIFO, S_IFIFO },
    { LINUX_S_ISUID, S_ISUID },
    { LINUX_S_ISGID, S_ISGID },
    { LINUX_S_ISVTX, S_ISVTX },
    { LINUX_S_IRUSR, S_IRUSR },
    { LINUX_S_IWUSR, S_IWUSR },
    { LINUX_S_IXUSR, S_IXUSR },
    { LINUX_S_IRGRP, S_IRGRP },
    { LINUX_S_IWGRP, S_IWGRP },
    { LINUX_S_IXGRP, S_IXGRP },
    { LINUX_S_IROTH, S_IROTH },
    { LINUX_S_IWOTH, S_IWOTH },
    { LINUX_S_IXOTH, S_IXOTH },
    { 0, 0 }
};

/* External Variables */

/* Global Variables */

/* Local Variables */

/* External Prototypes */

/* Local Prototypes */

mode_t
ext2_mode_xlate(__u16 lmode);
__u16
host_mode_xlate(mode_t hmode);
static int
release_blocks_proc(ext2_filsys fs, blk_t *blocknr,
                    int blockcnt, void *private);
long
delete_file(ext2_filsys fs, ext2_ino_t inode);

/* translate a ext2 mode to the host OS representation */
mode_t ext2_mode_xlate(__u16 lmode)
{
  mode_t	mode = 0;
  int	i;

  for (i=0; xlat_tbl[i].lmask; i++)
    {
      if (lmode & xlat_tbl[i].lmask)
        mode |= xlat_tbl[i].mask;
    }
  return mode;
}

/* translate a host OS mode to the ext2 representation */
__u16 host_mode_xlate(mode_t hmode)
{
  __u16	mode = 0;
  int	i;
  
  for (i=0; xlat_tbl[i].lmask; i++)
    {
      if (hmode & xlat_tbl[i].mask)
        mode |= xlat_tbl[i].lmask;
    }
  return mode;
}

long
open_filesystem(char *name, ext2_filsys *fs, ext2_ino_t *root, int rw_mode)
{
  int retval;
  int closeval;
  
  
  if ((retval = ext2fs_open(name, (rw_mode) ? EXT2_FLAG_RW : 0, 0, 0,
                            unix_io_manager, fs)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      *fs = NULL;
      return retval;
    }

  if ((retval = ext2fs_read_inode_bitmap(*fs)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      if ((closeval = ext2fs_close(*fs)))
        fputs(error_message(closeval), stderr);        
      *fs = NULL;
      return retval;
    }

  if ((retval = ext2fs_read_block_bitmap(*fs)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      if ((closeval = ext2fs_close(*fs)))
        fputs(error_message(closeval), stderr);
      *fs = NULL;
      return retval;
    }

  *root = EXT2_ROOT_INO;
  return(0);
}

long
read_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode *inode)
{
  long retval;

  if ((retval = ext2fs_read_inode(fs, file, inode)))
    fprintf(stderr, "%s\n", error_message(retval));
  return retval;
}

long
write_inode(ext2_filsys fs, ext2_ino_t file, struct ext2_inode *inode)
{
  long retval;

  if ((retval = ext2fs_write_inode(fs, file, inode)))
    fprintf(stderr, "%s\n", error_message(retval));
  return retval;

}

long
rm_file(ext2_filsys fs, ext2_ino_t cwd, char *outfile, ext2_ino_t delfile)
{
  struct ext2_inode inode;
  long retval;
  
  if ((retval = read_inode(fs, delfile, &inode)))
    return(retval);

  --inode.i_links_count;
  if ((retval = write_inode(fs, delfile, &inode)))
    return(retval);

  if ((retval = ext2fs_unlink(fs, cwd, outfile, 0, 0)))
      return(retval);
  
  if (inode.i_links_count == 0)
    return(delete_file(fs, delfile));
  
  return(0);
}

long
delete_file(ext2_filsys fs, ext2_ino_t inode)
{
  struct ext2_inode inode_buf;
  long retval;
  
  if ((retval = read_inode(fs, inode, &inode_buf)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return(retval);
    }
  
  inode_buf.i_dtime = time(NULL);
  if ((retval = write_inode(fs, inode, &inode_buf)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return(retval);
    }
  
  if ((retval = ext2fs_block_iterate(fs, inode, 0, NULL,
                                     release_blocks_proc, NULL)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return(retval);
    }
  
  ext2fs_inode_alloc_stats(fs, inode, -1);
  
  return(0);
}

static int
release_blocks_proc(ext2_filsys fs, blk_t *blocknr,
                    int UNUSED_PARM(blockcnt), void UNUSED_PARM(*private))
{
	blk_t	block;

	block = *blocknr;
	ext2fs_block_alloc_stats(fs, block, -1);
	return 0;
}

void
init_stat_buf(struct stat *buf)
{
  if (buf)
    {
      memset(buf, 0, sizeof(struct stat));
      buf->st_atime = buf->st_ctime = buf->st_mtime = time(NULL);
      buf->st_uid = getuid();
      buf->st_gid = getgid();
    }
}

int
is_file_regexp(char *ptr)
{
  char c;

  while ((c = *ptr++) != '\0')
    {
      switch(c)
        {
        case '*':
        case '[':
        case ']':
        case '?':
          return(1);
          break;
        }
    }
  return(0);
}

regex_t *
make_regexp(char *shell)
{
  char *tmpstr;
  char *ptr;
  static regex_t reg;
  char c;

  if (NULL == (tmpstr = alloca(((strlen(shell)) << 1) + 3)))
    {
      perror("make_regexp");
      return(NULL);
    }

  ptr = tmpstr;
  *ptr++ = '^';
  
  while ((c = *shell++) != '\0')
    {
      switch(c)
        {
        case '*':
          *ptr++ = '.';
          *ptr++ = c;
          break;
        case '?':
          *ptr++ = '.';
          break;
        case '.':
          *ptr++ = '\\';
          *ptr++ = '.';
          break;
        default:
          *ptr++ = c;
        }
    }
  *ptr++ = '$';
  *ptr = '\0';
  
  if (regcomp(&reg, tmpstr, REG_NOSUB))
    {
      perror("make_regexp");
      return(NULL);
    }

  return(&reg);
}
