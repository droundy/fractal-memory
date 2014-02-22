
#pragma once

#include <SDL.h>
#include "secure-random.h"
#include "quick-random.h"

extern int dirty;

typedef struct Pt {
	double X, Y, R, G, B;
} Pt;

typedef struct AffineTransformation {
	double Mxx, Mxy, Myx, Myy, Ox, Oy;
} AffineTransformation;

typedef struct Transformation {
	char Type[3], NTypes;
	double R, G, B, A;
	AffineTransformation Pre, Post;
} Transformation;

enum {
  // see https://code.google.com/p/flam3/wiki/Variations
  SWIRL = 0,
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
  R6,
  D3,
  D4,
  D5,
  D6
};

static inline const char *show_type(char type) {
  switch (type) {
  case SWIRL: return "swirl";
  case HORSESHOE: return "horseshoe";
  case SINUSOIDAL: return "sinusoidal";
  case POLAR: return "polar";
  case HANDKERCHIEF: return "handkerchief";
  case HEART: return "heart";
  case DISC: return "disc";
  case SPIRAL: return "spiral";
  case HYPERBOLIC: return "hyperbolic";
  case DIAMOND: return "diamone";
  case EX: return "ex";
  // skip julia due to randomness
  case BENT: return "bent";
  case FISHEYE: return "fisheye";
  case SPHERICAL: return "spherical";
  case IDENTITY: return "identity";
  case MIRROR: return "mirror";
  case INVERSION: return "inversion";
  case R2: return "R2";
  case R3: return "R3";
  case R4: return "R4";
  case R5: return "R5";
  case R6: return "R6";
  case D3: return "D3";
  case D4: return "D4";
  case D5: return "D5";
  case D6: return "D6";
  default: return "ERROR!";
  }
}


typedef struct HistogramEntry {
  double R, G, B, A;
} HistogramEntry;

#define MAX_TRANS 33
static const int num_trans = 4;
typedef struct Flames {
	Transformation Transformations[MAX_TRANS];
  int N; // number of entries present in Transformations above
  int version; // a number that will change when we modify the Flames,
               // so a simulation thread can know when to stop.
  QuickRandom r;
} Flames;

// WARNING: the Flames content must be zeroed before calling InitFlames!
void InitFlames(Flames *t, SecureRandom *s);
SDL_sem *ComputeInThread(SDL_atomic_t *dirty, SDL_atomic_t *done,
                         Flames *f, int size, double quality, HistogramEntry *hist);
void ReadHistogram(int size, int x, int y, int width, int height,
                   HistogramEntry *hist, Uint32 *rgb);


void PrintAffine(AffineTransformation *t);
void PrintTransform(Transformation *t, int i);
void PrintFlames(Flames *f);
