/* $Header: /home/ksheff/src/e2tools/RCS/e2tools.c,v 0.7 2002/08/08 07:55:03 ksheff Exp $ */
/*
 * e2tools.c
 */

/* Description */
/*
 *
 *
 */
/*
 * $Log: e2tools.c,v $
 * Revision 0.7  2002/08/08 07:55:03  ksheff
 * Added e2tail
 *
 * Revision 0.6  2002/04/10 10:43:09  ksheff
 * Added e2rm
 *
 * Revision 0.5  2002/03/21 04:38:14  ksheff
 * Created a separate do_mv function instead of overloading do_ln.
 *
 * Revision 0.4  2002/03/07 07:17:24  ksheff
 * Added ability to move files on an ext2fs.
 *
 * Revision 0.3  2002/03/05 14:02:30  ksheff
 * Added a call to do_ln() if the user wants to invoke the ln program.
 *
 * Revision 0.2  2002/02/27 05:26:22  ksheff
 * Added a call to e2mkdir if the user wants to invoke the mkdir program.
 *
 * Revision 0.1  2002/02/27 04:47:21  ksheff
 * initial revision
 *
 */

/* Feature Test Switches */
/* Headers */

#include "e2tools.h"

void
usage()
{
    fprintf(stderr, "Usage: e2tools <command> [OPTION...]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, " Commands:\n");
    fprintf(stderr, "  e2cp\n");
    fprintf(stderr, "  e2mkdir\n");
    fprintf(stderr, "  e2ln\n");
    fprintf(stderr, "  e2mv\n");
    fprintf(stderr, "  e2rm\n");
    fprintf(stderr, "  e2tail\n");
}

int
main(int argc, char *argv[])
{
  char *ptr;

  if (NULL != (ptr = strrchr(argv[0], '/')))
    ptr++;
  else
    ptr = argv[0];

  if (strcmp(ptr, "e2tools") == 0)
    {
      if (argc < 2)
        {
            usage();
            exit(1);
        }
      ++argv;
      --argc;
      ptr = argv[0];
    }

  initialize_ext2_error_table();

  if (strcmp(ptr, "e2ls") == 0)
    return main_e2ls(argc, argv);
  else if (strcmp(ptr, "e2cp") == 0)
    return main_e2cp(argc, argv);
  else if (strcmp(ptr, "e2mkdir") == 0)
    return main_e2mkdir(argc, argv);
  else if (strcmp(ptr, "e2ln") == 0)
    return main_e2ln(argc, argv);
  else if (strcmp(ptr, "e2mv") == 0)
    return main_e2mv(argc, argv);
  else if (strcmp(ptr, "e2rm") == 0)
    return main_e2rm(argc, argv);
  else if (strcmp(ptr, "e2tail") == 0)
    return main_e2tail(argc, argv);

  fprintf(stderr, "e2tools command not implemented\n");
  return 1;
}
