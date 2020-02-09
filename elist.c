/* $Header: /home/ksheff/src/e2tools/RCS/elist.c,v 0.6 2004/04/06 19:34:44 ksheff Exp $ */
/*
 * elist.c
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
 * $Log: elist.c,v $
 * Revision 0.6  2004/04/06 19:34:44  ksheff
 * Corrected elist_delete to update the previous node if it exists.
 *
 * Revision 0.5  2002/06/05 22:05:01  ksheff
 * Added elist_delete.
 *
 * Revision 0.4  2002/06/03 23:02:01  ksheff
 * Added the function elist_sort().
 *
 * Revision 0.3  2002/04/23 01:49:48  ksheff
 * Added a define for NULL if it doesn't exist.
 *
 * Revision 0.2  2002/03/07 07:33:28  ksheff
 * silenced compiler warnings for calloc return value.
 *
 * Revision 0.1  2002/03/07 07:24:35  ksheff
 * initial revision
 *
 */

/* Feature Test Switches */
/* Headers */
#include <memory.h>
#include <stdlib.h>
#include "elist.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

elist_t *
elist_new(void)
{
  elist_t *list;

  list = (elist_t *) calloc(sizeof(elist_t), 1);
  return list;
}

elist_t *
elist_delete(elist_t *l, void (*data_free)(void *))
{
  elist_t *n;

  if (l)
    {
      n = l->next;
      if (n)
        n->prev = l->prev;
      if (l->prev)
        l->prev->next = n;
      if (data_free && l->data)
        (*data_free)(l->data);
      free(l);
      l = n;
    }
  return(l);
}

void
elist_free(elist_t *l, void (*data_free)(void *))
{
  if (l)
    {
      do
        {
          l = elist_delete(l, data_free);
        } while (l);
    }
}

elist_t *
elist_append(elist_t *l, void *data)
{
  elist_t *n;
  elist_t *t;

  if (NULL == (n = elist_new()))
    return(NULL);

  n->data = data;

  if (l)
    {
      t = l;
      while (t->next != NULL) t = t->next;
      t->next = n;
      n->prev = t;
    }
  else
    l = n;

  return(l);
}

elist_t *
elist_insert(elist_t *l, void *data)
{
  elist_t *n;

  if (NULL == (n = elist_new()))
    return(NULL);
  n->data = data;

  if (l)
    {
      n->prev = l->prev;
      l->prev = n;
      n->next = l;
      if (n->prev)
        n->prev->next = n;
    }
  l = n;

  return(l);
}

void elist_sort(elist_t *l, int (*sort_func)(const void *, const void *), int reverse)
{
  int c=0;
  elist_t *tl;
  void **data;
  void **dptr;

  if (l != NULL && sort_func != NULL)
    {
      /* count the number of nodes */
      tl = l;
      while (tl != NULL)
        {
          c++;
          tl = tl->next;
        }

      /* if there are more than 1 nodes, allocate a lookup table */
      if (c > 1 && (NULL != (data = (void **) malloc(c*sizeof(void*)))))
        {
          /* fill in the lookup table */
          tl = l;
          dptr = data;
          while (tl != NULL)
            {
              *dptr++ = tl->data;
              tl = tl->next;
            }

          qsort(data, c, sizeof(dptr), sort_func);

          /* now fill in the data pointers in the linked list with the
           * data nodes in the correct order
           */
          tl = l;
          if (reverse)
            {
              dptr = data + c - 1;
              while (tl != NULL)
                {
                  tl->data = *dptr--;
                  tl = tl->next;
                }
            }
          else
            {
              dptr = data;
              while (tl != NULL)
                {
                  tl->data = *dptr++;
                  tl = tl->next;
                }
            }
          free(data);
        }
    }
}



