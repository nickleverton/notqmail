#include "byte.h"
#include "stralloc.h"

int stralloc_copy(sato,safrom)
stralloc *sato;
const stralloc *safrom;
{
  return stralloc_copyb(sato,safrom->s,safrom->len);
}
