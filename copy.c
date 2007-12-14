/* $Header: /home/ksheff/src/e2tools/RCS/copy.c,v 0.16 2004/04/07 01:15:20 ksheff Exp $ */
/*
 * copy.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 */

#ifndef COPY_C
#define COPY_C
#endif

/* Description */
/*
 * This module contains functions used to copy files to and from an ext2fs
 * filesystem
 *
 */
/*
 * $Log: copy.c,v $
 * Revision 0.16  2004/04/07 01:15:20  ksheff
 * Modified to pass the default file stat to put_file() so the user may alter
 * the owner, group, or permissions of a file copied to an ext2fs file system.
 *
 * Revision 0.15  2004/04/07 00:13:38  ksheff
 * Corrected behavior when the argument to -d is a local filesystem.
 *
 * Revision 0.14  2004/04/06 23:26:37  ksheff
 * Updated the usage string.
 *
 * Revision 0.13  2002/07/09 06:43:19  ksheff
 * compensated for blank dest_dir.
 *
 * Revision 0.12  2002/07/09 06:31:41  ksheff
 * Forgot to remove debugging printfs.
 *
 * Revision 0.11  2002/07/09 02:36:51  ksheff
 * Verbose was not always reporting the correct output destination - fixed.
 *
 * Revision 0.10  2002/06/26 11:13:46  ksheff
 * Added a call to init_progress() before any copy operation if the verbose
 * flag is set.
 *
 * Revision 0.9  2002/06/19 00:52:00  ksheff
 * Fixed read from stdin to filesystem.
 *
 * Revision 0.8  2002/05/22 03:00:39  ksheff
 * Fixed the search order bug in find_link().
 *
 * Revision 0.7  2002/05/06 06:37:50  ksheff
 * Fixed a bug where if the destination filespec was just localpath:, the file
 * would not be copied to the root directory.
 *
 * Revision 0.6  2002/05/02 07:15:49  ksheff
 * Fixed a bug that was causing a file to be copied to the home directory if
 * the destination file already existed.
 *
 * Revision 0.5  2002/04/10 09:29:40  ksheff
 * Added the ability to set or keep the ownership, group, and permissions for
 * directories being created.
 *
 * Revision 0.4  2002/03/07 07:16:16  ksheff
 * Modified to copy hard links to an ext2fs system correctly.
 *
 * Revision 0.3  2002/02/27 13:34:16  ksheff
 * Added a directory check for file names read from stdin and copied to an
 * ext2fs.
 *
 * Revision 0.2  2002/02/27 05:05:45  ksheff
 * Removed unnecessary printf
 *
 * Revision 0.1  2002/02/27 04:46:46  ksheff
 * initial revision
 *
 */

/* Feature Test Switches */
/*  Headers */

#include "e2tools.h"
#include "elist.h"

/* Macros */
#define USAGE "Usage: e2cp [-0apv][-P mode][-O uid][-G gid][-d dest_dir][-s src_dir][file1...N dest]\n"
#define BUF_SIZE 8192

#ifdef isspace
#define ISSPACE(c) isspace(c)
#else
#define ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' \
		    || (c) == '\f' || (c) == '\v')
#endif

/* Structures and Unions */
typedef enum {NONE, HOST_FS, EXT2_FS} FS_CAT_T;
typedef struct 
{
    dev_t dev_no;
    ino_t ino_no;
    ext2_ino_t new_ino;
} INODE_XREF_T;

/* Local Variables */

elist_t *link_list = NULL;
static char cur_out_dir[BUF_SIZE];

/* External Prototypes */

extern void init_progress(char *file, struct stat *sbuf);

/* Local Prototypes */

long
copy(int argc, char *argv[]);
long
open_destination(char **dest_dir, char **cur_filesys, ext2_filsys *fs,
                 ext2_ino_t *root, ext2_ino_t *cwd, char *outfile,
                 int *absolute, FS_CAT_T *dst_cat, int *allow_non_dir,
                 struct stat *def_stat);
long
copy_files(int num_files, char **cur_file_names, char *dest_dir, char *dest_fs,
           ext2_filsys *fs, ext2_ino_t *root, ext2_ino_t orig_cwd,
           char *outpath, char *out_file, int max_file_len,
           FS_CAT_T src_category, FS_CAT_T dst_category, int absolute,
           int keep, int file_to_file, int verbose, struct stat *def_stat);
static char *
diag_output_name(char *odir, int file_to_file, char *ofile);

long
dir_changed(char *newdir, ext2_filsys fs, ext2_ino_t root, ext2_ino_t cwd,
            ext2_ino_t *newcwd, struct stat *def_stat, char *dest_dir);
int
read_line(char *inbuf);
int
read_string(char *inbuf);

int
my_strcmp(const void *n1, const void *n2);

INODE_XREF_T *
find_link(struct stat *sbuf);
long
add_link(struct stat *sbuf, ext2_ino_t newfile);
static long
cp_to_ext2fs(ext2_filsys fs, ext2_ino_t cwd, char *in_file, char *out_file,
             struct stat *statbuf, int keep, struct stat *def_stat);


/* Name:	copy()
 *
 * Description:
 *
 * This function will copy one or more files from a host filesystem to a
 * ext2fs filesystem and vice versa.  The files to be copied are presented
 * as command line arguments or are read from standard input.  The files are
 * in the form:
 *	
 * host filesystem:	  /this/is/a/file
 * ext2fs filesystem:  filesystem_path:/this/is/another/file
 *
 * The - character represents stdin/stdout.	 The meaning will vary depending if
 * it used as a source of destination.
 *
 * Where filesystem_path represents where the ext2fs is located in the
 * host or local filesystem.  This can be a regular file containing a
 * or block device formatted with ext2fs tools.
 *
 * The parameters are:
 *
 * -0		Input lines terminated by a null character
 * -a		Absolute directory names - create directories instead of just
 *			copying into the destination.  Only valid for copying into an
 *			ext2fs filesystem
 * -d		Destination of files to be copied.	May be in the ext2fs filesystem
 *			or the host filesystem
 * -p		Preserve host file attributes (permissions, times, etc.) when
 *			copying files.
 * -s		The source of the files to be copied.
 * -v		Be verbose.
 *
 *
 * Algorithm:
 *
 * Parse the command line for flags and special parameters
 * If there are any parameters left, they are the files to be copied
 *	   If no -s or -d parameters have been processed
 *		   Retrieve the last parameter which is the destination
 *		   If the destination is a ext2fs file specificiation
 *			   Open the filesystem with write capability
 *		   Test to make sure that is a directory and not a file
 *	   Sort the names of the files to be copied.
 *	   For each file
 *		   If it is an ext2fs file specification
 *			   Check to see if it is the same as the last ext2fs spec
 *			   If not, close the prior ext2fs and open the new one
 *		   If the source and destination files are of the same type
 *			   Print a warning message and continue
 *		   Copy the file
 * Otherwise, no parameters are left, file list is on stdin
 *	   For each line of input
 *		   If the -0 is not given, strip whitespace off the end
 *		   If the -a option is given
 *			   determine the dirname of the file
 *			   Compare the directory to the last directory processed
 *			   If different
 *				   Make a new directory
 *		   Copy the file
 * Close any open ext2fs filesystems
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * int argc;			 The number of arguments
 * char *argv[];		 The command line arguments
 *
 * Return Values:
 *
 * 0 - everything copied successfully
 * the error code of what went wrong
 *
 * Author: Keith W. Sheffield
 * Date:   02/20/2002
 *
 * Modification History:
 *
 * MM/DD/YY		 Name				Description
 * 02/27/02		 K. Sheffield		Added directory check for file names being
 *									read from stdin and copied to an ext2fs.
 *
 * 03/06/02		 K. Sheffield		Copying hard links to ext2fs correctly
 */
long
copy(int argc, char *argv[])
{
  ext2_filsys fs = NULL;
  ext2_ino_t root;
  ext2_ino_t orig_cwd = -1;
  ext2_ino_t cwd;
  char tmpbuf[BUF_SIZE];
  char outpath[BUF_SIZE];
  char *cur_filesys = NULL;
  int retval;
  int c;
  char *src_dir = NULL;
  char *dest_dir = NULL;
  char *in_file = NULL;
  char *out_file = NULL;
  int max_out_len = -1;
  int keep = 0;
  int absolute = 0;
  int verbose = 0;
  FS_CAT_T src_category = NONE;
  FS_CAT_T dst_category = NONE;
  char **cur_file_name;    
  int (*read_input)(char *buf) = read_line;
  int errcnt = 0;
  int num_files;
  char *ptr;
  int non_directory = 0;
  struct stat statbuf;
  struct stat def_stat;
  INODE_XREF_T *xref;
  int tmp_val;
  
  init_stat_buf(&def_stat);
  
#ifdef HAVE_OPTRESET
  optreset = 1;		/* Makes BSD getopt happy */
#endif
  while ((c = getopt(argc, argv, "0ad:G:O:pP:s:v")) != EOF)
    {      
      switch (c)
        {
        case '0':
          read_input = read_string;
          break;
        case 'a':
          absolute = 1;
          break;
        case 'd':
          dest_dir = optarg;
          break;
        case 'G':
          def_stat.st_gid = atoi(optarg);
          def_stat.st_ino = 1;
          break;
        case 'O':
          def_stat.st_uid = atoi(optarg);
          def_stat.st_ino = 1;
          break;
        case 'P':
          def_stat.st_mode = strtol(optarg, NULL, 8);
          def_stat.st_ino = 1;
          break;
        case 'p':
          keep = 1;
          def_stat.st_ino = 1;
          break;
        case 's':
          src_dir = optarg;
          break;
        case 'v':
          verbose = 1;
          break;
        default:
          errcnt++;
          break;
        }
    }

  if (errcnt)
    {
      fputs(USAGE, stderr);
      return(1);
    }

  /* if the source directory has been specified, go to it */
  if (src_dir != NULL)
    {
      /* check to see if the destination directory is an ext2fs */
      if (NULL != (ptr = strchr(src_dir, ':')))
        {
          *ptr++ = '\0';
          cur_filesys = src_dir;
          src_dir = ptr;
          src_category = EXT2_FS;
          if ((retval = open_filesystem(cur_filesys, &fs, &root, 0)))
            {
              fprintf(stderr, "%s\n", error_message(retval));
              return retval;
            }
          orig_cwd = root;
          cwd = root;
          
          if ((*src_dir != '\0' &&
               strcmp(src_dir, ".") != 0 &&
               strcmp(src_dir, "/") != 0) &&
              (retval = change_cwd(fs, root, &cwd, src_dir)))
            {
              fprintf(stderr, "Error changing to input directory %s\n",
                      src_dir);
              return(-1);
            }
          
          src_category = EXT2_FS;
        }
      else
        {                       /* deal with a normal host directory  */
          if (chdir(src_dir))
            {
              perror(src_dir);
              fprintf(stderr, "Error changing to input directory %s\n",
                      src_dir);
              return(-1);
            }
          src_category = HOST_FS;
        }
      /* get rid of any trailing /'s */
      tmp_val = strlen(src_dir) - 1;
      if (src_dir[tmp_val] == '/')
        src_dir[tmp_val] = '\0';
      
    }

  /* open the destination directory */
  if (dest_dir != NULL)
    {
      non_directory = 0;
      if ((retval = open_destination(&dest_dir, &cur_filesys, &fs, &root,
                                     &cwd, outpath, &absolute, &dst_category,
                                     &non_directory, &def_stat)))
        {
          fprintf(stderr, "Error opening destination %s:%s\n", cur_filesys,
                  dest_dir);
          if (fs)
            ext2fs_close(fs);
          return(-1);
        }

      orig_cwd = cwd;
      max_out_len = strlen(outpath);
      out_file = outpath + max_out_len;
      if (max_out_len > 0)
        {
          max_out_len++;
          *out_file++ = '/';
        }
      
      max_out_len = BUF_SIZE - max_out_len - 1;
    }

  if (src_category != NONE && src_category == dst_category)
    {
      fputs("The source and destination must be native and ext2fs\n", stderr);
      if (fs)
        ext2fs_close(fs);
      return(-1);
    }

  /* copy any remaining files */
  if (argc > optind)
    {
      num_files = argc - optind;
      if (dest_dir == NULL)
        {
          dest_dir = argv[optind + (--num_files)];
          if (num_files < 1)
            {
              fputs(USAGE, stderr);
              return(1);
            }
          non_directory = (num_files == 1);
          if ((retval = open_destination(&dest_dir, &cur_filesys, &fs, &root,
                                         &cwd, outpath, &absolute,
                                         &dst_category, &non_directory,
                                         &def_stat))) 
            {
              fprintf(stderr, "Error opening destination %s:%s\n", cur_filesys,
                      dest_dir);
              if (fs)
                ext2fs_close(fs);
              return(-1);
            }
          
          orig_cwd = cwd;
          max_out_len = strlen(outpath);
          out_file = outpath + max_out_len;
          if (max_out_len > 0 && non_directory == 0)
            {
              max_out_len++;
              *out_file++ = '/';
            }
          max_out_len = BUF_SIZE - max_out_len - 1;
        } /* src_dir & dest_dir == NULL? */

      if (num_files > 1 && dest_dir != NULL)
        qsort(argv+optind, num_files, sizeof(char *), my_strcmp);
      
      cur_file_name = argv + optind;

      if ((retval = copy_files(num_files, cur_file_name, dest_dir, cur_filesys,
                               &fs, &root, orig_cwd, outpath, out_file,
                               max_out_len, src_category, dst_category,
                               absolute, keep, non_directory, verbose,
                               &def_stat)))
        {
          fputs("Error encountered copying files\n", stderr);
          if (fs)
            ext2fs_close(fs);
          return(-1);
        }
    }
  else
    {
    /******************************************************************/
    /*            read from standard input                            */
    /******************************************************************/
                                                                             
      if (src_dir == NULL && dest_dir == NULL)
        {
          fputs("No input source or destination selected\n", stderr);
          if (fs)
            ext2fs_close(fs);
          return(-1);
        }

      if (dest_dir == NULL && dst_category == NONE)
        {
          dest_dir = outpath;
          out_file = outpath;
          max_out_len = BUF_SIZE - 1;
        }

      if (dst_category == EXT2_FS)
        {
          strncpy(cur_out_dir, (dest_dir) ? dest_dir : ".", BUF_SIZE);
          cur_out_dir[BUF_SIZE-1] = '\0';
        }

      while (-1 != (c = (read_input)(tmpbuf)))
        {
          if (c < 1)
            continue;
          
          in_file = tmpbuf;

          if (verbose)
              init_progress(in_file, &statbuf);
          
          if (dst_category == EXT2_FS)
            {
              if (stat(in_file, &statbuf) != 0)
                {
                  perror(in_file);
                  continue;
                }
              
              if (S_ISDIR(statbuf.st_mode))
                {
                  if (absolute &&
                      (retval = dir_changed(in_file, fs, root,
                                            orig_cwd, &cwd,
                                            (keep)?&statbuf:&def_stat,
                                            dest_dir)))
                    {
                      fprintf(stderr,"Error creating output directory %s\n",
                              dest_dir);
                      if (fs)
                        ext2fs_close(fs);
                      return(-1);
                    }
                  continue;
                }

              /* can't handle anything other than regular files right now */
              if (!S_ISREG(statbuf.st_mode))
                continue;

              
              if (NULL != (ptr = strrchr(in_file, '/')))
                {
                  *ptr = '\0';
                  
                  if (absolute &&
                      (retval = dir_changed(in_file, fs, root, orig_cwd,
                                            &cwd, &def_stat, dest_dir)))
                    {
                      fprintf(stderr,"Error creating output directory %s\n",
                              dest_dir);
                      if (fs)
                        ext2fs_close(fs);
                      return(-1);
                    }
              
                  *ptr = '/';
                  out_file = ptr + 1;
                } /* partial path included ? */
              else
                {
                  cwd = orig_cwd;
                  out_file = in_file;
                }
              
              if (statbuf.st_nlink > 1)
                {
                  if (NULL != (xref = find_link(&statbuf)))
                    {
                    if ((retval = create_hard_link(fs, cwd, xref->new_ino,
                                                   out_file, E2T_FORCE)))
                        {
                        fprintf(stderr, "Error creating link for %s/%s\n",
                                cur_out_dir, out_file);
                        if (fs)
                            ext2fs_close(fs);
                        elist_free(link_list, free);
                        return(1);
                        }
                    if (verbose)
                      fprintf(stderr, "Copied %s to %s:%s\n", in_file,
                              cur_filesys, cur_out_dir);
                    continue;
                    }
                }
              if ((retval = cp_to_ext2fs(fs, cwd, in_file, out_file,
                                         &statbuf, keep, &def_stat)))
                {
                  fprintf(stderr, "Error copying file %s to %s:%s\n",
                          in_file, cur_filesys, dest_dir);
                  if (fs)
                    ext2fs_close(fs);
                  return retval;
                }
              if (verbose)
                fprintf(stderr, "Copied %s to %s:%s\n", in_file, cur_filesys,
                        cur_out_dir);
              
            }
          else
            { /* copy to the local file system */
              
              /* get the basename of the file */
              if (NULL != (ptr = strrchr(in_file, '/')))
                ++ptr;
              else
                ptr = in_file;
              
              if (dest_dir != NULL)
                {    
                  /* create output file name */
                  strncpy(out_file, ptr, max_out_len);
                  outpath[BUF_SIZE-1] = '\0';
                }
              
              if ((retval = get_file(fs, root, cwd, in_file,
                                     (dest_dir == NULL) ? NULL:outpath, keep)))
                {
                  fprintf(stderr, "Error copying file %s to %s\n",
                          in_file, outpath);
                  if (fs)
                    ext2fs_close(fs);
                  return retval;
                }
              
              if (verbose)
                fprintf(stderr, "Copied %s:%s/%s to %s\n", cur_filesys,
                        src_dir, in_file,
                        (dest_dir == NULL) ? outpath : dest_dir);
            }
          
        }
    }

  if (fs)
    ext2fs_close(fs);

  return(0);      
} /* end of copy */ 

long
open_destination(char **dest_dir, char **cur_filesys, ext2_filsys *fs,
                 ext2_ino_t *root, ext2_ino_t *cwd, char *outfile,
                 int *absolute, FS_CAT_T *dst_cat, int *allow_non_dir,
                 struct stat *def_stat)
{
  char *ptr;
  struct stat	statbuf;
  char *dptr;
  int retval;
  ext2_ino_t inode;
  int new_dir = 1;
  
  if (dest_dir == NULL || cur_filesys == NULL || fs == NULL || root == NULL ||
      cwd == NULL || outfile == NULL)
    {
      fputs("Invalid parameters\n", stderr);
      return(-1);
    }
  
  dptr = *dest_dir;
  *outfile = '\0';
  
  /* check to see if the destination directory is an ext2fs */
  if (NULL != (ptr = strchr(dptr, ':')))
    {
      if (*fs != NULL)
        {
          fputs("An Ext2fs filesystem is already open!", stderr);
          return(-1);
        }
      
      *ptr++ = '\0';
      *cur_filesys = dptr;
      dptr = ptr;
      *dst_cat = EXT2_FS;
      if ((retval = open_filesystem(*cur_filesys, fs, root, 1)))
        {
          fprintf(stderr, "%s\n", error_message(retval));
          return retval;
        }
      
      *cwd = *root;
      
      if ((*dptr != '\0' &&
           strcmp(dptr, ".") != 0 &&
           strcmp(dptr, "/") != 0))
        {
          
          if (*allow_non_dir == 1)
            {
              ptr = NULL;
              new_dir = 0;
              /* check to see if it exists */
              if ((retval = ext2fs_namei(*fs, *root, *cwd, dptr, &inode)) ||
                  (retval = ext2fs_check_directory(*fs, inode)))
                {
                  if ((retval != EXT2_ET_FILE_NOT_FOUND) &&
                      (retval != EXT2_ET_NO_DIRECTORY))
                    {
                      fprintf(stderr, "%s\n", error_message(retval));
                      return(retval);
                    }
                  
                  /* ok, so it doesn't exist or isn't a directory
                   *...let's see if it's parent does
                   */
                  if (NULL != (ptr = strrchr(dptr, '/')))
                    {
                      *ptr = '\0';
                      if (ptr[1] == '\0')
                        *allow_non_dir = 0;
                      new_dir = 1;
                      
                      if ((retval = ext2fs_namei(*fs, *root, *cwd, dptr,
                                                 &inode)))
                        {
                          if (retval != EXT2_ET_FILE_NOT_FOUND)
                            {
                              fprintf(stderr, "%s\n", error_message(retval));
                              return(retval);
                            }
                          /* it doesn't exist either */
                        }
                      else
                        {
                          if ((retval = ext2fs_check_directory(*fs, inode)))
                            {
                              /* ok, it exists, but it's not a directory... */
                              fprintf(stderr, "%s\n", error_message(retval));
                              return(retval);
                            }
                          *cwd = inode;
                          new_dir = 0;
                        }
                    }
                }
              else
                {
                  /* ok, the directory exists */
                  *cwd = inode;
                  *allow_non_dir = 0;
                }
            }

          if (new_dir == 1 &&
              (retval = create_dir(*fs, *root, cwd, dptr, def_stat)))
            {
              fprintf(stderr, "Error creating output directory %s\n",
                      dptr);
              return(-1);
            }
          /* see if we need to copy the file into the output path */
          if (*allow_non_dir == 1)
            {
              strncpy(outfile, (ptr != NULL) ? ++ptr : dptr, BUF_SIZE);
              outfile[BUF_SIZE-1] = '\0';
            }
        }
      else
        {
          if (*allow_non_dir == 1)
            {
              *allow_non_dir = 0;
              dptr = NULL;
            }
        }
    }
  else
    {                   /* deal with a normal host directory  */
      *absolute = 0;
      if (strcmp(dptr, "-") == 0)
        dptr = NULL;
      else
        {
          strncpy(outfile, dptr, BUF_SIZE);
          outfile[BUF_SIZE-1] = '\0';
          
          if (0 > stat(outfile, &statbuf) ||
              !S_ISDIR(statbuf.st_mode))
            {
              if (*allow_non_dir == 0)
                {
                  fprintf(stderr, "%s is not a directory!\n", outfile);
                  return(-1);
                }
            }
          else
            *allow_non_dir = 0;
        }
      
      *dst_cat = HOST_FS;
    }
  *dest_dir = dptr;
  return(0);
}

long
copy_files(int num_files, char **cur_file_names, char *dest_dir, char *dest_fs,
           ext2_filsys *fs, ext2_ino_t *root, ext2_ino_t orig_cwd,
           char *outpath, char *out_file, int max_file_len,
           FS_CAT_T src_category, FS_CAT_T dst_category, int absolute,
           int keep, int file_to_file, int verbose, struct stat *def_stat)
{
  char *in_file;
  char *dst_file;
  char *ptr;
  char *last_filesys = NULL;
  char *cur_filesys = dest_fs;
  ext2_ino_t cwd = orig_cwd;
  int retval;
  int i;
  struct stat	statbuf;
  INODE_XREF_T *xref;
  
  strncpy(cur_out_dir, (dest_dir) ? dest_dir : ".", BUF_SIZE);
  cur_out_dir[BUF_SIZE-1] = '\0';

  for(i=0;i<num_files;i++)
    {
      in_file = *cur_file_names++;

      if (verbose)
          init_progress(in_file, &statbuf);
      /* dealing with a filesystem or a regular file */
      if (NULL != (ptr = strchr(in_file, ':')))
        {
          if (dst_category == EXT2_FS)
            {
              fputs("Already copying to an ext2fs!\n", stderr);
              return(-1);
            }
          if (src_category == EXT2_FS)
            {
              fputs("The source is already an ext2fs!\n", stderr);
              return(-1);
            }
          
          /* separate the filesystem name from the file name */
          *ptr++ = '\0';
          cur_filesys = in_file;
          in_file = ptr;
          
          if (dest_dir != NULL)
            {
              if (!file_to_file)
                {
                  /* get the basename of the file */
                  if (NULL != (ptr = strrchr(in_file, '/')))
                    ++ptr;
                  else
                    ptr = in_file;
                  
                  /* create output file name */
                  strncpy(out_file, ptr, max_file_len);
                  outpath[BUF_SIZE-1] = '\0';
                }
            }
          
          /* check to see if the filesystem has changed */
          if (last_filesys == NULL ||
              strcmp(last_filesys, cur_filesys) != 0)
            {
              if (last_filesys != NULL &&
                  (retval = ext2fs_close(*fs)))
                {
                  fprintf(stderr, "%s\n", error_message(retval));
                  return retval;
                }
              
              if ((retval = open_filesystem(cur_filesys, fs, root, 0)))
                  {
                  fprintf(stderr, "%s\n", error_message(retval));
                  fprintf(stderr, "Error opening fileystem %s\n",
                          cur_filesys);
                  return retval;
                }
              cwd = *root;
              last_filesys = cur_filesys;
            } /* end of filesystem change? */
          
          if ((retval = get_file(*fs, *root, cwd, in_file,
                                 (dest_dir == NULL) ? NULL:outpath, keep)))
            {
              fprintf(stderr, "Error copying file %s to %s\n",
                      in_file, outpath);
              return retval;
            }
        } /* end of do we have a filesystem spec? */
      else
        {      
          if (dst_category == HOST_FS)
            {
              fputs("Already copying to a native filesystem!\n",
                    stderr);
              continue;
            }

          
          memset(&statbuf, 0, sizeof(statbuf));
          if (strcmp(in_file, "-") == 0)
            {
              statbuf.st_mode = S_IFREG;
              in_file = NULL;
            }
          
          if (in_file != NULL && stat(in_file, &statbuf) != 0)
            {
              perror(in_file);
              continue;
            }

          /* can't handle anything other than regular files right now */
          if (!S_ISREG(statbuf.st_mode))
            continue;

          if (file_to_file)
            dst_file = outpath;
          else
            dst_file = in_file;
          
          if (NULL != (ptr = strrchr(dst_file, '/')))
            {
              *ptr = '\0';
                  
              if (absolute)
                {
                  if ((retval = dir_changed(dst_file, *fs, *root, orig_cwd,
                                            &cwd, def_stat, dest_dir)))
                    {
                      fprintf(stderr,"Error creating output directory %s\n",
                              dst_file);
                      return(-1);
                    }
                }                  
              *ptr = '/';
              out_file = ptr + 1;
            }
          else
            {
              cwd = orig_cwd;
              out_file = dst_file;
            }

          if (statbuf.st_nlink > 1)
            {
              if (NULL != (xref = find_link(&statbuf)))
                {
                  if ((retval = create_hard_link(*fs, cwd, xref->new_ino,
                                                 out_file, E2T_FORCE)))
                    {
                      fprintf(stderr, "Error creating link for %s/%s\n",
                              cur_out_dir, out_file);
                      elist_free(link_list, free);
                      return(1);
                    }
                  
                  if (verbose)
                    fprintf(stderr, "Copied %s to %s:%s\n", in_file,
                            cur_filesys, diag_output_name(cur_out_dir,
                                                          file_to_file,
                                                          outpath));
                  
                  continue;
                }
            }

          if ((retval = cp_to_ext2fs(*fs, cwd, in_file, out_file, &statbuf,
                                     keep, def_stat)))
            {
              fprintf(stderr, "Error copying file %s to %s:%s\n",
                      (in_file) ? in_file : "<stdin>" , cur_filesys,
                      diag_output_name(cur_out_dir, file_to_file, outpath));
              return retval;
            }
          if (verbose)
            fprintf(stderr, "Copied %s to %s:%s\n",
                    (in_file) ? in_file : "<stdin>", cur_filesys, 
                    diag_output_name(cur_out_dir, file_to_file, outpath));
        }
    }
  
  return(0);
}

static char *
diag_output_name(char *odir, int file_to_file, char *ofile)
{
  static char tmpstr[BUF_SIZE];
  if (file_to_file)
    {
      snprintf(tmpstr, BUF_SIZE, "%s/%s", odir, ofile);
      tmpstr[BUF_SIZE-1] = '\0';
      return(tmpstr);
    }
  else
    return(odir);
}


long
dir_changed(char *newdir, ext2_filsys fs, ext2_ino_t root, ext2_ino_t cwd,
            ext2_ino_t *newcwd, struct stat *def_stat, char *dest_dir)
{
  static char last_cwd[BUF_SIZE];
  static int first = 1;
  long retval;
  
  
  if (first || strcmp(last_cwd, newdir))
    {
      first = 0;
      
      *newcwd = cwd;
      if ((*newdir != '\0' &&
           strcmp(newdir, ".") != 0) &&
          (retval = create_dir(fs, root, newcwd, newdir, def_stat)))
        {
          fprintf(stderr,"Error creating output "
                  "directory %s\n", newdir);
          return(-1);
        }
      strncpy(last_cwd, newdir, BUF_SIZE);
      last_cwd[BUF_SIZE-1] = '\0';
      
      if (*newdir == '/' || dest_dir == NULL || *dest_dir == '\0')
          strncpy(cur_out_dir, newdir, BUF_SIZE);
      else
          snprintf(cur_out_dir, BUF_SIZE, "%s/%s", dest_dir, newdir);

      cur_out_dir[BUF_SIZE-1] = '\0';
    }
  return(0);
}


int
read_line(char *inbuf)
{
  char *ptr;
  char c;

  *inbuf = '\0';
  
  if (NULL == fgets(inbuf, BUF_SIZE, stdin))
    {
      if (ferror(stdin))
        perror("read_line");
      return(-1);
    }

  ptr = inbuf + strlen(inbuf);
  ptr--;
  
  while (ptr >= inbuf)
    {
      c = *ptr;
      if (!ISSPACE(c))
        break;
      ptr--;
    }
  ptr++;
  *ptr = '\0';
  return(strlen(inbuf));
}

int
read_string(char *inbuf)
{
  char *ptr;
  char *boundary;
  int c;

  boundary = inbuf + BUF_SIZE - 1;
  ptr = inbuf;
  *ptr = '\0';
  while ((c = getchar()) != EOF && c != '\0')
    {
      if (ptr < boundary)
        *ptr++ = c;
      else
        break;
    }
  
  if (c == EOF)
    {
      if (ferror(stdin))
        {
          perror("read_string");
          return(-1);
        }
      else if (feof(stdin))
        return(-1);
    }
  *ptr = '\0';
  return(strlen(inbuf));
}

int
my_strcmp(const void *n1, const void *n2)
{
    char *s1 = *((char **)n1);
    char *s2 = *((char **)n2);

    return(strcmp(s1, s2));
}

INODE_XREF_T *
find_link(struct stat *sbuf)
{
  elist_t *n;
  INODE_XREF_T *xref;
  
  n = link_list;

  while (n != NULL)
    {
      if ((xref = n->data) != NULL)
        {
          if (xref->dev_no > sbuf->st_dev)
              return(NULL);
          else if (xref->dev_no == sbuf->st_dev)
            {
              if (xref->ino_no > sbuf->st_ino)
                  return(NULL);
              else if (xref->ino_no == sbuf->st_ino)
                return(xref);
            }
        }
      n = n->next;
    }

  return(NULL);
}
  
long
add_link(struct stat *sbuf, ext2_ino_t newfile)
{
  INODE_XREF_T *xref;
  INODE_XREF_T *x_node;
  elist_t *n;
  
  if (NULL == (xref = malloc(sizeof(INODE_XREF_T))))
    {
      perror("malloc");
      return(1);
    }

  xref->dev_no = sbuf->st_dev;
  xref->ino_no = sbuf->st_ino;
  xref->new_ino = newfile;
  
  if (link_list == NULL)
    {
      if (NULL == (link_list = elist_new()))
        {
          perror("elist_new");
          free(xref);
          return(1);
        }
    }

  n = link_list->next;
  while (n != NULL)
    {
      if (NULL != (x_node = n->data))
        {
          if (x_node->dev_no > xref->dev_no ||
              (x_node->dev_no == xref->dev_no &&
               x_node->ino_no > xref->ino_no))
            {
              if (NULL == elist_insert(n, xref))
                {
                  perror("elist_insert");
                  free(xref);
                  return(1);
                }
              return(0);
            }
        }
      n = n->next;
    }

  if (NULL == elist_append(link_list, xref))
    {
      perror("elist_append");
      free(xref);
      return(1);
    }
  return(0);
}

static long
cp_to_ext2fs(ext2_filsys fs, ext2_ino_t cwd, char *in_file, char *out_file,
             struct stat *statbuf, int keep, struct stat *def_stat)
{
  long retval;
  ext2_ino_t out_file_ino;
  
  if ((retval = put_file(fs, cwd, in_file, out_file,
                         &out_file_ino, keep, def_stat)))
    {
      elist_free(link_list, free);
      return retval;
    }
  if (statbuf->st_nlink > 1)
    {
      if ((retval = add_link(statbuf, out_file_ino)))
        {
          fprintf(stderr, "Error adding link info for %s to list\n",
                  out_file);
          elist_free(link_list, free);
          return retval;
        }
    }
  return(0);
}
