#include "fractal-flames.h"
#include <math.h>

const int ran_max = 256;
static const double pi = 3.1415926535897932384626433832795028841971L;

void InitAffine(AffineTransformation *t, int minfraction, SecureRandom *s) {
  int Mxx , Myy, Mxy, Myx, Mlensqr, Oxsqr, Oysqr;
  do {
    Mxx = secure_random32(s) % (2*ran_max+1) - ran_max;
    Mxy = secure_random32(s) % (2*ran_max+1) - ran_max;
    Myx = secure_random32(s) % (2*ran_max+1) - ran_max;
    Myy = secure_random32(s) % (2*ran_max+1) - ran_max;
    Mlensqr = abs(Mxx*Myy - Mxy*Myx);
  } while (Mlensqr < ran_max*ran_max/minfraction || Mlensqr > ran_max*ran_max*7/10);

  const int osqr_max = 1*(ran_max*ran_max - Mlensqr);
  Oxsqr = (secure_random32(s) % (2*osqr_max+1)) - osqr_max;
  Oysqr = (secure_random32(s) % (2*osqr_max+1)) - osqr_max;

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

int InitSymmetry(Transformation *t, SecureRandom *s) {
  const unsigned char Syms[] = {
    IDENTITY,
    MIRROR,
    INVERSION,
    R2,
    R3,
    R4,
    R5,
    R6,
    D3,
    D4,
    D5,
    D6
  };
  const int which = secure_random32(s) % sizeof(Syms);
  t->Post.Mxx = 1;
  t->Post.Mxy = 0;
  t->Post.Myx = 0;
  t->Post.Myy = 1;
  {
    double x2, y2;
    do {
      x2 = secure_random32(s) % (2*ran_max+1) - ran_max;
      y2 = secure_random32(s) % (2*ran_max+1) - ran_max;
    } while (x2*x2 + y2*y2 > ran_max*ran_max || (x2 == 0 && y2 == 0));
    //x2 = 0; y2 = 0;
    const double symm_offset_ratio = 1.8;
    t->Pre.Ox = symm_offset_ratio*x2/ran_max;
    t->Pre.Oy = symm_offset_ratio*y2/ran_max;
    t->Post.Ox = -symm_offset_ratio*x2/ran_max;
    t->Post.Oy = -symm_offset_ratio*y2/ran_max;
  }
  SDL_assert(!isnan(t->Pre.Ox));
  SDL_assert(!isnan(t->Pre.Oy));
  SDL_assert(!isnan(t->Post.Ox));
  SDL_assert(!isnan(t->Post.Oy));
  t->R = t->G = t->B = t->A = 0.0; // No color change!
  t->NTypes = 0;
  t->Type[0] = Syms[which];
  switch (t->Type[0]) {
  case INVERSION:
    t->Pre.Mxx = -1;
    t->Pre.Mxy =  0;
    t->Pre.Myx =  0;
    t->Pre.Myy = -1;
    return 2;
  case MIRROR:
    {
      // pick a couple of points in the unit circle:
      double x, y;
      do {
        x = secure_random32(s) % (2*ran_max+1) - ran_max;
        y = secure_random32(s) % (2*ran_max+1) - ran_max;
      } while (x*x + y*y > ran_max*ran_max || (x == 0 && y == 0));
      double norm = 1.0/sqrt(x*x + y*y);
      t->Pre.Mxx =  x*norm;
      t->Pre.Mxy =  y*norm;
      t->Pre.Myx =  y*norm;
      t->Pre.Myy = -x*norm;
      return 2;
    }
  case R2:
    t->Pre.Mxx =  0;
    t->Pre.Mxy =  1;
    t->Pre.Myx = -1;
    t->Pre.Myy =  0;
    return 2;
  case R3:
    t->Pre.Mxx =  cos(2*pi/3);
    t->Pre.Mxy =  sin(2*pi/3);
    t->Pre.Myx = -sin(2*pi/3);
    t->Pre.Myy =  cos(2*pi/3);;
    return 3;
  case R4:
    t->Pre.Mxx =  cos(2*pi/4);
    t->Pre.Mxy =  sin(2*pi/4);
    t->Pre.Myx = -sin(2*pi/4);
    t->Pre.Myy =  cos(2*pi/4);;
    return 4;
  case R5:
    t->Pre.Mxx =  cos(2*pi/5);
    t->Pre.Mxy =  sin(2*pi/5);
    t->Pre.Myx = -sin(2*pi/5);
    t->Pre.Myy =  cos(2*pi/5);;
    return 5;
  case R6:
    t->Pre.Mxx =  cos(2*pi/6);
    t->Pre.Mxy =  sin(2*pi/6);
    t->Pre.Myx = -sin(2*pi/6);
    t->Pre.Myy =  cos(2*pi/6);;
    return 6;
  case D3:
    t->Pre.Mxx =  cos(2*pi/3);
    t->Pre.Mxy =  sin(2*pi/3);
    t->Pre.Myx = -sin(2*pi/3);
    t->Pre.Myy =  cos(2*pi/3);;
    return 6;
  case D4:
    t->Pre.Mxx =  cos(2*pi/4);
    t->Pre.Mxy =  sin(2*pi/4);
    t->Pre.Myx = -sin(2*pi/4);
    t->Pre.Myy =  cos(2*pi/4);;
    return 8;
  case D5:
    t->Pre.Mxx =  cos(2*pi/5);
    t->Pre.Mxy =  sin(2*pi/5);
    t->Pre.Myx = -sin(2*pi/5);
    t->Pre.Myy =  cos(2*pi/5);;
    return 10;
  case D6:
    t->Pre.Mxx =  cos(2*pi/6);
    t->Pre.Mxy =  sin(2*pi/6);
    t->Pre.Myx = -sin(2*pi/6);
    t->Pre.Myy =  cos(2*pi/6);;
    return 12;
  }
  return 0;
}

static int max3(int a, int b, int c) {
  if (a > b) {
    return (a > c) ? a : c;
  } else {
    return (b > c) ? b : c;
  }
}

void InitFlames(Flames *f, SecureRandom *s) {
  f->version = 0;
  if (f->renderthread) SDL_WaitThread(f->renderthread, NULL);
  bzero(f, sizeof(Flames)); // first zero out whatever was previously there...
  f->version = secure_random32(s);
  init_quick(&f->r); // initialize the random number generator for the simulator
  f->N = num_trans;
  for (int i=0; i < f->N; i++) {
    InitAffine(&f->Transformations[i].Pre, 2, s);
    InitAffine(&f->Transformations[i].Post, 2, s);
    f->Transformations[i].Type[0] = secure_random32(s) % IDENTITY;
    f->Transformations[i].Type[1] = secure_random32(s) % IDENTITY;
    f->Transformations[i].Type[2] = secure_random32(s) % IDENTITY;
    f->Transformations[i].NTypes = 1 + secure_random32(s) % 3;
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

  for (int i=0; i< num_trans; i++) {
    int r = secure_random32(s) % ran_max;
    int g = secure_random32(s) % ran_max;
    int b = secure_random32(s) % ran_max;
    double m = max3(r,g,b);
    f->Transformations[i].R = r / m;
    f->Transformations[i].G = g / m;
    f->Transformations[i].B = b / m;
    f->Transformations[i].A = 0.5;
  }

  // Now we add in the symmetry operations!
  if (f->N < MAX_TRANS/2) {
    int n = InitSymmetry(&f->Transformations[f->N], s);
    if (n*f->N < MAX_TRANS && n > 0) {
      for (int i=f->N+1; i<n*f->N; i++) {
        f->Transformations[i] = f->Transformations[f->N];
      }
      if (f->Transformations[f->N].Type[0] >= D3) {
        //PrintTransform(&f->Transformations[f->N], 0);
        // These symmetries have a mirror plane (with same center of symmetry)
        while (f->Transformations[f->N+n/2].Type[0] != MIRROR) {
          InitSymmetry(&f->Transformations[f->N+n/2], s);
          //fprintf(stderr, "type %s (i.e. %d)\n", show_type(f->Transformations[f->N+n/2].Type[0]),
          //        (int)f->Transformations[f->N+n/2].Type[0]);
          //PrintTransform(&f->Transformations[f->N+n/2]);
        }
        //fprintf(stderr, "******** GOT ONE! *********\n");
        // Set the center of symmetry of the mirror plane to be the same...
        f->Transformations[f->N+n/2].Pre.Ox = f->Transformations[f->N].Pre.Ox;
        f->Transformations[f->N+n/2].Pre.Oy = f->Transformations[f->N].Pre.Oy;
        f->Transformations[f->N+n/2].Post.Ox = f->Transformations[f->N].Post.Ox;
        f->Transformations[f->N+n/2].Post.Oy = f->Transformations[f->N].Post.Oy;
        for (int i=f->N+n/2+1; i<n*f->N; i++) {
          f->Transformations[i] = f->Transformations[f->N+n/2];
        }
      }
      f->N = n*f->N;
    }
  }

  if (f->N < MAX_TRANS/2) {
    int n = InitSymmetry(&f->Transformations[f->N], s);
    if (n*f->N < MAX_TRANS && n > 0) {
      if (secure_random(s) & 1) {
        // with 50% probability, put the second symmetry operation at
        // the same origin as the first.
        f->Transformations[f->N].Pre.Ox = f->Transformations[f->N-1].Pre.Ox;
        f->Transformations[f->N].Pre.Oy = f->Transformations[f->N-1].Pre.Oy;
        f->Transformations[f->N].Post.Ox = f->Transformations[f->N-1].Post.Ox;
        f->Transformations[f->N].Post.Oy = f->Transformations[f->N-1].Post.Oy;
      }
      for (int i=f->N+1; i<n*f->N; i++) {
        f->Transformations[i] = f->Transformations[f->N];
      }
      f->N = n*f->N;
    }
  }
  if (SDL_ASSERT_LEVEL > 2) PrintFlames(f);
}
