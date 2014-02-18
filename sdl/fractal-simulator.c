#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>

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

static void Transform(const Transformation *t, Pt *p) {
  SDL_assert(!isnan(p->X));
  SDL_assert(!isnan(p->Y));
	AffineTransform(&t->Pre, p);
	p->R = t->A*t->R + (1-t->A)*p->R;
	p->G = t->A*t->G + (1-t->A)*p->G;
	p->B = t->A*t->B + (1-t->A)*p->B;
  const double r2 = p->X*p->X + p->Y*p->Y;
  const double r = sqrt(r2);
	const double theta = atan2(p->Y, p->X);
  switch (t->Type) {
	case HORSESHOE :
    {
      double oX = (p->X - p->Y)*(p->X + p->Y)/(1+r);
      double oY = 2*p->X*p->Y/(1+r);
      p->X = oX;
      p->Y = oY;
    }
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SWIRL:
		if (r2 < 1e10) {
			double sinr2 = sin(r2);
      double cosr2 = cos(r2);
			double oX = p->X*sinr2 - p->Y*cosr2;
      double oY = p->X*cosr2 + p->Y*sinr2;
			p->X = oX;
			p->Y = oY;
		}
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SPHERICAL:
		if (r2 != 0) {
      p->X /= 1 + r2;
      p->Y /= 1 + r2;
		}
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case SINUSOIDAL:
		p->X = sin(p->X);
		p->Y = sin(p->Y);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case POLAR:
		p->X = theta/pi;
		p->Y = r - 1;
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	case CIRCLE:
		p->X = r*sin(r+theta);
		p->Y = r*cos(r+theta);
    SDL_assert(!isnan(p->X));
    SDL_assert(!isnan(p->Y));
    break;
	/* case SMEAR: */
  /*   { */
  /*     const double dx = 0.1*sin(p->Y); */
  /*     const double dy = 0.1*sin(p->X); */
  /*     p->X += dx; */
  /*     p->Y += dy; */
  /*   } */
  /*   break; */
	}
	AffineTransform(&t->Post, p);
}

static void TransformFlames(const Flames *t, Pt *p) {
  const int which = rand() % t->N;
  Transform(&t->Transformations[which], p);
}

void Compute(const Flames *f, int size, double quality, HistogramEntry *hist) {
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

	double meanr = 0.0;
	for (int i = 0; i<1000; i++) {
		TransformFlames(f, &p);
		meanr += sqrt(p.X*p.X + p.Y*p.Y);
	}
	meanr /= 1000;
  //meanr = 2.0;
	if (SDL_ASSERT_LEVEL > 2) printf("meanr = %g\n", meanr);

	while (hits < wanthits && f->version == oldversion) {
		TransformFlames(f, &p);
    const int xint = (p.X/meanr/2 + 1.0)*0.5*size + 0.5;
    const int yint = (p.Y/meanr/2 + 1.0)*0.5*size + 0.5;
		const int n = xint + size*yint;
		if (n >= 0 && n < size*size) {
			hist[n].R += p.R;
			hist[n].G += p.G;
			hist[n].B += p.B;
			hist[n].A += 1;
			hits++;
      dirty = 1;
		} else {
			misses++;
		}
		//if misses > 1000 && misses < 2000 && hits/misses < 0.5 {
		//	fmt.Printf("\nGiving up with ratio %g!\n\n", hits/misses)
		//	return nil
		//}
	}
}

static Uint32 RGB(double r, double g, double b) {
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;
  return 0xFF000000 + (int)(r*256)*0x10000 + (int)(g*256)*0x100 + (int)(b*256);
}

void ReadHistogram(int size, int stride, HistogramEntry *hist, Uint32 *rgb) {
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
    for (int iy=0; iy<size; iy++) {
      const int n = ix + size*iy;
      if (hist[n].A > 0) {
        // a(minA)*histA == 0
        // I wish that... a(maxA)*histA == 1 ... but it's not true
        // a(meanA)*histA == 0.5
        const double a = norm*log(factor*hist[n].A)/hist[n].A;
        //printf(ix, iy, a, hist[n].A)
        rgb[ix+stride*iy] = RGB(hist[n].R*a, hist[n].G*a, hist[n].B*a);
      } else {
        rgb[ix+stride*iy] = 0xFF000000;
      }
    }
  }
}

struct Computation {
  const Flames *f;
  int size;
  double quality;
  HistogramEntry *hist;
};

int DoCompute(void *computation) {
  struct Computation *c = (struct Computation *)computation;
  //printf("size: %d\n", c->size);
  Compute(c->f, c->size, c->quality, c->hist);
  free(c);
  return 0;
}

void ComputeInThread(const Flames *f, int size, double quality, HistogramEntry *hist) {
  struct Computation *c = (struct Computation *) malloc(sizeof(struct Computation));
  c->f = f;
  c->size = size;
  c->quality = quality;
  c->hist = hist;
  //printf("size: %d\n", size);
  //DoCompute(c);
  //exit(1);
  SDL_Thread *thread = SDL_CreateThread(DoCompute, "Compute", (void *)c);
  if (!thread) {
    printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
  }
}
