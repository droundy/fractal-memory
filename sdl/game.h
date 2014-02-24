#pragma once

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "fractal-flames.h"

static inline  void exitMessage(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                           "Fatal error!",
                           msg,
                           NULL);
  SDL_Quit();
  exit(1);
}

typedef enum {
  COPYCOLOR = 1,
  COPYSHAPE0 = 1<<1,
  COPYSHAPE1 = 1<<2,
  COPYSHAPE2 = 1<<3,
  COPYSHAPE3 = 1<<4,
  COPYSYMMETRY = 1<<5,
  COPYALLBUTSYMMETRY = 1<<6,
  NOTWEAK = 0,
  COPYGRAY = 1<<7,
} Tweak;

static const Tweak COPYSHAPE = COPYSHAPE0 | COPYSHAPE1 | COPYSHAPE2 | COPYSHAPE3;
static const Tweak COPYORIGINAL = COPYSYMMETRY | COPYALLBUTSYMMETRY;

typedef struct {
  const char *str;
  int original, seed;
  Tweak tweak, alltweak;
} TweakedSeed;

typedef struct {
  int size, width, height, frame_time;

  int false_positives, true_positives, false_negatives, true_negatives;
  int games_won;

  int x, y;
  TweakedSeed original, on_display;
  Flames f;
  HistogramEntry *hist;
  Uint32 *myPixels;
  SDL_Texture *sdlTexture;
  SDL_Renderer *sdlRenderer;

  SDL_sem *renderme;
  SDL_atomic_t done, dirty;

  // The buffer holds the fractal image and is filled by the buffer_filler
  Uint32 *buffer;
  SDL_Thread *buffer_filler;
  TTF_Font *font;
} SingleHistogramGame;

extern SingleHistogramGame game;

void Draw(SingleHistogramGame *game);
void Init(SingleHistogramGame *game);
void SetFlame(SingleHistogramGame *g, TweakedSeed seed);
void HandleKey(SingleHistogramGame *g, SDL_Keycode c);
void HandleMouse(SingleHistogramGame *g, int x, int y);

void HandleLeft(SingleHistogramGame *g);
void HandleRight(SingleHistogramGame *g);
void HandleUp(SingleHistogramGame *g);
void HandleDown(SingleHistogramGame *g);

void renderTextAt(SingleHistogramGame *g, const char *message, int x, int y);
