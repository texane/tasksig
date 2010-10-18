typedef struct
{
  double x;
  double y;
} cpVect;

static void add_vect(cpVect* a, const cpVect* b)
{
  a->x += b->x;
  a->y += b->y;
}

static void entry(cpVect* a, const cpVect* b, unsigned int count, unsigned int* res)
{
  *res = 0;

  for (unsigned int i = 0; i < count; ++i)
  {
    cpVect* const aptr = a + i;
    const cpVect* const bptr = b + i;
    add_vect(aptr, bptr);

    if (aptr->x == 0.f) ++*res;
  }
}

/* should produce:
   entry()
   a: in, out
   b: in
   count: in
   res: out

   notes:
   if a param is const, no need to track
   should be able to track which field is dirty
   from this description produce the dirtymap
 */
