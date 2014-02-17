#include "fractal-flames.h"

void PrintAffine(AffineTransformation *t) {
  fprintf(stderr, "M:  [ %10g %10g ]\n", t->Mxx, t->Myx);
  fprintf(stderr, "    [ %10g %10g ]\n", t->Mxy, t->Myy);
  fprintf(stderr, "O:  [ %10g %10g ]\n", t->Ox, t->Oy);
}

void PrintTransform(Transformation *t, int i) {
  fprintf(stderr, "[%2d] Transform (type %d):\n", i, (int)t->Type);
  PrintAffine(&t->Pre);
  fprintf(stderr, "RGBA: ( %10g %10g %10g %10g )\n", t->R, t->G, t->B, t->A);
  PrintAffine(&t->Post);
}

void PrintFlames(Flames *f) {
  fprintf(stderr, "Flames: (size %d)\n", f->N);
  for (int i=0; i<f->N; i++) {
    PrintTransform(&f->Transformations[i], i);
  }
}
