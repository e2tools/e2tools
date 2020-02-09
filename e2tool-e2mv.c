/** \file e2tool-e2mv.c
 * \brief File moving and renaming tool.
 */

/* $Header: /home/ksheff/src/e2tools/RCS/mv.c,v 0.1 2002/03/21 09:03:25 ksheff Exp $ */
/*
 * mv.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 */

/* Description */
/*
 * Module to move/rename files
 *
 */
/*
 * $Log: mv.c,v $
 * Revision 0.1  2002/03/21 09:03:25  ksheff
 * initial revision.
 *
 */

/* Feature Test Switches */
/*  Headers */
#include "e2tools.h"

/* Macros */
#define USAGE "Usage: e2mv [-vfs] source1 [... sourceN] destination\n"

/* Local Prototypes */
static long
do_swap(int force, int verbose, int curidx, int argc, char **argv);

/* Name:    main_e2mv()
 *
 * Description:
 *
 * This function reads the command line arguments and moves or renames files
 * in an ext2fs file system
 *
 * Algorithm:
 *
 * Read any command line switches
 * Get the first source file specification
 * If we are performing a file swap, call do_swap()
 * Open the file system
 * Get the destination and determine if it is a directory
 *    If not, then get the destination's directory and basename
 *    Also check that the number of source files are no more than one
 * For each source file
 *    Get the directory and basename of the source file
 *    Determine the inode number for the source file
 *    Create the link
 *    Unlink the original source file.
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
 * 0 - the file was move successfully
 * an error occurred.
 *
 * Author: Keith W. Sheffield
 * Date:   03/20/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 */
int
main_e2mv(int argc, char *argv[])
{
  int verbose=0;
  int force=0;
  int swap_files=0;
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
  char *result_name;
  long retval;
  int c;
  int curidx;

#ifdef HAVE_OPTRESET
  optreset = 1;     /* Makes BSD getopt happy */
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
          swap_files = 1;
          break;
        default:
          errcnt++;
          break;
        }
    }

  curidx = optind;

  force |= E2T_DO_MV;

  if (errcnt || argc < curidx+2)
    {
      fputs(USAGE, stderr);
      return(1);
    }

  if (swap_files)
    return(do_swap(force, verbose, curidx, argc, argv));

  cur_filesys = argv[curidx++];
  if (NULL == (src_dir = strchr(cur_filesys, ':')))
    {
      fprintf(stderr, "Invalid file specification: %s\n", cur_filesys);
      return(1);
    }
  *src_dir++ = '\0';

  if ((retval = open_filesystem(cur_filesys, &fs, &root, 1)))
    {
      return retval;
    }


  /* get the destination directory */
  dest_name = NULL;
  if (strcmp(dest_dir = argv[argc-1], ".") != 0)
    {
      /* check to see if the file name already exists in the current
       * directory  and also see if it is a directory.
       */
      if ((retval = ext2fs_namei(fs, root, root, dest_dir, &destd)) ||
          (retval = ext2fs_check_directory(fs, destd)))
        {
          if (retval != EXT2_ET_FILE_NOT_FOUND &&
              retval != EXT2_ET_NO_DIRECTORY)
            {
              fprintf(stderr, "%s\n",error_message(retval));
              ext2fs_close(fs);
              return(retval);
            }

          /* ok, so it's either not there or it's not a directory, so
           * get the real destination directory and file name.
           */
          if (curidx+1 < argc)
            {
              fprintf(stderr, "%s must be a directory!\n", dest_dir);
              ext2fs_close(fs);
              return(1);
            }

          if (get_file_parts(fs, root, dest_dir, &destd, &dest_dir,
                             &dest_name))
            {
              ext2fs_close(fs);
              return(-1);
            }
        }
      else                  /* we have a directory!!! */
        dest_name = NULL;
    }
  else
    {
      destd = root;
      dest_name = NULL;
    }

  do
    {
      /* move to the source directory */
      if (get_file_parts(fs, root, src_dir, &srcd, &src_dir, &src_name))
        {
          ext2fs_close(fs);
          return(-1);
        }

      /* get the inode number for the source file */
      if ((retval = ext2fs_namei(fs, srcd, srcd, src_name, &source_file)))
        {
          fprintf(stderr, "%s: source file %s\n",error_message(retval),
                  src_name);
          ext2fs_close(fs);
          return(retval);
        }

      result_name = (dest_name) ? dest_name : src_name;

      /* now create the link */
      if ((retval = create_hard_link(fs, destd, source_file, result_name,
                                     force)))
        {
          fprintf(stderr, "Error renaming %s/%s as %s/%s\n",
                  ((src_dir == NULL) ? "." : src_dir), src_name,
                  ((dest_dir == NULL) ? "." : dest_dir), result_name);
          ext2fs_close(fs);
          return(1);
        }

      if ((retval = ext2fs_unlink(fs, srcd, src_name, 0, 0)))
        {
          fprintf(stderr, "%s - %s\n", src_name, error_message(retval));
          ext2fs_close(fs);
          return(retval);
        }

      if (verbose)
        fprintf(stderr, "moved %s/%s as %s/%s\n",
                ((src_dir == NULL) ? "." : src_dir), src_name,
                ((dest_dir == NULL) ? "." : dest_dir), result_name);
      src_dir = argv[curidx++];
    }
  while (curidx < argc);

  ext2fs_close(fs);
  return(0);

} /* end of do_mv */

/* Name:    get_file_parts()
 *
 * Description:
 *
 * This function returns each of the following file 'parts': directory name,
 * base name, inode number of the directory
 *
 * Algorithm:
 *
 * Use the root directory as the current working directory
 * Find the last / in the full pathname
 *     If none are found, set the basename to the full pathname,
 *     and the directory to NULL
 * Otherwise,
 *     Separate the basename from the directory
 *     Change the working directory
 * Set the return pointers.
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * ext2_filsys fs;            the filesystem being used
 * ext2_ino_t root;           the root directory of the filesystem
 * char *pathname;            the full pathname of the file
 * ext2_ino_t *dir_ino;       The inode number of the directory
 * char **dir_name;           the directory the file is in
 * char **base_name;          The basename of the file
 *
 * Return Values:
 *
 * 0 - retrieved the information ok
 * otherwise the error code of what went wrong
 *
 * Author: Keith W. Sheffield
 * Date:   03/21/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
long
get_file_parts(ext2_filsys fs, ext2_ino_t root, char *pathname,
               ext2_ino_t *dir_ino, char **dir_name, char **base_name)
{
  char *fname;
  long retval;

  /* move to the source directory */
  *dir_name = pathname;
  *dir_ino = root;
  if (NULL == (fname = strrchr(pathname, '/')))
    {
      fname = pathname;
      *dir_name = NULL;
    }
  else
    {
      *fname++ = '\0';
      if ((*pathname != '\0' && strcmp(pathname, ".") != 0) &&
          (retval = change_cwd(fs, root, dir_ino, pathname)))
        {
          fprintf(stderr, "Error changing to directory %s\n",
                  pathname);
          return(retval);
        }
    }

    *base_name = fname;
    return(0);
} /* end of get_file_parts */


/* Name:    do_swap()
 *
 * Description:
 *
 * This function swaps the names of two files and optionally assigns the
 * first file a new name:
 *
 * file1 file2 file3
 *
 * After the operation, file1 will reference the file that was originally file2, and file3 will reference the what used to be file1.
 *
 * Algorithm:
 *
 * check input parameters
 * Get the directory and inode numbers for the first two files
 * If a third file exists
 *     Get the directory info for the file
 *     Rename the first file to the third
 *     Rename the 2nd file to the first
 * Otherwise
 *     Remove the first file from its directory
 *     Rename the 2nd file to the first
 *     Add the first file back as the 2nd file.
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * int force;                 Flag indicating if existing files are to be removed
 * int verbose;          Flag to print lots of output
 * int curidx;           The current index in the command line args
 * int argc;             The total number of arguments
 * char **argv;          Pointer to an array of argument strings.
 *
 * Return Values:
 *
 * 0 - the operation was successful
 * otherwise the error code of what went wrong.
 *
 * Author: Keith W. Sheffield
 * Date:   03/21/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */

static long
do_swap(int force, int verbose, int curidx, int argc, char **argv)
{
  char *cur_filesys = NULL;
  ext2_filsys fs = NULL;
  ext2_ino_t root;
  ext2_ino_t file1_dirno;
  ext2_ino_t file2_dirno;
  ext2_ino_t file3_dirno;
  ext2_ino_t file1_no;
  ext2_ino_t file2_no;
  char *file1_dir;
  char *file2_dir;
  char *file3_dir;
  char *file1_name;
  char *file2_name;
  char *file3_name;
  long retval;

  if (curidx + 2 > argc)
    {
      fputs(USAGE, stderr);
      return(1);
    }

  cur_filesys = argv[curidx++];
  if (NULL == (file1_dir = strchr(cur_filesys, ':')))
    {
      fprintf(stderr, "Invalid file specification: %s\n", cur_filesys);
      return(1);
    }
  *file1_dir++ = '\0';

  if ((retval = open_filesystem(cur_filesys, &fs, &root, 1)))
    {
      return retval;
    }

  /* move to the file 1 directory */
  if (get_file_parts(fs, root, file1_dir, &file1_dirno, &file1_dir,
                     &file1_name))
    {
      ext2fs_close(fs);
      return(-1);
    }

  /* get the inode number for the file 1 file */
  if ((retval = ext2fs_namei(fs, file1_dirno, file1_dirno, file1_name,
                             &file1_no)))
    {
      fprintf(stderr, "%s: file 1 file %s\n",error_message(retval),
              file1_name);
      ext2fs_close(fs);
      return(retval);
    }


  /* move to the file 2 directory */
  if (get_file_parts(fs, root, argv[curidx++], &file2_dirno, &file2_dir,
                     &file2_name))
    {
      ext2fs_close(fs);
      return(-1);
    }

  /* get the inode number for the file 2 file */
  if ((retval = ext2fs_namei(fs, file2_dirno, file2_dirno, file2_name,
                             &file2_no)))
    {
      fprintf(stderr, "%s: file 2 file %s\n",error_message(retval),
              file2_name);
      ext2fs_close(fs);
      return(retval);
    }

  if (curidx < argc)
    {
      /* move to the file 3 directory */
      if (get_file_parts(fs, root, argv[curidx++], &file3_dirno, &file3_dir,
                         &file3_name))
        {
          ext2fs_close(fs);
          return(-1);
        }

      /* now move the first file to the 3rd */
      if ((retval = create_hard_link(fs, file3_dirno, file1_no, file3_name,
                                     force)))
        {
          fprintf(stderr, "Error renaming %s/%s as %s/%s\n",
                  ((file1_dir == NULL) ? "." : file1_dir), file1_name,
                  ((file3_dir == NULL) ? "." : file3_dir), file3_name);
          ext2fs_close(fs);
          return(1);
        }

      if ((retval = ext2fs_unlink(fs, file1_dirno, file1_name, 0, 0)))
        {
          fprintf(stderr, "%s - %s\n", file1_name, error_message(retval));
          ext2fs_close(fs);
          return(retval);
        }


      /* now move the 2nd file to the 1st */
      if ((retval = create_hard_link(fs, file1_dirno, file2_no, file1_name,
                                     force)))
        {
          fprintf(stderr, "Error renaming %s/%s as %s/%s\n",
                  ((file2_dir == NULL) ? "." : file2_dir), file2_name,
                  ((file1_dir == NULL) ? "." : file1_dir), file1_name);
          ext2fs_close(fs);
          return(1);
        }

      if ((retval = ext2fs_unlink(fs, file2_dirno, file2_name, 0, 0)))
        {
          fprintf(stderr, "%s - %s\n", file2_name, error_message(retval));
          ext2fs_close(fs);
          return(retval);
        }

      if (verbose)
        fprintf(stderr, "renamed file %s/%s as %s/%s\n"
                "renamed file %s/%s as %s/%s\n",
                ((file1_dir == NULL) ? "." : file1_dir), file1_name,
                ((file3_dir == NULL) ? "." : file3_dir), file3_name,
                ((file2_dir == NULL) ? "." : file2_dir), file2_name,
                ((file1_dir == NULL) ? "." : file1_dir), file1_name);
    }
  else
    {
      /* now remove the first file */
      if ((retval = ext2fs_unlink(fs, file1_dirno, file1_name, 0, 0)))
        {
          fprintf(stderr, "%s - %s\n", file1_name, error_message(retval));
          ext2fs_close(fs);
          return(retval);
        }

      /* now move the 2nd file to the 1st */
      if ((retval = create_hard_link(fs, file1_dirno, file2_no, file1_name,
                                     force)))
        {
          fprintf(stderr, "Error renaming %s/%s as %s/%s\n",
                  ((file2_dir == NULL) ? "." : file2_dir), file2_name,
                  ((file1_dir == NULL) ? "." : file1_dir), file1_name);
          ext2fs_close(fs);
          return(1);
        }

      if ((retval = ext2fs_unlink(fs, file2_dirno, file2_name, 0, 0)))
        {
          fprintf(stderr, "%s - %s\n", file2_name, error_message(retval));
          ext2fs_close(fs);
          return(retval);
        }

      if ((retval = create_hard_link(fs, file2_dirno, file1_no, file2_name,
                                     force)))
          {
          fprintf(stderr, "Error renaming %s/%s as %s/%s\n",
                  ((file1_dir == NULL) ? "." : file1_dir), file1_name,
                  ((file2_dir == NULL) ? "." : file2_dir), file2_name);
          ext2fs_close(fs);
          return(1);
        }

      if (verbose)
        fprintf(stderr, "swapped files %s/%s <-> %s/%s\n",
                ((file1_dir == NULL) ? "." : file1_dir), file1_name,
                ((file2_dir == NULL) ? "." : file2_dir), file2_name);

    }

  ext2fs_close(fs);
  return(0);

} /* end of do_swap */
