#ifndef ELIST_H
#define ELIST_H

/* $Header: /home/ksheff/src/e2tools/RCS/elist.h,v 0.3 2002/06/05 22:04:13 ksheff Exp $ */

/* Copyright 2002 Keith W. Sheffield */

/*
 * $Log: elist.h,v $
 * Revision 0.3  2002/06/05 22:04:13  ksheff
 * Added elist_delete.
 *
 * Revision 0.2  2002/06/03 21:02:39  ksheff
 * Added elist_sort() definition.
 *
 * Revision 0.1  2002/03/07 07:24:50  ksheff
 * initial revision
 *
 */


typedef struct _elist_t
{
    struct _elist_t *prev;
    struct _elist_t *next;
    void *data;
} elist_t;

extern
elist_t * elist_new();

extern
elist_t *elist_delete(elist_t *l, void (*data_free)(void *));

extern
void elist_free(elist_t *l, void (*data_free)(void *));

extern
elist_t *elist_append(elist_t *l, void *data);

extern
elist_t *elist_insert(elist_t *l, void *data);

extern
void elist_sort(elist_t *l, int (sort_func)(const void *, const void *), int reverse);

#endif /* !defined(ELIST_H) */
