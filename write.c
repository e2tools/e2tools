/* $Header: /home/ksheff/src/e2tools/RCS/write.c,v 0.5 2004/04/07 01:12:30 ksheff Exp $ */
/*
 * write.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Derived from debugfs.c Copyright (C) 1993 Theodore Ts'o <tytso@mit.edu>
 * and modified by Robert Sanders <gt8134b@prism.gatech.edu>
 */

/* Description */
/* This file contains the functions used to write a file to an ext2fs
 * filesystem.
 *
 */
/*
 * $Log: write.c,v $
 * Revision 0.5  2004/04/07 01:12:30  ksheff
 * Modified to use a default file stat specified by the user.
 *
 * Revision 0.4  2002/07/08 11:16:11  ksheff
 * Additional error messages after perror().
 *
 * Revision 0.3  2002/06/26 11:12:48  ksheff
 * Modified to call update_progress() so that the user can know the state of
 * the file copy operation.  The put_file function also calls ext2fs_flush()
 * before exiting now.
 *
 * Revision 0.2  2002/03/07 07:15:40  ksheff
 * Added parameter to put_file so that the original file's owner, group,
 * permission bits, access, modification, and creation times would be used.
 *
 * Revision 0.1  2002/02/27 04:49:32  ksheff
 * initial revision
 *
 */

/* Headers */

#include <errno.h>
#include "e2tools.h"

/* External Prototypes */

extern void init_progress(char *file, struct stat *sbuf);
extern void update_progress(unsigned long num_bytes);
extern void finish_progress();

/* Local Prototypes */

long
put_file(ext2_filsys fs, ext2_ino_t cwd, char *infile, char *outfile,
         ext2_ino_t *outfile_ino, int keep, struct stat *def_stat);

static long
store_data(ext2_filsys fs, int fd, ext2_ino_t newfile, off_t *file_size); 

/* Name:	put_file()
 *
 * Description:
 *
 * This function copies a file from disk or stdin to the current directory
 *
 * Algorithm:
 *
 * Check input parameters
 * If the input file is NULL
 *     Get the file descriptor for stdin
 *     Initialize a file stat structure
 * Otherwise
 *     Open the input file
 *     Get the file stat structure for the file
 * Get a new inode for the file
 * Link the inode to the currect directory
 * If this fails because there isn't enough room in the directory
 *     Expand the directory and try again
 * Set the file statistics for the current inode
 * If the file is a regular file
 *     Copy it to the current inode
 * Close the input file
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * ext2_filsys fs;            The current file system
 * ext2_ino_t cwd;            The current working directory
 * char *infile;              The name of the input file
 * char *outfile;             The name of the output file
 * int keep;                  Flag indicating to use the input file's stat info
 * struct stat *def_stat;     The default file stat information
 *
 * Return Values:
 *
 * 0 - the file was copied successfully
 * any other value indicates an error
 *
 * Author: Keith W. Sheffield
 * Date:   02/17/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 02/27/02      K.Sheffield        Added parameter to pass back to caller the
 *                                  inode number of the output file.
 * 03/05/02      K.Sheffield        Fixed a bug with reading from stdin
 * 03/06/02      K.Sheffield        Added time/owner/group keep flag
 * 06/26/02      K.Sheffield        Flush file system added at the end
 * 07/08/02      K.Sheffield        Additional error messages after perror()
 * 04/06/04      K.Sheffield        Added a default file stat parameter
 */
long
put_file(ext2_filsys fs, ext2_ino_t cwd, char *infile, char *outfile,
         ext2_ino_t *outfile_ino, int keep, struct stat *def_stat)
{
  int fd;
  struct stat	statbuf;
  ext2_ino_t newfile;
  long  retval;
  struct ext2_inode inode;
  mode_t cur_umask;

  if (fs == NULL || outfile == NULL)
    {
      fputs("Invalid input parameter.  Exiting put_file() with -1\n",
            stderr);
      return (-1);
    }

  if (infile == NULL)
    {
      fd = fileno(stdin);
      memset(&statbuf, 0, sizeof(statbuf));
    }
  else
    {
      if (0 > (fd = open(infile, O_RDONLY)))
        {
          perror(infile);
          fprintf(stderr, "Error opening input file: %s\n", infile);
          return(-1);
        }
      if (0 > fstat(fd, &statbuf))
        {
          perror(infile);
          fprintf(stderr, "Error stat()'ing input file: %s\n", infile);
          close(fd);
          return(-1);
        }
    }

  if (keep == 0 || infile == NULL)
    {
      statbuf.st_atime = statbuf.st_ctime = statbuf.st_mtime = time(NULL);
      umask(cur_umask = umask(0)); /* get the current umask */
      if (def_stat != NULL)
        {
          statbuf.st_mode = S_IFREG |
            ((def_stat->st_mode == 0) ? (0666 & ~cur_umask):def_stat->st_mode);
          statbuf.st_uid = def_stat->st_uid;
          statbuf.st_gid = def_stat->st_gid;
        }
      else
        {
          statbuf.st_mode = S_IFREG | (0666 & ~cur_umask);
          statbuf.st_uid = getuid();
          statbuf.st_gid = getgid();
        }
    }
  
  if ((retval = ext2fs_namei(fs, cwd, cwd, outfile, &newfile)))
    {
      if (retval != EXT2_ET_FILE_NOT_FOUND)
        {
          fprintf(stderr, "%s\n",error_message(retval));
          close(fd);
          return(retval);
        }

    }
  /* file name exists, let's see if is a directory */
  else if ((retval = ext2fs_check_directory(fs, newfile)))
    {
      if (retval != EXT2_ET_NO_DIRECTORY ||
          (retval = rm_file(fs, cwd, outfile, newfile)))
        {
          fprintf(stderr, "%s\n",error_message(retval));
          close(fd);
          return(retval);
        }
    }
  else
    {
    /* if we get here, then it's an existing directory */
      fprintf(stderr, "%s is a directory!\n", outfile);
      return(1);
    }

  /* ok, create a new inode and directory entry */
  if ((retval = ext2fs_new_inode(fs, cwd, 010755, 0, &newfile)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      close(fd);
      return retval;
    }
  
  if ((retval = ext2fs_link(fs, cwd, outfile, newfile, EXT2_FT_REG_FILE)))
    {
      /* check to see if we ran out of space in the directory */
      if (retval == EXT2_ET_DIR_NO_SPACE)
        {
          /* try resizing the directory and try again */
          if (0 == (retval = ext2fs_expand_dir(fs, cwd)))
            retval = ext2fs_link(fs, cwd, outfile, newfile,
                                 EXT2_FT_REG_FILE);
        }
      if (retval)
        {
          fprintf(stderr, "%s\n", error_message(retval));
          close(fd);
          return retval;
        }
    }
  ext2fs_inode_alloc_stats(fs, newfile, +1);
  memset(&inode, 0, sizeof(inode));
  inode.i_mode = host_mode_xlate(statbuf.st_mode);
  inode.i_atime = statbuf.st_atime;
  inode.i_ctime = statbuf.st_ctime;
  inode.i_mtime = statbuf.st_mtime;
  inode.i_links_count = 1;
  inode.i_size = statbuf.st_size;
  inode.i_uid = statbuf.st_uid;
  inode.i_gid = statbuf.st_gid;
    
  if ((retval = write_inode(fs, newfile, &inode)))
    {
      close(fd);
      return (retval);
    }

  if (LINUX_S_ISREG(inode.i_mode) &&
      (retval = store_data(fs, fd, newfile, &statbuf.st_size)))
    {
      close(fd);
#ifndef DEBUG
      rm_file(fs, cwd, outfile, newfile);
      
#endif
      return(retval);
    }

  close(fd);

  /* if we were reading from standard input, figure out the size of
   * the file and save it.
   */
    
  if (infile == NULL)
    {
      if ((retval = read_inode(fs, newfile, &inode)))
        return(retval);

      inode.i_size = statbuf.st_size;
        
      if ((retval = write_inode(fs, newfile, &inode)))
        return(retval);
    }

  /* save the files inode number for later use */
  if (outfile_ino != NULL)
    *outfile_ino = newfile;
  
  return(ext2fs_flush(fs));
    
} /* end of put_file */ 

/* Name:	store_data()
 *
 * Description:
 *
 * This function stores the contents of a file descriptor into the current ext2
 * file system 
 *
 * Algorithm:
 *
 * Open a new file in the ext2 file system
 * While data can be read from the input file descriptor
 *	   Write the data to the file on the ext2 filesystem
 * Close the file
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * ext2_filsys fs;			  The current file system
 * int fd;					  Input file descriptor
 * ext2_ino_t newfile;		  Inode number of the new file
 * off_t *file_size;		  The size of the file written
 *
 * Return Values:
 *
 * 0 - file copied successfully
 * otherwise the error code of what went wrong
 *
 * Author: Keith W. Sheffield
 * Date:   02/18/2002
 *
 * Modification History:
 *
 * MM/DD/YY		 Name				Description
 * 06/26/02      K.Sheffield        Added a call to update_progress()
 */
static long
store_data(ext2_filsys fs, int fd, ext2_ino_t newfile, off_t *file_size)
{
  ext2_file_t	outfile;
  long retval;
  int bytes_read;
  unsigned int bytes_written;
  char buf[8192];
  char *ptr;
  off_t total = 0;

  if ((retval = ext2fs_file_open(fs, newfile, EXT2_FILE_WRITE, &outfile)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      ext2fs_file_close(outfile);
      *file_size = 0;
      return retval;
    }

  while (0 < (bytes_read = read(fd, buf, sizeof(buf))))
    {
      ptr = buf;
      while (bytes_read > 0)
        {
          if ((retval = ext2fs_file_write(outfile, ptr, bytes_read,
                                          &bytes_written)))
            {
              fprintf(stderr, "%s\n", error_message(retval));
              ext2fs_file_close(outfile);
              *file_size = total;
              return retval;
            }
          bytes_read -= bytes_written;
          total += bytes_written;
          ptr += bytes_written;
        }
      update_progress((unsigned long) total);
    }

  if (bytes_read < 0)
    {
      perror("store_data");
      retval = errno;
    }
  else
    retval = 0;
    
  finish_progress();
  
  ext2fs_file_close(outfile);
  *file_size = total;
  return retval;

} /* end of store_data */ 

