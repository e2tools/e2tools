/* $Header: /home/ksheff/src/e2tools/RCS/rm.c,v 0.3 2004/04/06 23:39:28 ksheff Exp $ */
/*
 * rm.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Derived from debugfs.c Copyright (C) 1993 Theodore Ts'o <tytso@mit.edu>
 * and modified by Robert Sanders <gt8134b@prism.gatech.edu>
 * as well as portions of the ext2fs library (C) 1993-1995 Theodore Ts'o
 */

/* Description */
/*
 * This file contains function used to remove files in an ext2fs filesystem.
 *
 */
/*
 * $Log: rm.c,v $
 * Revision 0.3  2004/04/06 23:39:28  ksheff
 * Corrected getopt and usage strings.
 *
 * Revision 0.2  2002/06/05 23:39:13  ksheff
 * Added ability to delete files based on a regular expression and recursive
 * deletion of directories.
 *
 * Revision 0.1  2002/04/10 10:42:40  ksheff
 * initial revision
 *
 */


/*  Headers */
#include "e2tools.h"
#include "rm.h"
#include <regex.h>

static ext2_filsys gbl_fs = NULL;
static char *gbl_filesys = NULL;
static char *gbl_path_end = NULL;
static char gbl_dir_name[8192];

struct regexp_args
{
  int verbose;
  int recursive;
  regex_t *reg;
};

/* Local Prototypes */
static
int rm_dir_proc(ext2_ino_t dir,int	entry, struct ext2_dir_entry *dirent,
                int	offset, int	blocksize, char	*buf, void *verbose);
static int
recursive_rm(ext2_ino_t dir, char *name, struct ext2_dir_entry *dirent,
             int verbose);
static int
rm_regex_proc(ext2_ino_t dir, int entry, struct ext2_dir_entry *dirent,
              int offset, int blocksize, char *buf, void *private);

/* Name:	e2rm()
 *
 * Description:
 *
 * This function extracts the command line parameters and creates
 * directories based on user input
 *
 * Algorithm:
 *
 * Extract any command line switches
 * Sort the remaining arguments
 * For each argument
 *     Open a new filesystem if needed
 *     Remove the file if it exists.
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * int argc;       The number of arguments
 * char *argv[];   the command line arguments
 *
 * Return Values:
 *
 * 0 - the filess were created successfully
 * otherwise, an error code is returned
 *
 * Author: Keith W. Sheffield
 * Date:   04/10/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 04/06/04      K.Sheffield        Corrected getopt and usage strings.
 */
long
e2rm(int argc, char *argv[])
{
  int verbose=0;
  int errcnt=0;
  int recursive=0;
  char *last_filesys = NULL;
  char *cur_filesys = NULL;
  ext2_filsys fs = NULL;
  ext2_ino_t root;
  ext2_ino_t cwd;
  char **cur_opt;
  char *filespec;
  long retval;
  char *dir_name;
  char *base_name;
  ext2_ino_t curr_ino;
  int c;
  int num_files;
  char *ptr;
  struct regexp_args reg;

#ifdef HAVE_OPTRESET
  optreset = 1;		/* Makes BSD getopt happy */
#endif
  while ((c = getopt(argc, argv, "vr")) != EOF)
    {
      switch (c)
        {
        case 'v':
          verbose = 1;
          break;
        case 'r':
          recursive = 1;
          break;
        default:
          errcnt++;
          break;
        }
    }

  if (errcnt || argc == optind)
    {
      fputs("Usage: e2rm [-vr] filesystem:filepath...\n", stderr);
      return(1);
    }

  num_files = argc - optind;

  cur_opt = argv + optind;
  if (num_files > 1 )
    qsort(cur_opt, num_files, sizeof(char *), my_strcmp);

  for(c=0;c<num_files;c++)
    {
      filespec = *cur_opt++;

      /* dealing with a filesystem or a regular file */
      if (NULL != (ptr = strchr(filespec, ':')))
        {
          /* separate the filesystem name from the file name */
          *ptr++ = '\0';
          cur_filesys = filespec;
          filespec = ptr;

          /* check to see if the filesystem has changed */
          if (last_filesys == NULL ||
              strcmp(last_filesys, cur_filesys) != 0)
            {
              if (last_filesys != NULL &&
                  (retval = ext2fs_close(fs)))
                {
                  fprintf(stderr, "%s\n", error_message(retval));
                  return retval;
                }

              if ((retval = open_filesystem(cur_filesys, &fs, &root, 1)))
                {
                  fprintf(stderr, "%s\n", error_message(retval));
                  fprintf(stderr, "Error opening fileystem %s\n",
                          cur_filesys);
                  return retval;
                }
              cwd = root;
              last_filesys = cur_filesys;
            } /* end of filesystem change? */


          if (get_file_parts(fs, root, filespec, &cwd, &dir_name, &base_name))
            {
              ext2fs_close(fs);
              return(-1);
            }

          if (is_file_regexp(base_name))
              {
                if (NULL == (reg.reg = (regex_t *) make_regexp(base_name)))
                  {
                    fprintf(stderr,
                            "Error creating regular expression for %s\n",
                            base_name);
                    continue;
                  }

                gbl_fs = fs;
                gbl_filesys = cur_filesys;

                reg.verbose = verbose;
                reg.recursive = recursive;

                strcpy(gbl_dir_name, dir_name);
                gbl_path_end = gbl_dir_name + strlen(gbl_dir_name);

                retval = ext2fs_dir_iterate2(gbl_fs, cwd,
                                             DIRENT_FLAG_INCLUDE_EMPTY, 0,
                                             rm_regex_proc, (void *) &reg);
                regfree(reg.reg);
                continue;
              }

          /* check to see if the file name exists in the current directory */
          else if ((retval = ext2fs_namei(fs, cwd, cwd, base_name, &curr_ino)))
            {
              if (retval != EXT2_ET_FILE_NOT_FOUND)
                {
                  fprintf(stderr, "%s\n",error_message(retval));
                  return(retval);
                }
            }
          /* file name exists, let's see if is a directory */
          else if ((retval = ext2fs_check_directory(fs, curr_ino)) == 0)
            {
              if (recursive)
                {
                  gbl_fs = fs;
                  gbl_filesys = cur_filesys;

                  sprintf(gbl_dir_name, "%s/%s", (dir_name == NULL)?".":
                          dir_name, base_name);
                  gbl_path_end = gbl_dir_name + strlen(gbl_dir_name);

                  retval = ext2fs_dir_iterate2(gbl_fs, curr_ino,
                                               DIRENT_FLAG_INCLUDE_EMPTY, 0,
                                               rm_dir_proc,
                                               (void *)
                                               ((verbose) ? &verbose : NULL));
                }
              else
                {
                printf("%s:%s/%s is a directory!  Not removed\n", cur_filesys,
                       (dir_name == NULL)?".": dir_name, base_name);
                continue;
                }
            }
          if (retval != 0 && retval != EXT2_ET_NO_DIRECTORY)
            {
              fprintf(stderr, "%s\n",error_message(retval));
              return(retval);
            }
          /* delete the existing file if needed */
          if ((retval = rm_file(fs, cwd, base_name, curr_ino)))
            {
              fprintf(stderr, "%s\n",error_message(retval));
              return(retval);
            }

          if (verbose)
            printf("Removed %s:%s/%s\n", cur_filesys,
                   (dir_name == NULL)?".":dir_name, base_name);
        }

      else if (verbose)
        {
          printf("%s is not a valid e2fs filesystem specification. skipping\n",
                 filespec);
        }
    }

  if (fs != NULL &&
      (retval = ext2fs_close(fs)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return retval;
    }
  return(0);

} /* end of e2rm */

static int rm_dir_proc(ext2_ino_t UNUSED_PARM(dir), int entry,
                       struct ext2_dir_entry *dirent, int UNUSED_PARM(offset),
                       int UNUSED_PARM(blocksize), char UNUSED_PARM(*buf), void *verbose)
{
  char name[EXT2_NAME_LEN];
  int thislen;

  thislen = ((dirent->name_len & 0xFF) < EXT2_NAME_LEN) ?
    (dirent->name_len & 0xFF) : EXT2_NAME_LEN;

  if (dirent->name[0] == '.' &&
      (thislen == 1 || (thislen == 2 && dirent->name[1] == '.')))
    return 0;

  strncpy(name, dirent->name, thislen);
  name[thislen] = '\0';

  if (entry == DIRENT_DELETED_FILE)
    return 0;

  return (recursive_rm(dir, name, dirent, (verbose) ? 1 : 0));
}

static int
recursive_rm(ext2_ino_t dir, char *name, struct ext2_dir_entry *dirent,
             int verbose)
{
  long retval;

  /* file name exists, let's see if is a directory */
  if ((retval = ext2fs_check_directory(gbl_fs, dirent->inode)) == 0)
    {
      *gbl_path_end++ = '/';
      strncpy(gbl_path_end, dirent->name, dirent->name_len);
      gbl_path_end += dirent->name_len;
      *gbl_path_end = '\0';

      retval = ext2fs_dir_iterate2(gbl_fs, dirent->inode,
                                   DIRENT_FLAG_INCLUDE_EMPTY, 0,
                                   rm_dir_proc, (verbose) ? &verbose : NULL);

      while (gbl_path_end > gbl_dir_name  && *gbl_path_end != '/')
        gbl_path_end--;
      *gbl_path_end = '\0';
    }

  if (retval !=0 && retval != EXT2_ET_NO_DIRECTORY)
    {
      fprintf(stderr, "%s\n",error_message(retval));
      return(retval);
    }
  /* delete the existing file if needed */
  if ((retval = rm_file(gbl_fs, dir, name, dirent->inode)))
    {
      fprintf(stderr, "%s\n",error_message(retval));
      return(retval);
    }

  if (verbose)
    printf("Removed %s:%s/%s\n", gbl_filesys, gbl_dir_name, name);

  return 0;
}

static int
rm_regex_proc(ext2_ino_t dir, int entry, struct ext2_dir_entry *dirent,
              int UNUSED_PARM(offset), int UNUSED_PARM(blocksize), char *UNUSED_PARM(buf), void *arg)
{
  long retval;
  char name[EXT2_NAME_LEN];
  int thislen;
  struct regexp_args *reg = (struct regexp_args *) arg;

  thislen = ((dirent->name_len & 0xFF) < EXT2_NAME_LEN) ?
    (dirent->name_len & 0xFF) : EXT2_NAME_LEN;

  if (dirent->name[0] == '.' &&
      (thislen == 1 || (thislen == 2 && dirent->name[1] == '.')))
    return 0;

  strncpy(name, dirent->name, thislen);
  name[thislen] = '\0';

  if (entry == DIRENT_DELETED_FILE)
    return 0;

  if (0 == (retval = regexec(reg->reg, name, 0, NULL, 0)))
    {
      if (reg->recursive)
        return (recursive_rm(dir, name, dirent, (reg->verbose) ? 1 : 0));
      else
        {
          if ((retval = ext2fs_check_directory(gbl_fs, dirent->inode)))
            {
              if (retval != EXT2_ET_NO_DIRECTORY)
                {
                  fprintf(stderr, "%s\n",error_message(retval));
                  return(retval);
                }
              /* delete the existing file if needed */
              if ((retval = rm_file(gbl_fs, dir, name, dirent->inode)))
                {
                  fprintf(stderr, "%s\n",error_message(retval));
                  return(retval);
                }

              if (reg->verbose)
                printf("Removed %s:%s/%s\n", gbl_filesys, gbl_dir_name, name);
            }
          else
            {
              printf("%s:%s/%s is a directory!  Not removed\n", gbl_filesys,
                     gbl_dir_name, name);
            }
        }
    }
  return (0);
}





