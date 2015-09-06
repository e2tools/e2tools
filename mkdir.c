/* $Header: /home/ksheff/src/e2tools/RCS/mkdir.c,v 0.8 2004/04/07 03:19:32 ksheff Exp $ */
/*
 * mkdir.c
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
 * This file contains the functions used to create a directory in an ext2fs
 * filesystem.
 *
 */
/*
 * $Log: mkdir.c,v $
 * Revision 0.8  2004/04/07 03:19:32  ksheff
 * Modified to always apply the default user and group information.
 *
 * Revision 0.7  2004/04/07 02:55:44  ksheff
 * Updated usage string.
 *
 * Revision 0.6  2002/07/09 02:15:23  ksheff
 * Fixed a error reporting bug that was not displaying the full file name.
 *
 * Revision 0.5  2002/04/10 09:30:28  ksheff
 * Added the ability to set the mode, owner, group, etc. for a directory.
 *
 * Revision 0.4  2002/03/05 13:42:12  ksheff
 * Fixed minor indentation.
 *
 * Revision 0.3  2002/03/05 12:14:10  ksheff
 * Removed setting optind for SCO.
 *
 * Revision 0.2  2002/02/27 05:34:43  ksheff
 * Added the function e2mkdir which is run by a user to create directories in
 * an ext2fs filesystem.
 *
 * Revision 0.1  2002/02/27 04:48:30  ksheff
 * initial revision
 *
 */

/*  Headers */
#include "e2tools.h"
#include "mkdir.h"

/* Local Prototypes */
long
create_subdir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
              char *dirname, struct stat *def_stat);
/* Name:    e2mkdir()
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
 *     Create the directory
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
 * 0 - the directories were created successfully
 * otherwise, an error code is returned
 *
 * Author: Keith W. Sheffield
 * Date:   02/26/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 03/05/02      K. Sheffield       Removed setting optind for SCO
 * 04/10/02      K.Sheffield        Added a default directory permission
 * 04/06/04      K.Sheffield        Modified to apply the default user and
 *                                  group.
 */
long
e2mkdir(int argc, char *argv[])
{
  int verbose=0;
  int num_files;
  int errcnt=0;
  char *last_filesys = NULL;
  char *cur_filesys = NULL;
  ext2_filsys fs = NULL;
  ext2_ino_t root;
  ext2_ino_t cwd;
  char **cur_opt;
  char *filespec;
  int i;
  char *ptr;
  int c;
  long retval;
  struct stat def_stat;

  init_stat_buf(&def_stat);
  def_stat.st_ino = 1;

#ifdef HAVE_OPTRESET
  optreset = 1;     /* Makes BSD getopt happy */
#endif
  while ((c = getopt(argc, argv, "G:O:P:v")) != EOF)
    {
      switch (c)
        {
        case 'v':
          verbose = 1;
          break;
        case 'G':
          def_stat.st_gid = atoi(optarg);
          break;
        case 'O':
          def_stat.st_uid = atoi(optarg);
          break;
        case 'P':
          def_stat.st_mode = strtol(optarg, NULL, 8);
          break;
        default:
          errcnt++;
          break;
        }
    }

  if (errcnt || argc == optind)
    {
      fputs("Usage: e2mkdir [-G gid][-O uid][-P mode][-v] filesystem:directory...\n", stderr);
      return(1);
    }

  num_files = argc - optind;

  cur_opt = argv + optind;
  if (num_files > 1 )
    qsort(cur_opt, num_files, sizeof(char *), my_strcmp);

  for(i=0;i<num_files;i++)
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
          if ((retval = create_dir(fs, root, &cwd, filespec, &def_stat)))
            {
              fprintf(stderr, "Error creating %s:%s\n", cur_filesys, filespec);
              return(-1);
            }
          cwd = root;           /* reset the current directory */
          if (verbose)
            printf("Created directory %s:%s\n", cur_filesys, filespec);
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

} /* end of e2mkdir */

/* Name:    create_dir()
 *
 * Description:
 *
 * This function will create a directory in the given ext2fs file system.
 * It will re-create all the parent directories if necessary (similar
 * to mkdir -p).  The current working directory is then set to this new
 * directory.
 *
 * Algorithm:
 *
 * Check the input parameters
 * Check to see if the directory name contains a /
 * If it exists
 *     Break the entire path into specific directory names
 *     For each section of the path
 *     Check to see if the name exists in the current directory
 *     If it doesn't
 *         Call create_subdir for that section of the path
 *     Otherwise
 *         If it is a symbolic link, follow it
 *         Read the inode structure for the current section of the path
 *         If it is not a directory
 *             Print an error message and return an error.
 * Otherwise
 *    Call create_subdir for that section of the path
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * ext2_filsys fs;            The current file system
 * ext2_ino_t root;           The inode number of the root directory
 * ext2_ino_t *cwd;           Pointer to the inode number of the current
 *                            directory
 * char *dirname;             The directory to create
 * struct stat *def_stat;     Default directory status, owner, group, etc.
 *
 * Return Values:
 *
 * 0 - the directory was created successfully
 * error code of what went wrong
 *
 * Author: Keith W. Sheffield
 * Date:   02/18/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 04/10/02      K.Sheffield        Added a default directory permission
 * 07/08/02      K.Sheffield        Fixed bugs in error reporting
 */
long
create_dir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
           char *dirname, struct stat *def_stat)
{
  char *ptr;                  /* working pointer to /'s */
  ext2_ino_t parent;          /* the parent directory inode number*/
  ext2_ino_t child;           /* the inode number of the new directory */
  struct ext2_inode inode;    /* inode of file/directory dirname */
  long retval;                /* function return value */
  char *buf;
  int len;
  char c;
  char *dname;                 /* name of the sub directory */

  /* make sure we have valid parameters */
  if (fs == NULL || cwd == NULL || dirname == NULL || *dirname == '\0')
    {
      fputs("create_dir: invalid parameter\n", stderr);
      return(-1);
    }

  EXT2_CHECK_MAGIC(fs, EXT2_ET_MAGIC_EXT2FS_FILSYS);

  /* check to see if there are any sub directories or absolute paths given
   */
  if (NULL != (ptr = strchr(dirname, '/')))
    {

      dname = dirname;
      /* if the directory name starts with / the starting parent directory
       * is the root directory.
       */
      if (*dirname == '/')
        {
          parent = root;
          dname++;
        }
      else
        parent = *cwd;

      /* allocate memory for the lookups */
      if ((retval = ext2fs_get_mem(fs->blocksize, (void *) &buf)))
        {
          fprintf(stderr, "%s\n", error_message(retval));
          return retval;
        }

      c = *dname;
      ptr = dname;
      while (c != '\0')
        {
          len = 0;
          /* skip ahead to the next / character */
          while ((c = *ptr) != '/' && c != '\0')
            {
              len++;
              ptr++;
            }

          if (c == '/')
            *ptr++ = '\0';

          /* multiple /'s in a row or trailing / */
          if (len == 0)
            {
              /* restore /'s */
              if (dname > dirname)
                *--dname = '/';

              dname = ptr;
              continue;
            }

          /* check to see if this file is in the parent directory */
          if ((retval = ext2fs_lookup(fs, parent, dname, len, buf, &child)))
            {
              if (retval == EXT2_ET_FILE_NOT_FOUND)
                {
                  /* ok, it's not there, so just create it */
                  if ((retval = create_subdir(fs, root, &parent, dname,
                                              def_stat)))
                    {
                      fprintf(stderr,
                              "create_dir: error creating directory %s/%s:%ld\n",
                              dirname, dname, retval);
                      ext2fs_free_mem((void *) &buf);
                      return(retval);
                    }
                }
              else
                {
                  fprintf(stderr, "%s\n", error_message(retval));
                  ext2fs_free_mem((void *) &buf);
                  return retval;
                }
            }
          else
            {
              /* ok, so the name exists...follow it if it's a symlink
               * otherwise, it's a directory so set the parent directory to
               * the child inode number
               */
              if ((retval = ext2fs_follow_link(fs, root, parent, child,
                                               &parent)))
                {
                  fprintf(stderr, "%s\n", error_message(retval));
                  ext2fs_free_mem((void *) &buf);
                  return retval;
                }

              /* now check to see if it's a directory */
              if ((retval = ext2fs_read_inode(fs, parent, &inode)))
                {
                  fprintf(stderr, "%s\n", error_message(retval));
                  ext2fs_free_mem((void *) &buf);
                  return retval;
                }

              /* not a directory? something's wrong */
              if (!LINUX_S_ISDIR(inode.i_mode))
                {
                  fprintf(stderr, "create_dir: %s/%s is not a directory: %o\n",
                          dirname, dname, inode.i_mode);
                  ext2fs_free_mem((void *) &buf);
                  return(-1);
                }
            }

          /* restore /'s */
          if (dname > dirname)
            *--dname = '/';
          dname = ptr;
        }
      ext2fs_free_mem((void *) &buf);
      *cwd = parent;
    }
  else
    {
      if ((retval = create_subdir(fs, root, cwd, dirname, def_stat)))
        {
          fprintf(stderr,
                  "create_dir: error creating directory %s:%ld\n",
                  dirname, retval);
          return(retval);
        }
    }
  return(0);
} /* end of create_dir */

/* Name:    create_subdir()
 *
 * Description:
 *
 * This function will create a sub directory in the given ext2fs file system.
 *  The current working directory is then set to this new directory.
 *
 * Algorithm:
 *
 * Find a new inode number to use
 * Create the directory
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * ext2_filsys fs;            The current file system
 * ext2_ino_t root;           The inode number of the root directory
 * ext2_ino_t *cwd;           Pointer to the inode number of the current
 *                            or parent directory
 * char *dirname;             The directory to create
 * struct stat *def_stat;     Default directory status, owner, group, etc.
 *
 * Return Values:
 *
 * 0 - the directory was created successfully
 * error code of what went wrong
 *
 * Author: Keith W. Sheffield
 * Date:   02/18/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 04/10/02      K.Sheffield        Added a default directory permission
 *
 */
long create_subdir(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd,
                   char *dirname, struct stat *def_stat)
{
  ext2_ino_t parent;          /* the parent directory inode number*/
  ext2_ino_t child;           /* the inode number of the new directory */
  long retval;                /* function return value */
  struct ext2_inode inode;

  parent = *cwd;

  if ((retval = ext2fs_namei(fs, root, parent, dirname, &child)))
    {
      if (retval != EXT2_ET_FILE_NOT_FOUND)
        {
          fprintf(stderr, "%s\n",error_message(retval));
          return(retval);
        }
    }
  /* file name exists, let's see if is a directory */
  else if ((retval = ext2fs_check_directory(fs, child)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return(retval);
    }
  else
    {
    /* if we get here, then it's an existing directory */
      *cwd = child;
      return(0);
    }

  /* ok, the directory doesn't exist */
  /* get a new inode number */
  if ((retval = ext2fs_new_inode(fs, parent, LINUX_S_IFDIR | 0755, 0, &child)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return retval;
    }

  /* now create the directory */
  if ((retval = ext2fs_mkdir(fs, parent, child, dirname)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return retval;
    }

  *cwd = child;
  if (def_stat != NULL && def_stat->st_ino != 0)
    {
      if ((retval = read_inode(fs, child, &inode)))
        return(retval);

      if (def_stat->st_mode != 0)
        inode.i_mode = LINUX_S_IFDIR | host_mode_xlate(def_stat->st_mode);

      inode.i_uid = def_stat->st_uid;
      inode.i_gid = def_stat->st_gid;
      inode.i_atime = def_stat->st_atime;
      inode.i_ctime = def_stat->st_ctime;
      inode.i_mtime = def_stat->st_mtime;

      if ((retval = write_inode(fs, child, &inode)))
        return(retval);
    }

  return(0);
} /* end of create_subdir */

/* Name:    change_cwd()
 *
 * Description:
 *
 * This function changes the current working directory
 *
 * Algorithm:
 *
 * Look up the inode number for the input string
 * check to see if it's a directory
 * Assign it as the current working directory if it is.
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * ext2_filsys fs;         The current filesystem
 * ext_ino_t root;         The root directory
 * ext_ino_t *cwd;         The current working directory
 * char *dirname;          The name of the directory we want to change to
 *
 * Return Values:
 *
 * 0 - changed to the directory successfully
 * the error code of what went wrong
 *
 * Author: Keith W Sheffield
 * Date:   02/21/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
long
change_cwd(ext2_filsys fs, ext2_ino_t root, ext2_ino_t *cwd, char *dirname)
{
  ext2_ino_t inode;
  long retval;

  if ((retval = ext2fs_namei(fs, root, *cwd, dirname, &inode)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return retval;
    }
  else if ((retval = ext2fs_check_directory(fs, inode)))
    {
      fprintf(stderr, "%s\n", error_message(retval));
      return retval;
    }
  *cwd = inode;
  return(0);
} /* end of change_cwd */
