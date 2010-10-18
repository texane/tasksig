typedef struct
{
  double x;
  double y;
} cpVect;

static void entry(cpVect* a, unsigned int count)
{
  for (unsigned int i = 0; i < count; ++i, ++a)
  {
    a->x = 0;
    a->y = 0;
  }
}
