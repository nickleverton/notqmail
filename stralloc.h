#ifndef STRALLOC_H
#define STRALLOC_H

#include "gen_alloc.h"

GEN_ALLOC_typedef(stralloc,char,s,len,a)

extern int stralloc_ready(stralloc *sa, unsigned int n);
extern int stralloc_readyplus(stralloc *sa, unsigned int pluslen);
extern int stralloc_copy(stralloc *sato, const stralloc *safrom);
extern int stralloc_cat(stralloc *sato, const stralloc *safrom);
extern int stralloc_copys(stralloc *sa, const char *s);
extern int stralloc_cats(stralloc *sa, const char *s);
extern int stralloc_copyb(stralloc *sa, const char *s, unsigned int n);
extern int stralloc_catb(stralloc *sa, const char *s, unsigned int n);
extern int stralloc_append(stralloc *sa, char *c); /* beware: this takes a pointer to 1 char */
extern int stralloc_starts(stralloc *sa, const char *s);

#define stralloc_0(sa) stralloc_append(sa,"")

#endif
