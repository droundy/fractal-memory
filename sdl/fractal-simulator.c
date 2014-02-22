#include <math.h>
#include <stdio.h>
#include <SDL.h>

#include "fractal-flames.h"

static const double pi = 3.1415926535897932384626433832795028841971L;

static void AffineTransform(const AffineTransformation *i, Pt *p) {
  if (isnan(i->Ox*i->Oy)) {
    printf("%g %g\n", i->Ox, i->Oy);
  }
  SDL_assert(!isnan(i->Mxx));
  SDL_assert(!isnan(i->Mxy));
  SDL_assert(!isnan(i->Myx));
  SDL_assert(!isnan(i->Myy));
  SDL_assert(!isnan(i->Ox));
  SDL_assert(!isnan(i->Oy));
	double pX = p->X + i->Ox;
	double pY = p->Y + i->Oy;
	p->X = pX*i->Mxx + pY*i->Mxy;
	p->Y = pX*i->Myx + pY*i->Myy;
  SDL_assert(!isnan(p->X));
  SDL_assert(!isnan(p->Y));
}

static const double EPS = 1e-15;

static void TransformType(char type, Pt *p, double px, double py, double r2, double r, double theta) {
  switch (type) {
	case HORSESHOE :
    p->X += (px - py)*(px + py)/(EPS+r);
    p->Y += 2*px*py/(EPS+r);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SWIRL:
    if (r2 < 1e10) { // FIXME remove this
      double sinr2 = sin(r2);
      double cosr2 = cos(r2);
      p->X += px*sinr2 - py*cosr2;
      p->Y += px*cosr2 + py*sinr2;
    } else {
      p->X += px;
      p->Y += py;
    }
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SPHERICAL:
    if (r2 != 0) { // FIXME remove this
      p->X += px/(1e-6 + r2);
      p->Y += py/(1e-6 + r2);
    }
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SINUSOIDAL:
		p->X += sin(px);
		p->Y += sin(py);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case POLAR:
		p->X += theta/pi;
		p->Y += r - 1;
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case HANDKERCHIEF:
		p->X += r*sin(theta+r);
		p->Y += r*cos(theta-r);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case HEART:
		p->X +=  r*sin(theta*r);
		p->Y += -r*cos(theta*r);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case DISC:
		p->X += theta/pi*sin(pi*r);
		p->Y += theta/pi*cos(pi*r);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SPIRAL:
    p->X += (cos(theta) + sin(r2+1e-6))/(r2+1e-6);
    p->Y += (sin(theta) - cos(r2+1e-6))/(r2+1e-6);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
  case HYPERBOLIC:
    p->X += sin(theta) / (r + 1e-6);
    p->Y += cos(theta) * (r + 1e-6);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
  case DIAMOND:
    p->X += sin(theta) * cos(r);
    p->Y += cos(theta) * sin(r);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
  case EX:
    {
      const double n0 = sin(theta+r);
      const double n1 = cos(theta-r);
      const double m0 = r * n0*n0*n0;
      const double m1 = r * n1*n1*n1;
      p->X += m0 + m1;
      p->Y += m0 - m1;
    }
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
  case BENT:
    if (px < 0) p->X += px*2;
    else p->X += px;
    if (py < 0) p->Y += py/2;
    else p->Y += py;
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
  case FISHEYE:
    p->X += 2/(r+1)*py;
    p->Y += 2/(r+1)*px;
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
  default:
    if (type >= IDENTITY) {
      p->X += px;
      p->Y += py;
    } else {
      printf("OOps type %d\n", type);
      SDL_assert(0);
    }
	}
}

static void Transform(const Transformation *t, Pt *p) {
  SDL_assert(!isnan(p->X));
  SDL_assert(!isnan(p->Y));
	AffineTransform(&t->Pre, p);
	p->R = t->A*t->R + (1-t->A)*p->R;
	p->G = t->A*t->G + (1-t->A)*p->G;
	p->B = t->A*t->B + (1-t->A)*p->B;

  if (t->NTypes > 0) { // otherwise it's a symmetry operation and doesn't need this
    const double r2 = p->X*p->X + p->Y*p->Y;
    const double r = sqrt(r2);
    const double theta = atan2(p->Y, p->X);
    const double px = p->X, py = p->Y;

    p->X = 0; p->Y = 0; // TransformType below will accumulate the "p" position.
    for (int i=0;i<t->NTypes;i++) TransformType(t->Type[i], p, px, py, r2, r, theta);
    p->X /= t->NTypes;
    p->Y /= t->NTypes;
  }
  AffineTransform(&t->Post, p);
}

static void TransformFlames(Flames *t, Pt *p) {
  const int which = quickrand32(&t->r) % t->N;
  Transform(&t->Transformations[which], p);
}

void Compute(SDL_atomic_t *done, SDL_atomic_t *dirty,
             Flames *f, int size, double quality, HistogramEntry *hist) {
  int oldversion = f->version;
	double hits = 0.0;
  double misses = 0.0;
  //printf("quality: %g\n", quality);
  //printf("size: %d\n", size);
	double wanthits = pow(10, quality)*size*size;

	Pt p = { 0.123, 0.137, 0, 0, 0 };
	for (int i=0;i<10000;i++) {
    TransformFlames(f, &p);
	}

  // Let's compute the mean radius of points
  const int mean_num = 10000;
	double meanr = 0.0;
	for (int i = 0; i<mean_num; i++) {
		TransformFlames(f, &p);
		meanr += sqrt(p.X*p.X + p.Y*p.Y);
	}
	meanr /= mean_num;

  // Now we'll compute the standard deviation of the radius
	double stddev = 0.0;
	for (int i = 0; i<mean_num; i++) {
		TransformFlames(f, &p);
    const double r = sqrt(p.X*p.X + p.Y*p.Y);
    stddev += (r-meanr)*(r-meanr);
	}
	stddev = sqrt(stddev/mean_num);

  // Now we'll estimate the median radius
	const double dr = 100*stddev/mean_num;
  double medianr = meanr;
	for (int i = 0; i<mean_num; i++) {
		TransformFlames(f, &p);
    const double r = sqrt(p.X*p.X + p.Y*p.Y);
    medianr += (r > medianr) ? dr : -dr;
	}

  //if (meanr > 20) meanr = 20;
	if (SDL_ASSERT_LEVEL > 2) printf("meanr = %g\n", meanr);
	if (SDL_ASSERT_LEVEL > 2) printf("stddev = %g\n", stddev);
	if (SDL_ASSERT_LEVEL > 2) printf("medianr = %g\n", medianr);

	while (hits < wanthits && !SDL_AtomicGet(done) && f->version == oldversion) {
		TransformFlames(f, &p);
    const int xint = (p.X/medianr/2 + 1.0)*0.5*size + 0.5;
    const int yint = (p.Y/medianr/2 + 1.0)*0.5*size + 0.5;
		const int n = xint + size*yint;
		if (n >= 0 && n < size*size) {
			hist[n].R += p.R;
			hist[n].G += p.G;
			hist[n].B += p.B;
			hist[n].A += 1;
			hits++;
      if (!(((int)hits) % (size*size))) SDL_AtomicSet(dirty, 1);
		} else {
			misses++;
		}
	}
  //printf("Finished Compute!\n");
}

static Uint32 RGB(double r, double g, double b) {
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;
  return 0xFF000000 + (int)(r*256)*0x10000 + (int)(g*256)*0x100 + (int)(b*256);
}

void ReadHistogram(int size, int x, int y, int width, int height,
                   HistogramEntry *hist, Uint32 *rgb) {
	double filling = 0.0;
  const int N = size*size;
	for (int i=0;i<N;i++) {
		if (hist[i].A != 0) {
			filling++;
		}
	}
	filling /= N;
	//printf("Filled %2.0f%%\n", filling*100);

  double maxA = 0.0;
  double minA = 10000.0;
  double mean_nonzero_A = 0.0;
  double mean_sqr_nonzero_A = 0.0;
  double meanA = 0.0;
  double meansqrA = 0.0;
	int hits = 0;
	for (int i = 0; i < N; i++) {
		if (hist[i].A > maxA) {
			maxA = hist[i].A;
		}
		if (hist[i].A > 0 && hist[i].A < minA) {
      minA = hist[i].A;
		}
		if (hist[i].A > 0) {
      mean_nonzero_A += hist[i].A;
      mean_sqr_nonzero_A += hist[i].A*hist[i].A;
      hits++;
		}
    meanA += hist[i].A;
    meansqrA += hist[i].A*hist[i].A;
	}
	mean_nonzero_A /= hits;
	mean_sqr_nonzero_A /= hits;
	meanA /= N;
	meansqrA /= N;
	//double stddev = sqrt(meansqrA - meanA*meanA);
	//printf("minA %g\n", minA);
	//printf("maxA %g\n", maxA);
	//printf("meanA %g\n", meanA);
	//printf("stddev %g\n", stddev);
	//printf("mean_nonzero_A %g\n", mean_nonzero_A);
	//printf("root mean_sqr_nonzero_A %g\n", sqrt(mean_sqr_nonzero_A));
  const double factor = maxA/(mean_nonzero_A*mean_nonzero_A);
  const double norm = 1.0/log(factor*maxA);
	//denominator = 2*stddev*stddev/maxA
	for (int ix=0; ix<size; ix++) {
    if (ix + x < width && ix + x >= 0) {
      for (int iy=0; iy<size; iy++) {
        if (iy + y < height && iy + y >= 0) {
          const int n = ix + size*iy;
          if (hist[n].A > 0) {
            // a(minA)*histA == 0
            // I wish that... a(maxA)*histA == 1 ... but it's not true
            // a(meanA)*histA == 0.5
            const double a = norm*log(factor*hist[n].A)/hist[n].A;
            //printf(ix, iy, a, hist[n].A)
            rgb[(x+ix)+width*(y+iy)] = RGB(hist[n].R*a, hist[n].G*a, hist[n].B*a);
          } else {
            rgb[(x+ix)+width*(y+iy)] = 0xFF000000;
          }
        }
      }
    }
  }
}

struct Computation {
  Flames *f;
  int size;
  double quality;
  HistogramEntry *hist;
  SDL_sem *sem;
  SDL_atomic_t *done, *dirty;
};

int DoCompute(void *computation) {
  struct Computation *c = (struct Computation *)computation;
  //printf("size: %d\n", c->size);
  while (!SDL_AtomicGet(c->done)) {
    Compute(c->done, c->dirty, c->f, c->size, c->quality, c->hist);
    SDL_SemWait(c->sem);
  }
  free(c);
  return 0;
}

SDL_sem *ComputeInThread(SDL_atomic_t *done, SDL_atomic_t *dirty,
                         Flames *f, int size, double quality, HistogramEntry *hist) {
  struct Computation *c = (struct Computation *) malloc(sizeof(struct Computation));
  c->f = f;
  c->size = size;
  c->quality = quality;
  c->hist = hist;
  //printf("size: %d\n", size);
  //DoCompute(c);
  //exit(1);
  c->sem = SDL_CreateSemaphore(0);
  c->done = done;
  c->dirty = dirty;
  SDL_Thread *renderthread = SDL_CreateThread(DoCompute, "Compute", (void *)c);
  if (!renderthread) {
    printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
  }
  return c->sem;
}
