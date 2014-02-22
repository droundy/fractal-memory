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
  int x, y;
  int original;
  int on_display;
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
} SingleHistogramGame;

extern SingleHistogramGame game;

void Draw(SingleHistogramGame *game);
void Init(SingleHistogramGame *game);
void SetFlame(SingleHistogramGame *g, const char *seed, int num);
void HandleKey(SingleHistogramGame *g, SDL_Keycode c);
void HandleMouse(SingleHistogramGame *g, int x, int y);

void HandleLeft(SingleHistogramGame *g);
void HandleRight(SingleHistogramGame *g);
