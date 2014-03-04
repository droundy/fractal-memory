/*  Fractal memory is a memory game that studies human memory
    Copyright (C) 2014 David Roundy

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA  */

#pragma once

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "fractal-flames.h"

static const int background_gray = 30;

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
  COPYCOLOR0 = 1,
  COPYCOLOR1 = 1<<1,
  COPYCOLOR2 = 1<<2,
  COPYCOLOR3 = 1<<3,
  COPYSHAPE0 = 1<<4,
  COPYSHAPE1 = 1<<5,
  COPYSHAPE2 = 1<<6,
  COPYSHAPE3 = 1<<7,
  COPYSYMMETRY = 1<<8,
  COPYALLBUTSYMMETRYLOCATION = 1<<9,
  COPYALLBUTSYMMETRYKIND = 1<<10,
  COPYALLBUTSYMMETRY = 1<<11,
  NOTWEAK = 0,
  COPYGRAY = 1<<12,
} Tweak;

static const Tweak COPYCOLOR = COPYCOLOR0 | COPYCOLOR1 | COPYCOLOR2 | COPYCOLOR3;
static const Tweak COPYSHAPE = COPYSHAPE0 | COPYSHAPE1 | COPYSHAPE2 | COPYSHAPE3;
static const Tweak COPYORIGINAL = COPYSYMMETRY | COPYALLBUTSYMMETRY;

typedef struct {
  const char *str;
  int original, seed;
  Tweak tweak, alltweak;
} TweakedSeed;

typedef struct {
  int size, width, height, frame_time;
  int fontsize, buttonwidth, buttonheight;
  int backR, backG, backB;

  int false_positives, true_positives, false_negatives, true_negatives;
  int score;

  int x, y;
  TweakedSeed original, on_display;
  Flames f;
  HistogramEntry *hist;
  Uint32 *myPixels;
  SDL_Texture *screen_texture;
  SDL_Texture *fractal_texture;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;

  SDL_sem *renderme;
  SDL_atomic_t done, dirty, bufferdirty, display_on;

  // The buffer holds the fractal image and is filled by the buffer_filler
  Uint32 *buffer;
  SDL_Thread *buffer_filler;
  TTF_Font *font;
} SingleHistogramGame;

extern SingleHistogramGame game;

void Draw(SingleHistogramGame *game);
void UpdateFractalTexture(SingleHistogramGame *g);
void Init(SingleHistogramGame *game);
void SetFlame(SingleHistogramGame *g, TweakedSeed seed);
void HandleKey(SingleHistogramGame *g, SDL_Keycode c);
void HandleMouse(SingleHistogramGame *g, int x, int y);

void HandleLeft(SingleHistogramGame *g);
void HandleRight(SingleHistogramGame *g);
void HandleUp(SingleHistogramGame *g);
void HandleDown(SingleHistogramGame *g);

void renderTextAt(SingleHistogramGame *g, const char *message, int x, int y);


int is_original(TweakedSeed seed);
void ShowTweaked(char *buffer, TweakedSeed seed);
void NextGuess(SingleHistogramGame *g);
void SetOriginal(SingleHistogramGame *g);
Flames CreateFlame(TweakedSeed seed);

void SaveToFile(SingleHistogramGame *g, const char *fname);

void PauseGame(SingleHistogramGame *g);
void ResumeGame(SingleHistogramGame *g);

void RedirectToNetwork(const char *userid);
