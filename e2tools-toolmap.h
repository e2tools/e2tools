#ifndef E2TOOLS_TOOLMAP_H
#define E2TOOLS_TOOLMAP_H

struct toolmap_element {
  char *name;
  int (*main_func)(int argc, char *argv[]);
};

extern struct toolmap_element toolmap[];

#endif /* !defined(E2TOOLS_TOOLMAP_H) */
