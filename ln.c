/* $Header: /home/ksheff/src/e2tools/RCS/ln.c,v 0.3 2002/06/30 05:54:02 ksheff Exp $ */
/*
 * ln.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 */

/* Description */
/*
 * Module to create links
 *
 */
/*
 * $Log: ln.c,v $
 * Revision 0.3  2002/06/30 05:54:02  ksheff
 * Modified create_hard_link() to check file type before linking to current
 * directory.  This was not being done before and it was causing problems if a
 * non regular file was renamed.
 *
 * Revision 0.2  2002/03/21 09:04:40  ksheff
 * Split the mv operation into its own module.
 *
 * Revision 0.1  2002/03/07 07:23:41  ksheff
 * initial revision.
 *
 */

/* Feature Test Switches */
/*  Headers */
#include "e2tools.h"
#include "ln.h"

/* Macros */
#define USAGE "Usage: e2ln [-vfs] source destination\n"

/* Name:	do_ln()
 *
 * Description:
 *
 * This function reads the command line arguments and creates a link
 * in an ext2fs file system
 *
 * Algorithm:
 *
 * Read any command line switches
 * Get the source file specification
 * Open the file system
 * Get the directory and basename of the source file
 * Determine the inode number for the source file
 * If the destination file is not given or if it's a .
 *     use the current directory and the basename of the source file
 * Otherwise
 *     Get the directory and basename of the destination file
 * Create the link
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * int argc;             The number of arguments
 * char *argv[];         The command line arguments
 *
 * Return Values:
 *
 * 0 - the link was created successfully
 * an error occurred.
 *
 * Author: Keith W. Sheffield
 * Date:   03/05/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 03/06/02      K. Sheffield       Modified to perform file moves
 * 03/20/02      K. Sheffield       Moved the mv operation to a separate file
 */
long
do_ln(int argc, char *argv[])
{
  int verbose=0;
  int force=0;
  int symlink=0;
  int errcnt=0;
  char *cur_filesys = NULL;
  ext2_filsys fs = NULL;
  ext2_ino_t root;
  ext2_ino_t srcd;
  ext2_ino_t destd;
  ext2_ino_t source_file;
  char *src_dir;
  char *dest_dir;
  char *src_name;
  char *dest_name;
  long retval;
  int c;

#ifdef HAVE_OPTRESET
  optreset = 1;		/* Makes BSD getopt happy */
#endif
  while ((c = getopt(argc, argv, "vfs")) != EOF)
    {
      switch (c)
        {
        case 'v':
          verbose = 1;
          break;
        case 'f':
          force = E2T_FORCE;
          break;
        case 's':
          symlink = 1;
          break;
        default:
          errcnt++;
          break;
        }
    }

  if (errcnt || argc == optind)
    {
      fputs(USAGE, stderr);
      return(1);
    }

  if (symlink)
    {
      fputs("Not implemented yet\n", stderr);
      return(1);
    }

  cur_filesys = argv[optind++];
  if (NULL == (src_dir = strchr(cur_filesys, ':')))
    {
      fprintf(stderr, "Invalid file specification: %s\n", cur_filesys);
      return(1);
    }
  *src_dir++ = '\0';

  if (*src_dir == '\0')
    {
      fputs(USAGE, stderr);
      return(1);
    }

  if ((retval = open_filesystem(cur_filesys, &fs, &root, 1)))
    {
      fprintf(stderr, "%s: %s\n", error_message(retval), cur_filesys);
      return retval;
    }

  /* move to the source directory */

  if (get_file_parts(fs, root, src_dir, &srcd, &src_dir, &src_name))
    {
      ext2fs_close(fs);
      return(-1);
    }

  /* get the inode number for the source file */
  if ((retval = ext2fs_namei(fs, srcd, srcd, src_name, &source_file)))
    {
      fprintf(stderr, "%s: source file %s\n",error_message(retval), src_name);
      ext2fs_close(fs);
      return(retval);
    }

  /* get the destination directory */
  destd = root;
  if (argc == optind || strcmp(dest_dir = argv[optind], ".") == 0)
    dest_name = src_name;
  else
    {
      if (get_file_parts(fs, root, dest_dir, &destd, &dest_dir,
                         &dest_name))
        {
          ext2fs_close(fs);
          return(-1);
        }
    }

  /* now create the link */
  if ((retval = create_hard_link(fs, destd, source_file, dest_name, force)))
    {
      fprintf(stderr, "Error linking %s/%s as %s/%s\n",
              ((src_dir == NULL) ? "." : src_dir), src_name,
              ((dest_dir == NULL) ? "." : dest_dir), dest_name);
      ext2fs_close(fs);
      return(1);
    }

  if (verbose)
    fprintf(stderr, "linked %s/%s as %s/%s\n",
            ((src_dir == NULL) ? "." : src_dir), src_name,
            ((dest_dir == NULL) ? "." : dest_dir), dest_name);

  ext2fs_close(fs);
  return(0);

} /* end of do_ln */

/* Name:	create_hard_link()
 *
 * Description:
 *
 * This function creates a hard link to an existing file
 *
 * Algorithm:
 *
 * Check input parameters
 * Check to see if the new file name already exists
 * Make sure the new file is not an existing directory
 * If the file exists, remove it if the del_current flag is set
 * Add the new file name and it's inode to the current directory.
 * Get the inode structure for the current inode number
 * update the number of links
 * Write the inode structure back out to the file system.
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * ext2_filsys fs;			  The current file system
 * ext2_ino_t cwd;			  The current working directory
 * ext2_ino_t new_file_ino;	  The inode number of the new file
 * char *newfile;			  The name of the new file
 * int ln_flags;			  Flags affecting hard_link action
 *
 * Return Values:
 *
 * 0 - the new file link was created successfully
 * any other value indicates an error
 *
 * Author: Keith W. Sheffield
 * Date:   03/05/2002
 *
 * Modification History:
 *
 * MM/DD/YY		 Name				Description
 * 06/30/02		 K.Sheffield		Directory link flag is now based on the
 *									type of file being linked.	This was
 *									causing problems if a directory was
 *									renamed.
 */
long
create_hard_link(ext2_filsys fs, ext2_ino_t cwd, ext2_ino_t new_file_ino,
                 char *newfile, int ln_flags)
{
  ext2_ino_t curr_ino;
  struct ext2_inode inode;
  long retval;
  int dir_flag;

  if (fs == NULL || newfile == NULL)
    {
      fputs("Invalid input parameter.  Exiting create_hard_link() with -1\n",
            stderr);
      return (-1);
    }

  /* check to see if the file name already exists in the current directory */
  if ((retval = ext2fs_namei(fs, cwd, cwd, newfile, &curr_ino)))
    {
      if (retval != EXT2_ET_FILE_NOT_FOUND)
        {
          fprintf(stderr, "%s\n",error_message(retval));
          return(retval);
        }

    }
  /* file name exists, let's see if is a directory */
  else if ((retval = ext2fs_check_directory(fs, curr_ino)))
    {
      if (retval != EXT2_ET_NO_DIRECTORY)
        {
          fprintf(stderr, "%s\n",error_message(retval));
          return(retval);
        }

      /* delete the existing file if needed */
      if ((ln_flags & E2T_FORCE) &&
          (curr_ino != new_file_ino))
        {
          if ((retval = rm_file(fs, cwd, newfile, curr_ino)))
            {
              fprintf(stderr, "%s\n",error_message(retval));
              return(retval);
            }
        }
      else
        {
          fprintf(stderr, "ln: %s: File exists\n", newfile);
          return(1);
        }
    }
  else
    {
    /* if we get here, then it's an existing directory */
      fprintf(stderr, "%s is a directory!\n", newfile);
      return(1);
    }

  /* read the inode associated with the file */
  if ((retval = read_inode(fs, new_file_ino, &inode)))
      {
      fprintf(stderr, "%s\n", error_message(retval));
      return (retval);
      }

  /* determine how to link into the directory based on the type of file */
  switch(inode.i_mode & LINUX_S_IFMT)
    {
    case LINUX_S_IFREG:
      dir_flag = EXT2_FT_REG_FILE;
      break;
    case LINUX_S_IFLNK:
      dir_flag = EXT2_FT_SYMLINK;
      break;
    case LINUX_S_IFDIR:
      dir_flag = EXT2_FT_DIR;
      break;
    case LINUX_S_IFSOCK:
      dir_flag = EXT2_FT_SOCK;
      break;
    case LINUX_S_IFBLK:
      dir_flag = EXT2_FT_BLKDEV;
      break;
    case LINUX_S_IFCHR:
      dir_flag = EXT2_FT_CHRDEV;
      break;
    case LINUX_S_IFIFO:
      dir_flag = EXT2_FT_FIFO;
      break;
    default:
      dir_flag = EXT2_FT_UNKNOWN;
      break;
    }


  if ((retval = ext2fs_link(fs, cwd, newfile, new_file_ino, dir_flag)))
    {
      /* check to see if we ran out of space in the directory */
      if (retval == EXT2_ET_DIR_NO_SPACE)
        {
          /* try resizing the directory and try again */
          if (0 == (retval = ext2fs_expand_dir(fs, cwd)))
            retval = ext2fs_link(fs, cwd, newfile, new_file_ino, dir_flag);
        }
      if (retval)
        {
          fprintf(stderr, "%s\n", error_message(retval));
          return retval;
        }
    }


  /* update the inode stat information */
  if ((ln_flags & E2T_DO_MV) == 0)
    {
      inode.i_links_count++;
      if ((retval = write_inode(fs, new_file_ino, &inode)))
        {
          fprintf(stderr, "%s\n", error_message(retval));
          return (retval);
        }
    }

  return(0);

} /* end of create_hard_link */

