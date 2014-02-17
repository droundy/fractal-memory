
#pragma once

#include <SDL2/SDL.h>

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
  SWIRL,
  HORSESHOE,
  SINUSOIDAL,
  POLAR,
  CIRCLE,
  //SMEAR,
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

#define MAX_TRANS 60
static const int num_trans = 4;
typedef struct Flames {
	Transformation Transformations[MAX_TRANS];
  int N;
} Flames;

void InitFlames(Flames *t);
void ComputeInThread(const Flames *f, int size, double quality, HistogramEntry *hist);
void ReadHistogram(int size, int stride, HistogramEntry *hist, Uint32 *rgb);


void PrintFlames(Flames *f);
