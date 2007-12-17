/* $Header: /home/ksheff/src/e2tools/RCS/ls.c,v 0.8 2004/04/07 03:30:49 ksheff Exp $ */
/*
 * ls.c --- list directories
 * 
 * Copyright (C) 1997 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * Modified by Keith W. Sheffield <shefff@pobox.com> for inclusion with e2tools
 */
/*
 * $Log: ls.c,v $
 * Revision 0.8  2004/04/07 03:30:49  ksheff
 * Updated usage string.
 *
 * Revision 0.7  2004/04/07 02:41:50  ksheff
 * Modified to bracket files with an inode number of 0 with >< characters.
 *
 * Revision 0.6  2004/04/06 20:27:26  ksheff
 * Modified to print "No files found!" for empty directories, corrected the
 * directory name display, and fixed REGEX_OPT masking.
 *
 * Revision 0.5  2002/06/05 23:14:11  ksheff
 * Fixed short display with respect to displaying the contents of multiple
 * directories.
 *
 * Revision 0.4  2002/06/05 23:07:34  ksheff
 * Allowed for multiple directory and file specifications.  Added the -f
 * option for no sorting.
 *
 * Revision 0.3  2002/06/03 23:00:51  ksheff
 * Removed display code from directory iterator.  A list of displayable files
 * is now generated.  This list can be sorted and displayed in a variety of
 * ways.  The -a, -c, -i, -r, and -t options are now accepted.
 *
 * Revision 0.2  2002/03/05 12:12:52  ksheff
 * Removed setting optind for SCO.
 *
 * Revision 0.1  2002/02/27 04:46:21  ksheff
 * initial revision
 *
 */


#include "e2tools.h"
#include "elist.h"
#include <regex.h>
#include <stdint.h>
#include <inttypes.h>

/*
 * list directory
 */

#define LONG_OPT	0x0001
#define DELETED_OPT	0x0002
#define REGEX_OPT   0x0004
#define REVERSE_OPT 0x0008
#define HIDDEN_OPT  0x0010
#define CREATE_OPT  0x0020
#define INODE_OPT   0x0040

#define DIRECTORY_TYPE -1
#define NORMAL_TYPE 0

struct list_dir_struct
{
  int	options;
  regex_t *reg;
  elist_t *files;  
};

typedef struct list_file_struct
{
  
  char *name;
  struct ext2_inode inode;
  ext2_ino_t dir;
  ext2_ino_t inode_num;
  int entry;
  long type;
} ls_file_t;

static const char *monstr[] =
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static ext2_filsys fs;
static int max_name_len = 0;

static int
list_dir_proc(ext2_ino_t dir, int entry, struct ext2_dir_entry *dirent,
              int offset, int blocksize, char *buf, void *private);
void
free_ls_file_t(void *f);
long
do_list_dir(int argc, char *argv[]);
void
long_disp(ls_file_t *info, int *col, int options);
void
short_disp(ls_file_t *info, int *col, int options);
int
no_sort(void *n1, void *n2);
int
name_sort(void *n1, void *n2);
int
inode_sort(void *n1, void *n2);
int
mod_time_sort(void *n1, void *n2);
int
creat_time_sort(void *n1, void *n2);
long
add_ls_file(char *name, int namelen, ext2_ino_t dir, ext2_ino_t ino,
            int entry, int type, struct list_dir_struct *ls);
elist_t *
remove_ls_dups(elist_t *list);

/* Name:	list_dir_proc()
 *
 * Description:
 *
 *
 * Algorithm:
 *
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 *
 * Return Values:
 *
 *
 * Author: Theodore Ts'o
 * Date:   1997
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 02/27/02      K.Sheffield        Modified for use with e2tools
 */
static int
list_dir_proc(ext2_ino_t dir, int entry, struct ext2_dir_entry *dirent,
              int UNUSED_PARM(offset), int UNUSED_PARM(blocksize),
	      char UNUSED_PARM(*buf), void *private)
{
  char			name[EXT2_NAME_LEN];
  struct list_dir_struct *ls = (struct list_dir_struct *) private;
  int thislen;
  
  thislen = ((dirent->name_len & 0xFF) < EXT2_NAME_LEN) ?
    (dirent->name_len & 0xFF) : EXT2_NAME_LEN;
  strncpy(name, dirent->name, thislen);
  name[thislen] = '\0';

  /* skip hidden files unless we ask for them */
  if (0 == (ls->options & HIDDEN_OPT) &&
      name[0] == '.')
    return(0);
  
  if ((ls->options & REGEX_OPT) &&
      regexec(ls->reg, name, 0, NULL, 0))
    return(0);

  return(add_ls_file(name, thislen, dir, dirent->inode, entry, NORMAL_TYPE,
                     ls));
  
}

/* Name:	add_ls_file()
 *
 * Description:
 *
 *
 * Algorithm:
 *
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 *
 * Return Values:
 *
 *
 * Author: K.Sheffield
 * Date:   02/27/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 */
long
add_ls_file(char *name, int namelen, ext2_ino_t dir, ext2_ino_t ino,
            int entry, int type, struct list_dir_struct *ls)
{
  ls_file_t *file_info;
  elist_t *flist;
    
  if (NULL == (file_info = calloc(sizeof(ls_file_t),1)))
    {
      perror("list_dir");
      return(0);
    }

  file_info->dir = dir;
  
  if (entry == DIRENT_DELETED_FILE)
    file_info->inode_num = 0;
  else
    file_info->inode_num = ino;

  file_info->entry = entry;
  file_info->type = type;
  
  if (file_info->inode_num)
    {
    if (read_inode(fs, file_info->inode_num, &file_info->inode))
        {
        free(file_info);
        return 0;
        }
    }

  if (name)
    file_info->name = strdup(name);
  
  if (NULL == (flist = elist_insert(ls->files, file_info)))
    {
      perror("list_dir");
      free_ls_file_t(file_info);
      return 0;
    }
  ls->files = flist;

  if (max_name_len < namelen)
    max_name_len = namelen;
  
  return 0;
}

/* Name:	free_ls_file_t()
 *
 * Description:
 *
 * This function is used to free an ls_file_t structure.
 *
 * Algorithm:
 *
 * Free the file's name if it is not NULL
 * Free the ls_file_t structure
 * Check to make sure the ls_file_t structure is not NULL
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * void *f;                The structure to free
 *
 * Return Values:
 *
 * none
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
void free_ls_file_t(void *f)
{
  ls_file_t *n = (ls_file_t *) f;
  
  if (n != NULL)
    {
      if (n->name != NULL)
        free(n->name);
      free(n);
    }
} /* end of free_ls_file_t */ 

/* Name:	do_list_dir()
 *
 * Description:
 *
 *
 * Algorithm:
 *
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 *
 * Return Values:
 *
 *
 * Author: Theodore Ts'o
 * Date:   1997
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 02/27/02      K.Sheffield        Modified for use with e2tools
 * 04/06/04      K.Sheffield        Modified to print "No Files Found!" for
 *                                  each empty directory.  Corrected masking
 *                                  out REGEX_OPT.
 */

long
do_list_dir(int argc, char *argv[])
{
  ext2_ino_t	root;
  ext2_ino_t	cwd;
  ext2_ino_t	inode=0;
  int		retval;
  int		c;
  int		flags;
  struct list_dir_struct ls;
  char *fs_name;
  char *last_fs_name;
  char *path = NULL;
  char *dup_path = NULL;
  char *dir_name;
  char *base_name;
  int (*file_sort)(void *n1, void *n2) = name_sort;
  void (*file_disp)(ls_file_t *n, int *col, int options) = short_disp;
  elist_t *files=NULL;
  int col=0;
  ls_file_t *cur_file;
  long last_type = 1;
  
  memset(&ls, 0, sizeof(ls));

  last_fs_name = NULL;
#ifdef HAVE_OPTRESET
  optreset = 1;		/* Makes BSD getopt happy */
#endif
  while ((c = getopt (argc, argv, "acDd:filrt")) != EOF)
    {
      switch (c)
        {
        case 'a':
          ls.options |= HIDDEN_OPT;
          break;
        case 'c':
          ls.options |= CREATE_OPT;
          break;
        case 'l':
          file_disp = long_disp;
          break;
        case 'D':
          ls.options |= DELETED_OPT;
          break;
        case 'd':
          fs_name = optarg;
          if (NULL != (path = strchr(fs_name, ':')))
            *path++ = '\0';
          if ((retval = open_filesystem(fs_name, &fs, &root, 0)))
            {
              fputs(error_message(retval), stderr);
              return(1);
            }
          last_fs_name = fs_name;
          break;
        case 'f':
          file_sort = no_sort;
          break;
        case 't':
          file_sort = mod_time_sort;
          break;
        case 'r':
          ls.options |= REVERSE_OPT;
          break;
        case 'i':
          file_sort = inode_sort;
          ls.options |= INODE_OPT;
          break;
        }
    }

  if (argc <= optind)
    {
      fputs("Usage: e2ls [-acDfilrt][-d dir] file\n", stderr);
      return(1);
    }

  if (ls.options & CREATE_OPT &&
      (file_sort == mod_time_sort || file_disp != long_disp))
    file_sort = creat_time_sort;

  /* sort the remaining command line arguments */
  qsort(argv+optind, argc-optind, sizeof(char *), my_strcmp);
  
  for(c=optind;c<argc;c++)
    {
      fs_name = argv[c];
      
      if (NULL != (path = strchr(fs_name, ':')))
        *path++ = '\0';
      else if (last_fs_name != NULL)
        {
        path = fs_name;
        fs_name = last_fs_name;
        }

      /* keep a copy of the file path for later because get_file_parts() is
       * destructive.
       */
      
      if (dup_path)
        {
          free(dup_path);
          dup_path = NULL;
        }

      if (path)
        dup_path = strdup(path);
      
      if (last_fs_name == NULL || strcmp(last_fs_name, fs_name))
        {
          if (last_fs_name != NULL)
            ext2fs_close(fs);
              
          if ((retval = open_filesystem(fs_name, &fs, &root, 0)))
            {
              fputs(error_message(retval), stderr);
              return(1);
            }
          last_fs_name = fs_name;
        }

      dir_name = NULL;
      cwd = root;
      ls.options &= (~REGEX_OPT);

      if (path != NULL && *path != '\0')
        {
          if (get_file_parts(fs, root, path, &cwd, &dir_name, &base_name))
            {
              ext2fs_close(fs);
              return(-1);
            }

          if (is_file_regexp(base_name))
            {
              if (NULL == (ls.reg = (regex_t *) make_regexp(base_name)))
                {
                  fprintf(stderr,
                          "Error creating regular expression for %s\n",
                          base_name);
                  return(1);
                }
              ls.options |= REGEX_OPT;
              inode = cwd;

            }
          /* check to see if the file name exists in the current directory
           */
          else if ((retval = ext2fs_namei(fs, cwd, cwd, base_name, &inode)))
            {
              fputs(error_message(retval), stderr);
              ext2fs_close(fs);
              return(1);
            }
        }
      else
          inode = root;
      
      if(!dir_name)
        dir_name = ".";
            
      if (!inode)
        continue;
            
      flags = DIRENT_FLAG_INCLUDE_EMPTY;
      if (ls.options & DELETED_OPT)
        flags |= DIRENT_FLAG_INCLUDE_REMOVED;
            
      if ((retval = ext2fs_check_directory(fs, inode)))
        {
        if (retval != EXT2_ET_NO_DIRECTORY)
            {
              fputs(error_message(retval), stderr);
              ext2fs_close(fs);
              return(1);
            }
        
        if(add_ls_file(dir_name, 0, cwd, 0, 0, DIRECTORY_TYPE, &ls))
          {
            fputs(error_message(retval), stderr);
            ext2fs_close(fs);
            return(1);
          }
        
        if(add_ls_file(base_name, strlen(base_name), cwd, inode, 0,
                       NORMAL_TYPE, &ls))
          {
            fputs(error_message(retval), stderr);
            ext2fs_close(fs);
            return(1);
          }
        }
      else
        {
          if (ls.options & REGEX_OPT || path == NULL || *path == '\0')
            path = dir_name;
          else if (dup_path)
            path = dup_path;

          
          //        if(add_ls_file((ls.options & REGEX_OPT) ? dir_name : path, 0, inode, 0,
          if(add_ls_file(path, 0, inode, 0, 0, DIRECTORY_TYPE, &ls))
            {
              fputs(error_message(retval), stderr);
              ext2fs_close(fs);
              return(1);
            }
          
          retval = ext2fs_dir_iterate2(fs, inode, flags, 0, list_dir_proc,
                                       &ls);
          if (retval)
            {
              fputs(error_message(retval), stderr);
              ext2fs_close(fs);
              return(1);
            }
        }
    }
  

  elist_sort(ls.files, file_sort, ls.options & REVERSE_OPT);
  
  ls.files = files = remove_ls_dups(ls.files);
  
  if (files == NULL)
    printf("No files found!");
  else
    {
      while(files != NULL)
        {
          cur_file = (ls_file_t *)files->data;
          if (cur_file->type == DIRECTORY_TYPE)
            {
              if (col > 0)
                {
                  putchar('\n');
                  col = 0;
                }
              if (last_type == DIRECTORY_TYPE)
                printf("No files found!\n");
              
              printf("%s:", cur_file->name);
            }
          else
            {
              if (last_type == DIRECTORY_TYPE)
                putchar('\n');
              (file_disp)(cur_file, &col, ls.options);
            }
          
          last_type = cur_file->type;
          
          files = files->next;
        }
      if (last_type == DIRECTORY_TYPE)
        printf("No files found!");
    }
  
  putchar('\n');

  elist_free(ls.files, free_ls_file_t);
    
  ext2fs_close(fs);
  return(0);
}

/* Name:	long_disp()
 *
 * Description:
 *
 * This function displays a file's information for a long listing
 *
 * Algorithm:
 *
 * Generate the time string for the modification time
 * Display the file's permission bits, owner, group, mod time, and name
 *
 * Global Variables:
 *
 * none
 *
 * Arguments:
 *
 * ls_file_t *info;             The structure containing the file information
 * int *col;                    The current column - unused
 * int options;                 Options to ls
 *
 * Return Values:
 *
 * None
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 06/06/02      K.Sheffield        Increased file size width
 * 04/06/04      K.Sheffield        Modified to show entries with an inode of 0
 *                                  as deleted.
 */
void long_disp(ls_file_t *info, int UNUSED_PARM(*col), int options)
{
  char lbr, rbr;
  char datestr[80];
  time_t modtime;
  struct tm *tm_p;

  
  if (info->entry == DIRENT_DELETED_FILE ||
      info->inode_num == 0)
    {
      lbr = '>';
      rbr = '<';
	}
  else
    {
		lbr = rbr = ' ';
	}

  
  if (info->inode_num)
    {
      if (options & CREATE_OPT)
        modtime = info->inode.i_ctime;
      else
        modtime = info->inode.i_mtime;
      tm_p = localtime(&modtime);
      sprintf(datestr, "%2d-%s-%4d %02d:%02d",
              tm_p->tm_mday, monstr[tm_p->tm_mon],
              1900 + tm_p->tm_year, tm_p->tm_hour,
              tm_p->tm_min);
    }
  else
    strcpy(datestr, "                 ");
      
  printf("%c%6u%c %6o %5d %5d  ", lbr, info->inode_num, rbr,
         info->inode.i_mode, info->inode.i_uid, info->inode.i_gid);
  if (LINUX_S_ISDIR(info->inode.i_mode))
    printf("%7d", info->inode.i_size);
  else
    printf("%7" PRIu64, (uint64_t)(info->inode.i_size |
				  ((__u64)info->inode.i_size_high << 32)));
  printf(" %s %s\n", datestr, info->name);

} /* end of long_disp */ 


/* Name:	short_disp()
 *
 * Description:
 *
 * This function displays a file's information for a long listing
 *
 * Algorithm:
 *
 * Display the file's name at the appropriate column.
 *
 * Global Variables:
 *
 * none
 *
 * Arguments:
 *
 * ls_file_t *info;             The structure containing the file information
 * int *col;                    The current column
 * int options;                 Options to ls
 *
 * Return Values:
 *
 * None
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 04/06/04      K.Sheffield        Modified to show entries with an inode of 0
 *                                  as deleted.
 */
void short_disp(ls_file_t *info, int *col, int options)
{
  char lbr, rbr;
  char tmp[300];
  int thislen;
  static int max_col_size = 0;

  if (max_col_size == 0)
    {
      max_col_size = 80/(max_name_len + 2 + ((options & INODE_OPT) ? 8 : 0));
      if (max_col_size == 0)
        max_col_size = -1;
      else
        max_col_size = 80/max_col_size;
    }
  
      
  if (info->entry == DIRENT_DELETED_FILE ||
      info->inode_num == 0)
    {
      lbr = '>';
      rbr = '<';
	}
  else
    {
      lbr = 0;
      rbr = ' ';
	}

  if (lbr == 0)
    {
      if (options & INODE_OPT)    
        sprintf(tmp, "%7d %s%c", info->inode_num, info->name, rbr);
      else
        sprintf(tmp, "%s%c", info->name, rbr);
    }
  else
    {
      if (options & INODE_OPT)    
        sprintf(tmp, "%7d %c%s%c", info->inode_num, lbr, info->name, rbr);
      else
        sprintf(tmp, "%c%s%c", lbr, info->name, rbr);
    }
  
  thislen = strlen(tmp);

  if (*col + thislen > 80)
    {
      putchar('\n');
      *col = 0;
    }
  thislen = max_col_size - thislen;
  if (thislen < 0)
    thislen = 0;
  
  printf("%s%*.*s", tmp, thislen, thislen, "");
  *col += max_col_size;
}

/* Name:	no_sort()
 *
 * Description:
 *
 * This function sorts two ls_file_t structures by the file name
 *
 * Algorithm:
 *
 * Assign two ls_file_t pointers from the input void pointers
 * Return the result of the comparison of the directories & type
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * void *n1;                     The first node being compared
 * void *n2;                     The second node being compared
 *
 * Return Values:
 *
 * >0 - n1 > n2
 * =0 - n1 == n2
 * <0 - n1 < n2
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
int no_sort(void *n1, void *n2)
{
  ls_file_t *f1 = *((ls_file_t **) n1);
  ls_file_t *f2 = *((ls_file_t **) n2);
  int retval;
  
  return((retval = (f1->dir - f2->dir)) ? retval : (f1->type - f2->type));
  
} /* end of name_sort */

/* Name:	name_sort()
 *
 * Description:
 *
 * This function sorts two ls_file_t structures by the file name
 *
 * Algorithm:
 *
 * Assign two ls_file_t pointers from the input void pointers
 * Return the result of the comparison of the names
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * void *n1;                     The first node being compared
 * void *n2;                     The second node being compared
 *
 * Return Values:
 *
 * >0 - n1 > n2
 * =0 - n1 == n2
 * <0 - n1 < n2
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
int name_sort(void *n1, void *n2)
{
  ls_file_t *f1 = *((ls_file_t **) n1);
  ls_file_t *f2 = *((ls_file_t **) n2);
  int retval;
  
  return((retval = (f1->dir - f2->dir)) ? retval :
         ((retval = (f1->type - f2->type)) ? retval :
          strcmp(f1->name, f2->name)));
} /* end of name_sort */

/* Name:	inode_sort()
 *
 * Description:
 *
 * This function sorts two ls_file_t structures by the file inode number
 *
 * Algorithm:
 *
 * Assign two ls_file_t pointers from the input void pointers
 * Return the result of the comparison of the inode numbers
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * void *n1;                     The first node being compared
 * void *n2;                     The second node being compared
 *
 * Return Values:
 *
 * >0 - n1 > n2
 * =0 - n1 == n2
 * <0 - n1 < n2
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
int inode_sort(void *n1, void *n2)
{
  ls_file_t *f1 = *((ls_file_t **) n1);
  ls_file_t *f2 = *((ls_file_t **) n2);
  int retval;
  
  return((retval = (f1->dir - f2->dir)) ? retval :
         ((retval = (f1->type - f2->type)) ? retval :
          (int)(f1->inode_num - f2->inode_num)));
} /* end of inode_sort */ 

/* Name:	mod_time_sort()
 *
 * Description:
 *
 * This function sorts two ls_file_t structures by the file modification time
 *
 * Algorithm:
 *
 * Assign two ls_file_t pointers from the input void pointers
 * Return the result of the comparison of the modification time
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * void *n1;                     The first node being compared
 * void *n2;                     The second node being compared
 *
 * Return Values:
 *
 * >0 - n1 > n2
 * =0 - n1 == n2
 * <0 - n1 < n2
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
int mod_time_sort(void *n1, void *n2)
{
  ls_file_t *f1 = *((ls_file_t **) n1);
  ls_file_t *f2 = *((ls_file_t **) n2);
  int retval;
  
  return((retval = (f1->dir - f2->dir)) ? retval :
         ((retval = (f1->type - f2->type)) ? retval :
          (int)(f2->inode.i_mtime - f1->inode.i_mtime)));
  
} /* end of mod_time_sort */

/* Name:	creat_time_sort()
 *
 * Description:
 *
 * This function sorts two ls_file_t structures by the file creation time
 *
 * Algorithm:
 *
 * Assign two ls_file_t pointers from the input void pointers
 * Return the result of the comparison of the creation time
 *
 * Global Variables:
 *
 * None
 *
 * Arguments:
 *
 * void *n1;                     The first node being compared
 * void *n2;                     The second node being compared
 *
 * Return Values:
 *
 * >0 - n1 > n2
 * =0 - n1 == n2
 * <0 - n1 < n2
 *
 * Author: Keith W. Sheffield
 * Date:   06/03/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
int creat_time_sort(void *n1, void *n2)
{
  ls_file_t *f1 = *((ls_file_t **) n1);
  ls_file_t *f2 = *((ls_file_t **) n2);
  int retval;
  
  return((retval = (f1->dir - f2->dir)) ? retval :
         ((retval = (f1->type - f2->type)) ? retval :
          (int)(f2->inode.i_ctime - f1->inode.i_ctime)));
} /* end of creat_time_sort */ 

/* Name:	remove_ls_dups()
 *
 * Description:
 *
 * This function will remove the first directory node if it is the only one
 * found.  It will also remove bogus entries.
 *
 * Algorithm:
 *
 * For each node in the linked list
 *     If the current node has no data
 *        Remove it from the linked list
 *     If the current entry is a DIRECTORY_TYPE
 *        Increment the number of directories.
 * Return the starting node in the linked list
 *
 * Global Variables:
 *
 * None.
 *
 * Arguments:
 *
 * elist *list;                 The linked list to process
 *
 * Return Values:
 *
 * The starting node in the linked list.
 *
 * Author: Keith W. Sheffield
 * Date:   06/05/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 * 04/06/04      K.Sheffield        Modified to just remove the first directory
 *                                  node if it is the only one found.
 *
 */
elist_t * remove_ls_dups(elist_t *list)
{
  elist_t *start = list;
  ls_file_t *cd;
  int cnt=0;

  while(list != NULL)
    {
      cd = (ls_file_t *) list->data;
      if (cd == NULL)
        {
          /* remove any empty nodes */
          if (list == start)
            start = list->next;
          list = elist_delete(list, free_ls_file_t);
          continue;
        }
      else if (cd->type == DIRECTORY_TYPE)
        {
          cnt++;
        }
      list = list->next;
    }

  /* if there is only one directory entry, delete it */
  if (cnt == 1)
    start = elist_delete(start, free_ls_file_t);
  
  return(start);
      
} /* end of remove_ls_dups */
