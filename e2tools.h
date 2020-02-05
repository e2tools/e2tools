#ifndef E2TOOLS_H
#define E2TOOLS_H

/* $Header: /home/ksheff/src/e2tools/RCS/e2tools.h,v 0.7 2004/04/07 01:15:55 ksheff Exp $ */

/* Copyright 2002 Keith W. Sheffield */

/* Description */
/*
 * $Log: e2tools.h,v $
 * Revision 0.7  2004/04/07 01:15:55  ksheff
 * Added the parameter struct stat *def_stat to put_file().
 *
 * Revision 0.6  2002/08/08 07:57:04  ksheff
 * Added new routine do_tail() from tail.c
 * Added new routine read_to_eof() from read.c
 * Made retrieve_data() from read.c a public routine
 *
 * Revision 0.5  2002/04/10 10:43:27  ksheff
 * Added e2rm().
 *
 * Revision 0.4  2002/04/10 09:33:26  ksheff
 * Modified prototypes for functions involved with setting directory
 * attributes.
 *
 * Revision 0.3  2002/03/21 09:05:16  ksheff
 * Added function prototypes from mv.c and altered do_ln() slightly.
 *
 * Revision 0.2  2002/03/07 07:26:02  ksheff
 * Added function prototypes and defined the macros E2T_FORCE and E2T_DO_MV
 *
 * Revision 0.1  2002/02/27 04:47:44  ksheff
 * initial revision
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <utime.h>
/*
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#endif
*/

#ifdef HAVE_OPTRESET
extern int optreset;        /* defined by BSD, but not others */
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"

#define E2T_FORCE 1
#define E2T_DO_MV 2

#include "util.h"
#include "write.h"
#include "progress.h"
#include "read.h"

#include "e2tool-e2cp.h"
#include "e2tool-e2ln.h"
#include "e2tool-e2ls.h"
#include "e2tool-e2mkdir.h"
#include "e2tool-e2mv.h"
#include "e2tool-e2rm.h"
#include "e2tool-e2tail.h"

#if __GNUC__ >= 4
# define UNUSED_PARM(foo) foo __attribute__((unused))
#else
# define UNUSED_PARM(foo) foo ## __UNUSED
#endif

#endif
