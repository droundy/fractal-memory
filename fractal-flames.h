/*  Fractal memory is a memory game that studies human memory
    Copyright (C) 2014 David Roundy

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.  */

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
  case DIAMOND: return "diamond";
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
  default: return "";
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
  char symmetry_description[5000]; // hokey, hokey
} Flames;

// WARNING: the Flames content must be zeroed before calling InitFlames!
void InitFlames(Flames *t, SecureRandom *s);
SDL_sem *ComputeInThread(SDL_atomic_t *dirty, SDL_atomic_t *done,
                         Flames *f, int size, double quality, HistogramEntry *hist);
void ReadHistogram(int size, int x, int y, int width, int height,
                   HistogramEntry *hist, Uint32 *rgb);
void ReadHistogramTransparent(int size, int x, int y, int width, int height,
                              HistogramEntry *hist, Uint32 *argb);
void SaveHistogram(int size, HistogramEntry *hist, const char *fname);


void PrintAffine(AffineTransformation *t);
void PrintTransform(Transformation *t, int i);
void PrintFlames(Flames *f);
void AnnouncePair(Flames *original, Flames *contender, const char *str);
