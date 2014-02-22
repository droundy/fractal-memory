#pragma once

#include <stdio.h>
#include <SDL.h>
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

typedef struct {
  int size, width, height, frame_time;
  Flames f;
  HistogramEntry *hist;
  Uint32 *myPixels;
  SDL_Texture *sdlTexture;
  SDL_Renderer *sdlRenderer;

  SDL_sem *renderme;
  SDL_atomic_t done, dirty;
} SingleHistogramGame;

extern SingleHistogramGame game;

void Draw(SingleHistogramGame *game);
void Init(SingleHistogramGame *game);
void SetFlame(SingleHistogramGame *g, const char *seed, int num);
void HandleKey(SingleHistogramGame *g, SDL_Keycode c);
