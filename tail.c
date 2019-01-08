/* $Header: /home/ksheff/src/e2tools/RCS/tail.c,v 0.2 2003/07/12 16:33:11 ksheff Exp $ */
/*
 * tail.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 */
#ifndef TAIL_C
#define TAIL_C
#endif

/* Description */
/* This module implements a basic version of the tail command.
 *
 * The user can specify the number of lines to view from the bottom of the
 * file.  This is done by specifying -n #of_lines on the command line.	The
 * default is 5.
 *
 * The user can also run in 'follow' mode which prints new lines as they are
 * appended to the end of the file.	 This can be specified by the -f option,
 * which is dependent on the initial inode of the file being tailed or the -F
 * option which will always use the inode associated with the file name.  The
 * latter can be useful for log files that get switched out.
 *
 * The -s #seconds option allows the user to specify the sleep interval while
 * in follow mode.	The default is 1.
 *
 */
/*
 * $Log: tail.c,v $
 * Revision 0.2  2003/07/12 16:33:11  ksheff
 * fixed a bug when no arguments are given.
 *
 * Revision 0.1  2002/08/08 08:01:51  ksheff
 * Initial revision.
 *
 */

/* Feature Test Switches */
/*  Headers */

#include "e2tools.h"


/* Macros */
#define USAGE "Usage: e2tail [-n num_lines][-fF][-s sleep_interval] file\n"
#define BLK_SIZE 4096

#define FOLLOW_INODE 1
#define FOLLOW_NAME 2

/* Structures and Unions */

/* External Variables */

/* Global Variables */

/* Local Variables */

/* External Prototypes */

/* Local Prototypes */
long
do_tail(int argc, char *argv[]);
static long
tail(ext2_filsys *fs, ext2_ino_t root, char *input, int num_lines,
     int follow, int sleep_int, char *cur_filesys);


/* Name:	do_tail()
 *
 * Description:
 *
 * This function reads the command line arguments and displays the last lines
 * at the end of a file in an ext2 file system.
 *
 * Algorithm:
 *
 * Read any command line switches
 * Get the file specification for the file we are going to display.
 * Open the file system read only
 * Tail the file
 * Close the file system
 * return the status of the tail
 *	  
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * int argc;			 The number of arguments
 * char *argv[];		 The command line arguments
 *
 * Return Values:
 *
 * 0 - the last number of lines was displayed correctly.
 * an error occurred.
 *
 * Author: Keith W. Sheffield
 * Date:   08/07/2002
 *
 * Modification History:
 *
 * MM/DD/YY		 Name				Description
 * 07/12/03      K.Sheffield        fixed a bug when no arguments are given.
 */
long
do_tail(int argc, char *argv[])
{
  /* UNUSED int verbose=0; */
  int follow=0;
  int num_lines = 5;
  int sleep_int = 1;
  int errcnt=0;
  char *cur_filesys = NULL;
  ext2_filsys fs = NULL;
  ext2_ino_t root;
  long retval;
  int curidx;
  char *tail_dir;
  int c;
  
  
#ifdef HAVE_OPTRESET
  optreset = 1;		/* Makes BSD getopt happy */
#endif
  while ((c = getopt(argc, argv, "vFfn:s:")) != EOF)
    {
      switch (c)
        {
        case 'v':
          /* UNUSED verbose = 1; */
          break;
        case 'f':
          follow = FOLLOW_INODE;
          break;
        case 'F':
          follow = FOLLOW_NAME;
          break;
        case 'n':
          num_lines = atoi(optarg);
          break;
        case 's':
          sleep_int = atoi(optarg);
          if (sleep_int < 1)
            errcnt++;
          break;
        default:
          errcnt++;
          break;
        }
    }

  curidx = optind;
  
  if (errcnt || argc <= curidx)
    {
      fputs(USAGE, stderr);
      return(1);
    }

  cur_filesys = argv[curidx++];
  if (NULL == (tail_dir = strchr(cur_filesys, ':')))
    {
      fprintf(stderr, "Invalid file specification: %s\n", cur_filesys);
      return(1);
    }
  *tail_dir++ = '\0';
  
  if ((retval = open_filesystem(cur_filesys, &fs, &root, 0)))
    {
      fprintf(stderr, "%s: %s\n", error_message(retval), cur_filesys);
      return retval;
    }

  retval = tail(&fs, root, tail_dir, num_lines, follow, sleep_int,
                cur_filesys) ? -1 : 0;
  ext2fs_close(fs);
  return(retval);
}

/* Name:	tail()
 *
 * Description:
 *
 * This function displays the last lines at the end of a file in an ext2
 * file system.
 *
 * Algorithm:
 *
 * Get the directory and basename of the file
 * Determine the inode number for the file
 * Open the file for reading
 * Skip to the last block in the file
 * While we have not found the last num_lines of newline characters
 *	  Skip backwards in the file one block and read it
 * Display the contents of the block from that point on.
 * Display the rest of the file if not contained in the block
 * Save the current location of the file.
 * If we are following the file as it grows
 *	  While forever
 *		  Sleep
 *		  Re-read the inode for the file
 *		  If the size has changed
 *			  Display the file from the saved point on
 *            Save the current location of the file.
 *	  
 *	  
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * ext2_filsys *fs;             Our filesystem
 * ext2_ino_t root;             The root directory inode number
 * char *input;                 The name of the input file to tail
 * int num_lines;               The number of lines to display
 * int follow;                  Flag indicating if the we should follow any
 *                              new contents to the file.
 * int sleep_int;               The number of seconds to sleep between checking
 *                              for new lines
 * char *cur_filesys
 *
 * Return Values:
 *
 * 0 - the last number of lines was displayed correctly.
 * an error occurred.
 *
 * Author: Keith W. Sheffield
 * Date:   08/07/2002
 *
 * Modification History:
 *
 * MM/DD/YY		 Name				Description
 */
static long
tail(ext2_filsys *fs_ptr, ext2_ino_t root, char *input, int num_lines,
     int follow, int sleep_int, char *cur_filesys)
{
  ext2_filsys fs = *fs_ptr;
  ext2_ino_t cwd;
  ext2_ino_t tail_ino;
  ext2_ino_t t_tail_ino;
  char *tail_dir;
  char *tail_name;
  long retval;
  char buf[BLK_SIZE];
  unsigned int bytes_to_read;
  unsigned int bytes_read;
  char *ptr;
  struct ext2_inode inode;
  ext2_file_t tail_fd;    
  ext2_off_t offset;
  ext2_off_t cur_pos;

  if (get_file_parts(fs, root, input, &cwd, &tail_dir, &tail_name))
    {
      ext2fs_close(fs);
      return(-1);
    }          

  /* get the inode number for the source file */
  if ((retval = ext2fs_namei(fs, cwd, cwd, tail_name, &tail_ino)))
    {
      fprintf(stderr, "%s: file %s\n",error_message(retval),
              tail_name);
      return(retval);
    }

  /* open the file */
  if ((retval = ext2fs_file_open(fs, tail_ino, 0, &tail_fd)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  /* get the length of the file and determine where to start reading */
  inode.i_size = offset = ext2fs_file_get_size(tail_fd);
  bytes_to_read = offset % BLK_SIZE;
  if (bytes_to_read == 0)
    bytes_to_read = BLK_SIZE;

  offset -= bytes_to_read;
  if (((int32_t)offset) < 0)
    offset = 0;

  do
    {
      /* seek to the start of the last block in the file */
      if ((retval = ext2fs_file_lseek(tail_fd, offset, EXT2_SEEK_SET, NULL)))
        {
          fputs(error_message(retval), stderr);
          return retval;
        }
      /* read the last block in the file */
      if ((retval = ext2fs_file_read(tail_fd, buf, bytes_to_read,
                                     &bytes_read)))
        {
          fputs(error_message(retval), stderr);
          return retval;
        }
      if (bytes_to_read != bytes_read)
        {
          fputs("error reading file\n", stderr);
          return(-1);
        }
      
      ptr = buf + bytes_read - 1;
      while (bytes_to_read--)
        {
          if (*ptr == '\n' && num_lines-- == 0)
            {
              /* if the newline wasn't the last character in the buffer, then
               * print what's remaining.
               */
              if (bytes_to_read != bytes_read - 1)
                {
                  ptr++;
		  if (0 > write(1, ptr, bytes_read - bytes_to_read - 1))
		    {
		      perror("writing bytes to stdout");
		      return -1;
		    }
                }
              offset = 0;       /* make sure we break out of the main loop */
              break;
            }
          ptr--;
        }

      offset -= (offset < BLK_SIZE) ? offset : BLK_SIZE;
      bytes_to_read = BLK_SIZE;
    }
  while (offset > 0);

  /* if we are here and have any lines left, we hit the beginning, so
   * dump the rest of what's in memory out.
   */
  
  if (num_lines > 0)
    {
      if (0 > write(1, buf, bytes_read)) {
	perror("writing bytes to stdout");
	return -1;
      }
    }
    
  /* retreive the current position in the file */
  if ((retval = ext2fs_file_lseek(tail_fd, 0, EXT2_SEEK_CUR, &cur_pos)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  /* ok, if we are before the end of the file, then dump the rest of it */
  if (cur_pos < inode.i_size)
    {
      if ((retval = read_to_eof(tail_fd, 1, cur_pos, &cur_pos)))
        {
          return retval;
        }
    }

  if ((retval = ext2fs_file_close(tail_fd)))
    {
      fputs(error_message(retval), stderr);
      return retval;
    }

  if (follow)
    {
      while(1)
        {
          sleep(sleep_int);
          /* I don't know how to force a re-read of the file system info yet,
           * so, just close the file system and reopen it.
           */
          ext2fs_close(fs);
          if ((retval = open_filesystem(cur_filesys, &fs, &root, 0)))
            {
              *fs_ptr = NULL;
              fprintf(stderr, "%s: %s\n", error_message(retval), cur_filesys);
              return retval;
            }
          *fs_ptr = fs;

          /* if we are following the name, find the directory and file name
           * again.
           */
          if (follow == FOLLOW_NAME)
            {
              cwd = root;
              
              if (tail_dir != NULL && *tail_dir != '\0' &&
                  strcmp(tail_dir, ",") != 0 &&
                  (retval = change_cwd(fs, root, &cwd, tail_dir)))
                {
                  fprintf(stderr, "Error changing to directory %s\n",
                          tail_dir);
                  return(retval);
                }

              /* get the inode number for the source file */
              if ((retval = ext2fs_namei(fs, cwd, cwd, tail_name,
                                         &t_tail_ino)))
                {
                  fprintf(stderr, "%s: file %s\n",error_message(retval),
                          tail_name);
                  return(retval);
                }

              /* if we are dealing with a new file, then start from the
               * beginning.
               */
              
              if (t_tail_ino != tail_ino)
                {
                  tail_ino = t_tail_ino;
                  cur_pos = 0;
                }
            }
          
          if ((retval = ext2fs_read_inode(fs, tail_ino, &inode)))
            {
              fputs(error_message(retval), stderr);
              return retval;
            }
          if (inode.i_size > cur_pos)
            {
              if ((retval = retrieve_data(fs, tail_ino, 1, NULL, 0, cur_pos,
                                          &cur_pos)))
                {
                  fputs(error_message(retval), stderr);
                  return retval;
                }
            }
          else if (inode.i_size < cur_pos)
            {
              /* the file was truncated, so bail */
              return(0);
            }          
        }
    }
  return(0);
}
