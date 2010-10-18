typedef struct
{
  double x;
  double y;
} cpVect;

static void entry(cpVect* a)
{
  cpVect* const b = a + 4;
  b->x = 0;
  b->y = 0;
}
