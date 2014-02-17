#include "fractal-flames.h"
#include <math.h>

const int ran_max = 256;
static const double pi = 3.1415926535897932384626433832795028841971L;

void InitAffine(AffineTransformation *t, int minfraction) {
  int Mxx , Myy, Mxy, Myx, Mlensqr, Oxsqr, Oysqr;
  do {
    Mxx = rand() % (2*ran_max+1) - ran_max;
    Mxy = rand() % (2*ran_max+1) - ran_max;
    Myx = rand() % (2*ran_max+1) - ran_max;
    Myy = rand() % (2*ran_max+1) - ran_max;
    Mlensqr = abs(Mxx*Myy - Mxy*Myx);
  } while (Mlensqr < ran_max*ran_max/minfraction || Mlensqr > ran_max*ran_max*7/10);

  const int osqr_max = 1*(ran_max*ran_max - Mlensqr);
  Oxsqr = (rand() % (2*osqr_max+1)) - osqr_max;
  Oysqr = (rand() % (2*osqr_max+1)) - osqr_max;

  double scale = 1.0/ran_max;
  t->Mxx = Mxx*scale;
  t->Mxy = Mxy*scale;
  t->Myx = Myx*scale;
  t->Myy = Myy*scale;
  if (Oxsqr < 0) t->Ox = -sqrt(-Oxsqr*scale*scale);
  else t->Ox = sqrt(Oxsqr*scale*scale);
  if (Oysqr < 0) t->Oy = -sqrt(-Oysqr*scale*scale);
  else t->Oy = sqrt(Oysqr*scale*scale);
  SDL_assert(!isnan(t->Ox));
  SDL_assert(!isnan(t->Oy));
}

int InitSymmetry(Transformation *t) {
  const unsigned char Syms[] = {
    IDENTITY,
    MIRROR,
    INVERSION,
    R2,
    R3,
    R4,
    R5,
    R6
  };
  const int which = rand() % sizeof(Syms);
  printf("Syms has size %ld\n", sizeof(Syms));
  t->Post.Mxx = 1;
  t->Post.Mxy = 0;
  t->Post.Myx = 0;
  t->Post.Myy = 1;
  {
    double x2, y2;
    do {
      x2 = rand() % (2*ran_max+1) - ran_max;
      y2 = rand() % (2*ran_max+1) - ran_max;
    } while (x2*x2 + y2*y2 > ran_max*ran_max || (x2 == 0 && y2 == 0));
    //x2 = 0; y2 = 0;
    t->Pre.Ox = 0.8*x2/ran_max;
    t->Pre.Oy = 0.8*y2/ran_max;
    t->Post.Ox = -0.8*x2/ran_max;
    t->Post.Oy = -0.8*y2/ran_max;
  }
  SDL_assert(!isnan(t->Pre.Ox));
  SDL_assert(!isnan(t->Pre.Oy));
  SDL_assert(!isnan(t->Post.Ox));
  SDL_assert(!isnan(t->Post.Oy));
  t->R = t->G = t->B = t->A = 0.0; // No color change!
  t->Type = Syms[which];
  fprintf(stderr, "Creating symmetry type %d\n", (int) t->Type);
  switch (t->Type) {
  case INVERSION:
    printf("Creating inversion!\n");
    t->Pre.Mxx = -1;
    t->Pre.Mxy =  0;
    t->Pre.Myx =  0;
    t->Pre.Myy = -1;
    return 2;
  case MIRROR:
    printf("Creating mirror!\n");
    {
      // pick a couple of points in the unit circle:
      double x, y;
      do {
        x = rand() % (2*ran_max+1) - ran_max;
        y = rand() % (2*ran_max+1) - ran_max;
      } while (x*x + y*y > ran_max*ran_max || (x == 0 && y == 0));
      double norm = 1.0/sqrt(x*x + y*y);
      t->Pre.Mxx =  x*norm;
      t->Pre.Mxy =  y*norm;
      t->Pre.Myx =  y*norm;
      t->Pre.Myy = -x*norm;
      return 2;
    }
  case R2:
    printf("Creating R2!\n");
    t->Pre.Mxx =  0;
    t->Pre.Mxy =  1;
    t->Pre.Myx = -1;
    t->Pre.Myy =  0;
    return 2;
  case R3:
    printf("Creating R3!\n");
    t->Pre.Mxx =  cos(2*pi/3);
    t->Pre.Mxy =  sin(2*pi/3);
    t->Pre.Myx = -sin(2*pi/3);
    t->Pre.Myy =  cos(2*pi/3);;
    return 3;
  case R4:
    printf("Creating R4!\n");
    t->Pre.Mxx =  cos(2*pi/4);
    t->Pre.Mxy =  sin(2*pi/4);
    t->Pre.Myx = -sin(2*pi/4);
    t->Pre.Myy =  cos(2*pi/4);;
    return 4;
  case R5:
    printf("Creating R5!\n");
    t->Pre.Mxx =  cos(2*pi/5);
    t->Pre.Mxy =  sin(2*pi/5);
    t->Pre.Myx = -sin(2*pi/5);
    t->Pre.Myy =  cos(2*pi/5);;
    return 5;
  case R6:
    printf("Creating R6!\n");
    t->Pre.Mxx =  cos(2*pi/6);
    t->Pre.Mxy =  sin(2*pi/6);
    t->Pre.Myx = -sin(2*pi/6);
    t->Pre.Myy =  cos(2*pi/6);;
    return 6;
  }
  return 0;
}

void InitFlames(Flames *f) {
  f->N = num_trans;
  for (int i=0; i < f->N; i++) {
    InitAffine(&f->Transformations[i].Pre, 2);
    InitAffine(&f->Transformations[i].Post, 2);
    f->Transformations[i].Type = rand() % IDENTITY;
    f->Transformations[i].A = 0.5;
  }
  f->Transformations[0].R = 1;
  f->Transformations[0].G = 1;
  f->Transformations[0].B = 1;

  f->Transformations[1].R = 1;
  f->Transformations[1].G = 0;
  f->Transformations[1].B = 0;

  f->Transformations[2].R = 0;
  f->Transformations[2].G = 1;
  f->Transformations[2].B = 0;

  f->Transformations[3].R = 0;
  f->Transformations[3].G = 0;
  f->Transformations[3].B = 1;

  fprintf(stderr, "Without symmetry, total number is %d\n", f->N);
  // Now we add in the symmetry operations!
  if (f->N < MAX_TRANS/2) {
    int n = InitSymmetry(&f->Transformations[f->N]);
    if (n*f->N < MAX_TRANS && n > 0) {
      for (int i=f->N+1; i<n*f->N; i++) {
        f->Transformations[i] = f->Transformations[f->N];
        fprintf(stderr, "Old type %d: %d\n", f->N, f->Transformations[f->N].Type);
        fprintf(stderr, "New type %d: %d\n", i, f->Transformations[i].Type);
      }
      fprintf(stderr, "Adding %d to %d\n", n, f->N);
      f->N = n*f->N;
    }
  }
  if (f->N < MAX_TRANS/2) {
    int n = InitSymmetry(&f->Transformations[f->N]);
    if (n*f->N < MAX_TRANS && n > 0) {
      for (int i=f->N+1; i<n*f->N; i++) {
        f->Transformations[i] = f->Transformations[f->N];
        fprintf(stderr, "Old type %d: %d\n", f->N, f->Transformations[f->N].Type);
        fprintf(stderr, "New type %d: %d\n", i, f->Transformations[i].Type);
      }
      fprintf(stderr, "Adding %d to %d\n", n, f->N);
      f->N = n*f->N;
    }
  }
  fprintf("Total number is %d\n", f->N);
  PrintFlames(f);
}
