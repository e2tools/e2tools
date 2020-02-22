/* $Header: /home/ksheff/src/e2tools/RCS/progress.c,v 0.1 2002/06/26 11:17:28 ksheff Exp $ */
/*
 * progress.c
 *
 * Copyright (C) 2002 Keith W Sheffield.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 */

/* Description */
/*
 *
 *
 */
/*
 * $Log: progress.c,v $
 * Revision 0.1  2002/06/26 11:17:28  ksheff
 * Initial revision
 *
 */

/* Feature Test Switches */
/* System Headers */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

/* Macros */
#define PROG_FILE_SIZE 50
#define PROG_CLEAR \
"                                                                           \r"
/* Local Variables */

static char prog_file[PROG_FILE_SIZE+1];
static struct stat *prog_sbuf = NULL;
static long prog_start;
static long prog_time;

/* Name:    init_progress()
 *
 * Description:
 *
 * This function initializes the progress status variables for a particular
 * file operation
 * .
 *
 * Algorithm:
 *
 * Determine the size of the file name
 * Copy the last PROG_FILE_SIZE characters of the filename
 * Copy the pointer to the file statistics structure
 *
 * Global Variables:
 *
 * char prog_file[PROG_FILE_SIZE+1];        The file being operated on
 * struct stat *prog_sbuf;                  prog_file's information
 * long prog_start;                          The initial start time
 * long prog_time;                          The last update time
 *
 * Arguments:
 *
 * char *file;                The file to watch
 * struct stat *sbuf;         The file's information
 *
 * Return Values:
 *
 * None
 *
 * Author: Keith W. Sheffield
 * Date:   06/26/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
void init_progress(char *file, struct stat *sbuf)
{
  int len;
  struct timeval tv;

  if (file != NULL && sbuf != NULL)
    {
      if (strcmp(file, "-") == 0)
        file = "<stdin>";

      len = strlen(file);
      if (len > PROG_FILE_SIZE)
        {
          file += (len - PROG_FILE_SIZE);
          len = PROG_FILE_SIZE;
        }
      memset(prog_file, ' ', PROG_FILE_SIZE);
      memcpy(prog_file, file, len);
      prog_file[PROG_FILE_SIZE] = '\0';
      prog_sbuf = sbuf;
      gettimeofday(&tv, NULL);
      prog_start = prog_time = tv.tv_sec;
    }
  else
    {
      prog_file[0] = '\0';
      prog_sbuf = NULL;
      prog_start = prog_time = 0;
    }

} /* end of init_progress */

/* Name:    update_progress()
 *
 * Description:
 *
 * This function updates the current file progress display.
 *
 * Algorithm:
 *
 * Get the system time in seconds.
 * If different from the last update
 *     Save the system time
 *     Display the file name, current byte total, and total file size
 *
 * Global Variables:
 *
 * char prog_file[PROG_FILE_SIZE+1];        The file being operated on
 * struct stat *prog_sbuf;                  prog_file's information
 * long prog_time;                          The last update time
 *
 * Arguments:
 *
 * unsigned long num_bytes;          The number of bytes transferred
 *
 * Return Values:
 *
 * None
 *
 * Author: Keith W. Sheffield
 * Date:   06/26/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
void update_progress(unsigned long num_bytes)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  if (prog_sbuf != NULL && prog_time != tv.tv_sec)
    {
      prog_time = tv.tv_sec;
      fprintf(stderr, "%s %10ld / %10ld\r", prog_file, num_bytes,
              prog_sbuf->st_size);
      fflush(stderr);
    }
} /* end of update_progress */

/* Name:    finish_progress()
 *
 * Description:
 *
 * This function clears the current progress line if needed.
 *
 * Algorithm:
 *
 * Check to see if prog_start is different than prog_time
 *     Clear the progress line
 *
 * Global Variables:
 *
 *
 * Arguments:
 *
 * None
 *
 * Return Values:
 *
 * None
 *
 * Author: Keith W. Sheffield
 * Date:   06/26/2002
 *
 * Modification History:
 *
 * MM/DD/YY      Name               Description
 *
 */
void finish_progress(void)
{
  if (prog_start != prog_time)
    fputs(PROG_CLEAR, stderr);

} /* end of finish_progress */
