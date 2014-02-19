
#pragma once

#include <SDL2/SDL.h>

extern int dirty;

typedef struct Pt {
	double X, Y, R, G, B;
} Pt;

typedef struct AffineTransformation {
	double Mxx, Mxy, Myx, Myy, Ox, Oy;
} AffineTransformation;

typedef struct Transformation {
	char Type;
	double R, G, B, A;
	AffineTransformation Pre, Post;
} Transformation;

enum {
  // see https://code.google.com/p/flam3/wiki/Variations
  SWIRL,
  HORSESHOE,
  SINUSOIDAL,
  POLAR,
  HANDKERCHIEF,
  HEART,
  DISC,
  SPIRAL,
  HYPERBOLIC,
  DIAMOND,
  EX,
  // skip julia due to randomness
  BENT,
  FISHEYE,
  SPHERICAL,
  IDENTITY,
  MIRROR,
  INVERSION,
  R2,
  R3,
  R4,
  R5,
  R6
};


typedef struct HistogramEntry {
  double R, G, B, A;
} HistogramEntry;

#define MAX_TRANS 25
static const int num_trans = 4;
typedef struct Flames {
	Transformation Transformations[MAX_TRANS];
  int N; // number of entries present in Transformations above
  int version; // a number that will change when we modify the Flames,
               // so a simulation thread can know when to stop.
} Flames;

void InitFlames(Flames *t);
void ComputeInThread(const Flames *f, int size, double quality, HistogramEntry *hist);
void ReadHistogram(int size, int stride, HistogramEntry *hist, Uint32 *rgb);


void PrintFlames(Flames *f);
