#include "byte.h"
#include "stralloc.h"

int stralloc_cat(sato,safrom)
stralloc *sato;
const stralloc *safrom;
{
  return stralloc_catb(sato,safrom->s,safrom->len);
}
