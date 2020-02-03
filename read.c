/* $Header: /home/ksheff/src/e2tools/RCS/read.c,v 0.2 2002/08/08 07:58:35 ksheff Exp $ */
/*
 * read.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Derived from dump.c Copyright (C) 1994 Theodore Ts'o <tytso@mit.edu>
 *
 */

static char __attribute__ ((used)) copyright[] = "Copyright 2002 Keith W Sheffield";


/* Description */
/*
 *
 *
 */
/*
 * $Log: read.c,v $
 * Revision 0.2  2002/08/08 07:58:35  ksheff
 * Split the read loop from retrieve_data() out to its own function:
 * read_to_eof() and made retrieve_data() able to seek to an offset before
 * starting to read.
 *
 * Revision 0.1  2002/02/27 04:48:52  ksheff
 * initial revision
 *
 */

/* Feature Test Switches */
/*  Headers */
#include "e2tools.h"
#include "read.h"

/* Local Prototypes */
static void
fix_perms(const struct ext2_inode *inode, int fd, const char *name);

/* Name:    get_file()
 *
 * Description:
 *
 * This function copies a file from the current ext2fs directory to disk
 * or stdout.
 *
 * Algorithm:
 *
 * Check input parameters
 * If the output file is NULL
 *     Get the file descriptor for stdout
 * Otherwise
 *     Open the output file
 * Get the inode number for the input file
 * Copy the contents to the output file
 * Close output file
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * ext2_filsys fs;       The current file system
 * ext2_ino_t root;      The root directory
 * ext2_ino_t cwd;       The current working directory
 * char *infile;         The name of the input file
 * char *outfile;        The name of the output file
 * int keep;             Flag indicating if the permissions should be kept
 *
 * Return Values:
 *
 * 0 - the file was copied successfully
 * the error code of whatever went wrong.
 *
 * Author: Keith W. Sheffield
 * Date:   02/20/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */

long
get_file(ext2_filsys fs, ext2_ino_t root, ext2_ino_t cwd,
         char *infile, char *outfile, int keep)
{
  ext2_ino_t src;
  int dest;
  int retval;

  if (fs == NULL || infile == NULL)
    {
      fputs("Invalid input parameter.  Exiting get_file() with -1\n",
            stderr);
      return (-1);
    }

  /* open the output file if we need to */
  if (outfile == NULL)
    dest = fileno(stdout);
  else if (-1 == (dest = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0666)))
    {
      perror(outfile);
      return(-1);
    }

  /* get the inode number associated with the input file */
  if ((retval = ext2fs_namei(fs, root, cwd, infile, &src)))
    {
      if (retval == EXT2_ET_FILE_NOT_FOUND)
        {
          fprintf(stderr, "%s not found\n", infile);
          if (outfile)
            {
              close(dest);
#ifndef DEBUG
              unlink(outfile);
#endif
            }
          return(0);
        }
      fputs(error_message(retval), stderr);
      return retval;
    }

  /* get the data from the ext2fs */
  if ((retval = retrieve_data(fs, src, dest, outfile, keep, 0, NULL)))
    {
      if (outfile)
        {
          close(dest);
#ifndef DEBUG
          unlink(outfile);
#endif
        }
      return(retval);
    }
  if (outfile)
    close(dest);
  return(0);

} /* end of get_file */


/* Name:    retrieve_data()
 *
 * Description:
 *
 * This function retrieves the data stored as a file in an ext2fs file
 * system.
 *
 * Algorithm:
 *
 * Get the inode associated with the file name
 * Open the file in the ext2fs file system
 * Copy the contents of the ext2fs file to the output file descriptor
 * Close the ext2fs file
 * If the permissions are to be kept
 *     Call fix_perms
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * ext2_filsys fs;            The filesystem being read from
 * ext2_ino_t src;            The source inode number
 * int dest_fd;               The destination file descriptor
 * char *dest_name;           The name of the destination file
 * int keep;                  Keep the permissions/ownership, etc. from ext2fs
 *
 * Return Values:
 *
 * 0 - file read successfully
 * error code of what went wrong.
 *
 * Author: Keith W. Sheffield
 * Date:   02/20/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 08/07/02      K.Sheffield        Moved the copy loop to read_to_eof()
 *
 */

long
retrieve_data(ext2_filsys fs, ext2_ino_t src, int dest_fd,
              char *dest_name, int keep, ext2_off_t offset,
              ext2_off_t *ret_pos)
{
  struct ext2_inode inode;
  ext2_file_t infile;
  int retval;

  if (keep && (retval = ext2fs_read_inode(fs, src, &inode)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  if ((retval = ext2fs_file_open(fs, src, 0, &infile)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  if (read_to_eof(infile, dest_fd, offset, ret_pos))
    {
      if ((retval = ext2fs_file_close(infile)))
        fputs(error_message(retval), stderr);
      return(-1);
    }

  if ((retval = ext2fs_file_close(infile)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  if (keep)
    fix_perms(&inode, dest_fd, dest_name);

  return(0);

} /* end of retrieve_data */

/* Name:    read_to_eof()
 *
 * Description:
 *
 * This function reads from an ext2_file_t file and outputs the contents
 * to a standard file descriptor.
 *
 * Algorithm:
 *
 * Seek to the desired location
 * While one is able to read data from the ext2fs file
 *     Write data to the output file descriptor.
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * ext2_file_t infile;        Input file
 * int dest_fd;               The destination file descriptor
 * ext2_off_t offset;         The offset in the file to seek
 * ext2_off_t *ret_pos;       The returned file position
 *
 * Return Values:
 *
 *
 * Author: Keith W. Sheffield
 * Date:   08/07/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
long
read_to_eof(ext2_file_t infile, int UNUSED_PARM(dest_fd), ext2_off_t offset,
            ext2_off_t *ret_pos)
{
  char buf[4096];
  unsigned int bytes_read;
  int retval;

  if (offset != 0 &&
      (retval = ext2fs_file_lseek(infile, offset, EXT2_SEEK_SET, NULL)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  /* read all that we can and dump it to the output file descriptor */
  while (1)
    {
      ssize_t bytes_written;
      if ((retval = ext2fs_file_read(infile, buf, sizeof(buf), &bytes_read)))
        {
          fputs(error_message(retval), stderr);
          return retval;
        }
      if (bytes_read <= 0) break;
      bytes_written = write(dest_fd, buf, bytes_read);
      if (bytes_written < 0) break;
      if (bytes_read != (size_t)bytes_written) break;
    }

  if (bytes_read != 0)
    {
      perror("read_to_eof");
      return(-1);
    }

  if (ret_pos != NULL &&
      (retval = ext2fs_file_lseek(infile, 0, EXT2_SEEK_CUR, ret_pos)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  return(0);

} /* end of read_to_eof */

/* blatantly copied from code written by T Ts'o w/ minor changes */
static void
fix_perms(const struct ext2_inode *inode, int fd, const char *name)
{
  struct utimbuf ut;
  int i;

  if (fd != -1)
    i = fchmod(fd, ext2_mode_xlate(inode->i_mode));
  else
    i = chmod(name, ext2_mode_xlate(inode->i_mode));
  if (i == -1)
    perror(name);

#ifndef HAVE_FCHOWN
  i = chown(name, inode->i_uid, inode->i_gid);
#else
  if (fd != -1)
    i = fchown(fd, inode->i_uid, inode->i_gid);
  else
    i = chown(name, inode->i_uid, inode->i_gid);
#endif
  if (i == -1)
    perror(name);

  if (fd != -1)
    close(fd);

  ut.actime = inode->i_atime;
  ut.modtime = inode->i_mtime;
  if (utime(name, &ut) == -1)
    perror(name);

}



