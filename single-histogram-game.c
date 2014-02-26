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

#include "game.h"
#include "words.h"

static inline int min(int a, int b) {
  return (a<b) ? a : b;
}

void renderTextAt(SingleHistogramGame *g, const char *message, int x, int y) {
  //We need to first render to a surface as that's what TTF_RenderText
  //returns, then load that surface into a texture
  SDL_Color color = { 255, 255, 255 };
  SDL_Surface *surf = TTF_RenderText_Blended(g->font, message, color);
  if (surf == NULL) exitMessage("Trouble creating surface");
  SDL_Texture *texture = SDL_CreateTextureFromSurface(g->sdlRenderer, surf);
  if (texture == NULL) exitMessage("Trouble making texture");
  //Clean up the surface and font
  SDL_FreeSurface(surf);

  //Setup the destination rectangle to be at the position we want
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  //Query the texture to get its width and height to use
  SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
  SDL_RenderCopy(g->sdlRenderer, texture, NULL, &dst);

  SDL_DestroyTexture(texture);
}

int FillBuffer(SingleHistogramGame *g) {
  while (!SDL_AtomicGet(&g->done)) {
    ReadHistogram(g->size, 0, 0, g->size, g->size, g->hist, g->buffer);
    SDL_Delay(g->frame_time);
  }
  return 0;
}

void SetOriginal(SingleHistogramGame *g) {
  g->original.seed = SDL_GetTicks();
  g->original.str = words[g->original.seed % num_words];
  g->original.original = g->original.seed;
  g->original.tweak = NOTWEAK;
  g->original.alltweak = NOTWEAK;
  g->on_display = g->original;
  switch (SDL_GetTicks() % 18) {
  case 0:
    printf("Using shape of original.\n");
    g->on_display.alltweak = COPYSHAPE;
    break;
  case 1:
    printf("Using color of original.\n");
    g->on_display.alltweak = COPYCOLOR;
    break;
  case 2:
    printf("No tweaks.\n");
    break;
  case 3:
  case 4:
    printf("Maintaining symmetry.\n");
    g->on_display.alltweak = COPYSYMMETRY;
    break;
  case 5:
  case 6:
    printf("Maintaining symmetry and grayness.\n");
    g->on_display.alltweak = COPYSYMMETRY | COPYGRAY;
    g->original.alltweak = COPYGRAY;
    break;
  case 7:
  case 8:
    printf("Maintaining all but symmetry.\n");
    g->on_display.alltweak = COPYALLBUTSYMMETRY;
    break;
  case 9:
  case 10:
    printf("Maintaining all but symmetry and grayness.\n");
    g->on_display.alltweak = COPYALLBUTSYMMETRY | COPYGRAY;
    g->original.alltweak = COPYGRAY;
    break;
  case 11:
  case 12:
  case 13:
    printf("Maintaining symmetry and grayness.\n");
    g->on_display.alltweak = COPYSYMMETRY | COPYCOLOR;
    break;
  default:
    printf("Using black and white.\n");
    g->on_display.alltweak = COPYGRAY;
    g->original.alltweak = COPYGRAY;
    break;
  }
  SetFlame(g, g->on_display);
}

void Init(SingleHistogramGame *g) {
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

  if(TTF_Init()==-1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }

  SDL_Window *sdlWindow;
  SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
                              &sdlWindow, &g->sdlRenderer);
  if (!g->sdlRenderer || !sdlWindow) {
    exitMessage("Unable to create window!");
  }

  SDL_GetWindowSize(sdlWindow, &g->width, &g->height);
  if (SDL_ASSERT_LEVEL > 2) printf("Size: %d, %d\n", g->width, g->height);
  g->screen_texture = SDL_CreateTexture(g->sdlRenderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        g->width, g->height);

  g->myPixels = (Uint32 *)calloc(g->width*g->height, sizeof(Uint32));

  bzero(&g->f, sizeof(Flames));

  g->size = 50*min(g->width, g->height)/100;
  g->fractal_texture = SDL_CreateTexture(g->sdlRenderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         g->size, g->size);

  const int topborder = (g->height - g->size)/2;
  const int leftborder = (g->width - g->size)/2;
  g->x = leftborder;
  g->y = topborder;

  g->hist = (HistogramEntry *)calloc(g->size*g->size, sizeof(HistogramEntry));
  g->renderme = NULL;

  g->frame_time = 100;

  SDL_AtomicSet(&g->done, 0);
  SDL_AtomicSet(&g->dirty, 1);

  g->buffer = (Uint32 *)calloc(g->size*g->size, sizeof(Uint32));
  g->buffer_filler = SDL_CreateThread((SDL_ThreadFunction)FillBuffer, "Fill buffer", (void *)g);

  g->font = TTF_OpenFont( "LiberationMono-Regular.ttf", 28 );
  if (!g->font)
    g->font = TTF_OpenFont( "/usr/share/fonts/truetype/ttf-liberation/LiberationMono-Regular.ttf", 28 );
  if (!g->font) exitMessage("Unable to open font");

  g->false_positives = g->true_positives = g->false_negatives = g->true_negatives = 0;
  g->games_won = 0;

  SetOriginal(g);
}

void TweakFlame(Flames *f, Flames *o, Tweak t) {
  if (t & COPYCOLOR) {
    for (int i=0;i<MAX_TRANS; i++) {
      f->Transformations[i].R = o->Transformations[i].R;
      f->Transformations[i].G = o->Transformations[i].G;
      f->Transformations[i].B = o->Transformations[i].B;
      f->Transformations[i].A = o->Transformations[i].A;
    }
  }
  if (t & COPYSHAPE0) {
    o->Transformations[0].R = f->Transformations[0].R;
    o->Transformations[0].G = f->Transformations[0].G;
    o->Transformations[0].B = f->Transformations[0].B;
    o->Transformations[0].A = f->Transformations[0].A;
    f->Transformations[0] = o->Transformations[0];
  }
  if (t & COPYSHAPE1) {
    o->Transformations[1].R = f->Transformations[1].R;
    o->Transformations[1].G = f->Transformations[1].G;
    o->Transformations[1].B = f->Transformations[1].B;
    o->Transformations[1].A = f->Transformations[1].A;
    f->Transformations[1] = o->Transformations[1];
  }
  if (t & COPYSHAPE2) {
    o->Transformations[2].R = f->Transformations[2].R;
    o->Transformations[2].G = f->Transformations[2].G;
    o->Transformations[2].B = f->Transformations[2].B;
    o->Transformations[2].A = f->Transformations[2].A;
    f->Transformations[2] = o->Transformations[2];
  }
  if (t & COPYSHAPE3) {
    o->Transformations[3].R = f->Transformations[3].R;
    o->Transformations[3].G = f->Transformations[3].G;
    o->Transformations[3].B = f->Transformations[3].B;
    o->Transformations[3].A = f->Transformations[3].A;
    f->Transformations[3] = o->Transformations[3];
  }
  if (t & COPYSYMMETRY) {
    f->N = o->N;
    for (int i=num_trans; i<f->N; i++) {
      f->Transformations[i] = o->Transformations[i];
    }
  }
  if (t & COPYALLBUTSYMMETRY) {
    for (int i=0; i<num_trans; i++) {
      f->Transformations[i] = o->Transformations[i];
    }
  }
  if (t & COPYGRAY) {
    for (int i=0;i<MAX_TRANS; i++) {
      double gray = o->Transformations[i].R/3 +
                    o->Transformations[i].G/3 +
                    o->Transformations[i].B/3;
      f->Transformations[i].R = gray;
      f->Transformations[i].G = gray;
      f->Transformations[i].B = gray;
      f->Transformations[i].A = o->Transformations[i].A;
    }
  }
}

static inline int is_original(TweakedSeed seed) {
  seed.tweak |= seed.alltweak;
  return seed.tweak == COPYORIGINAL ||
    seed.seed == seed.original ||
    (seed.tweak & COPYSHAPE0 &&
     seed.tweak & COPYSHAPE1 &&
     seed.tweak & COPYSHAPE2 &&
     seed.tweak & COPYSHAPE3 &&
     seed.tweak & COPYSYMMETRY &&
     seed.tweak & COPYGRAY) ||
    (seed.tweak & COPYSHAPE0 &&
     seed.tweak & COPYSHAPE1 &&
     seed.tweak & COPYSHAPE2 &&
     seed.tweak & COPYSHAPE3 &&
     seed.tweak & COPYSYMMETRY &&
     seed.tweak & COPYCOLOR) ||
    (seed.tweak & COPYSYMMETRY && seed.tweak & COPYALLBUTSYMMETRY);
}

void ShowTweaked(char *buffer, TweakedSeed seed) {
  seed.tweak |= seed.alltweak;
  buffer += sprintf(buffer, "%s => ", seed.str);
  if (is_original(seed)) {
    sprintf(buffer, "original: %d", seed.original);
    return;
  }
  if (seed.tweak == COPYSHAPE) {
    sprintf(buffer, "copy shape: %d", seed.seed);
    return;
  }
  const char *separator = "";
  if (seed.tweak & COPYCOLOR) {
    buffer += sprintf(buffer, "%scolor", separator);
    separator = ", ";
  }
  if (seed.tweak & COPYSHAPE0 &&
      seed.tweak & COPYSHAPE1 &&
      seed.tweak & COPYSHAPE2 &&
      seed.tweak & COPYSHAPE3 &&
      !(seed.tweak & COPYALLBUTSYMMETRY)) {
    buffer += sprintf(buffer, "%scopy shape", separator);
    separator = ", ";
  } else {
    if (seed.tweak & COPYSHAPE0 && !(seed.tweak & COPYALLBUTSYMMETRY)) {
      buffer += sprintf(buffer, "%sshape 0", separator);
      separator = ", ";
    }
    if (seed.tweak & COPYSHAPE1 && !(seed.tweak & COPYALLBUTSYMMETRY)) {
      buffer += sprintf(buffer, "%sshape 1", separator);
      separator = ", ";
    }
    if (seed.tweak & COPYSHAPE2 && !(seed.tweak & COPYALLBUTSYMMETRY)) {
      buffer += sprintf(buffer, "%sshape 2", separator);
      separator = ", ";
    }
    if (seed.tweak & COPYSHAPE3 && !(seed.tweak & COPYALLBUTSYMMETRY)) {
      buffer += sprintf(buffer, "%sshape 3", separator);
      separator = ", ";
    }
  }
  if (seed.tweak & COPYSYMMETRY) {
    buffer += sprintf(buffer, "%ssame symmetry", separator);
    separator = ", ";
  }
  if (seed.tweak & COPYALLBUTSYMMETRY) {
    buffer += sprintf(buffer, "%ssame except for symmetry", separator);
    separator = ", ";
  }
  sprintf(buffer, ": %d", seed.seed);
}

Flames CreateFlame(TweakedSeed seed) {
  Flames f, original;
  SecureRandom s;
  SecureRandom o;
  init_secure_random_from_both(&s, seed.str, seed.seed);
  init_secure_random_from_both(&o, seed.str, seed.original);
  InitFlames(&f, &s);
  InitFlames(&original, &o);
  Flames originalcopy = original;
  TweakFlame(&f, &originalcopy, seed.tweak);
  TweakFlame(&f, &original, seed.alltweak);
  f.r = original.r; // use original random # seed, so identical really is identical
  return f;
}

void SetFlame(SingleHistogramGame *g, TweakedSeed seed) {
  /* static SDL_Haptic *haptic = (void *)-1; */
  /* if (haptic == (void *)-1) { */
  /*   haptic = SDL_HapticOpen(0); */
  /*   if (SDL_HapticRumbleInit(haptic)) haptic = NULL; */
  /* } */
  /* if (haptic != NULL) { */
  /*   SDL_HapticRumblePlay(haptic, 1.0, 100); */
  /* } */
  g->f = CreateFlame(seed);
  bzero(g->hist, g->size*g->size*sizeof(HistogramEntry));
  bzero(g->buffer, g->size*g->size*sizeof(Uint32));
  const double quality = 3;
  if (g->renderme == NULL) {
    // create a new render thread
    g->renderme = ComputeInThread(&g->done, &g->dirty, &g->f, g->size, quality, g->hist);
  } else {
    // inform the existing thread that we need to render this
    SDL_SemPost(g->renderme);
  }
}

void UpdateFractalTexture(SingleHistogramGame *g) {
  SDL_UpdateTexture(g->fractal_texture, NULL, g->buffer, g->size * sizeof (Uint32));
}

void Draw(SingleHistogramGame *g) {
  static int count = 0;
  if (SDL_AtomicGet(&g->dirty)) {
    count++;
    SDL_AtomicSet(&g->dirty, 0);

    const int gray = (count & 0) ? 0xFF : 30;
    /* memset(g->myPixels, gray, sizeof(Uint32)*g->width*g->height); */
    /* /\* ReadHistogram(g->size, g->x, g->y, g->width, g->height, g->hist, g->myPixels); *\/ */
    /* for (int ix=0; ix<g->size; ix++) */
    /*   if (ix + g->x < g->width && ix + g->x >= 0) */
    /*     for (int iy=0; iy<g->size; iy++) */
    /*       if (iy + g->y < g->height && iy + g->y >= 0) */
    /*         g->myPixels[(g->x+ix)+g->width*(g->y+iy)] = g->buffer[ix + g->size*iy]; */
    /* SDL_UpdateTexture(g->screen_texture, NULL, g->myPixels, g->width * sizeof (Uint32)); */

    SDL_SetRenderDrawColor(g->sdlRenderer, gray, gray, gray, 255);
    SDL_RenderClear(g->sdlRenderer);
    SDL_Rect dst = {g->x, g->y, g->size, g->size};
    SDL_RenderCopy(g->sdlRenderer, g->fractal_texture, NULL, &dst);
    //SDL_RenderCopy(g->sdlRenderer, g->screen_texture, NULL, NULL);
    char *buffer = malloc(1024);
    ShowTweaked(buffer, g->on_display);
    renderTextAt(g, buffer, 50, 90);
    if (g->on_display.seed == g->original.seed) {
      sprintf(buffer, "Remember this shape carefully!");
    } else {
      sprintf(buffer, "false negatives:  %2d/%2d   false positives:  %2d/%2d",
              g->false_negatives, g->true_negatives,
              g->false_positives, g->true_positives);
    }
    renderTextAt(g, buffer, 50, 130);
    sprintf(buffer, "games won:  %2d", g->games_won);
    renderTextAt(g, buffer, 50, 170);
    SDL_RenderPresent(g->sdlRenderer);
  }
}

void HandleKey(SingleHistogramGame *g, SDL_Keycode c) {
  switch (c) {
  case SDLK_q:
    SDL_AtomicSet(&g->done, 1);
    break;
  case SDLK_z:
    /* for (int i=0;i<6;i++) { */
    /*   bzero(hist[i], size*size*sizeof(HistogramEntry)); */
    /* } */
    break;
  case SDLK_s:
    HandleRight(g);
    /* for (int i=0;i<6;i++) { */
    /*   SecureRandom s; */
    /*   init_secure_random_from_int(&s, flame_number++); */
    /*   InitFlames(&f[i], &s); */
    /*   bzero(hist[i], size*size*sizeof(HistogramEntry)); */
    /*   ComputeInThread(&f[i], size, quality, hist[i]); */
    /* } */
    break;
  case SDLK_j:
    g->frame_time /= 2;
    break;
  case SDLK_RIGHT:
    HandleRight(g);
    break;
  case SDLK_LEFT:
    HandleLeft(g);
    break;
  case SDLK_UP:
    HandleUp(g);
    break;
  case SDLK_DOWN:
    HandleDown(g);
    break;
  case SDLK_k:
    g->frame_time *= 2;
    break;
    //default:
    //exitMessage("Unrecognized key pressed");
  }
}

static void NextGuess(SingleHistogramGame *g) {
  const double frac_original = 1.0/10;
  const double randd = (quickrand32(&g->f.r) % g->on_display.seed) / (double) g->on_display.seed;
  g->on_display.seed++;
  if (randd > frac_original) {
    do {
      int tweakness = quickrand32(&g->f.r) % 100;
      if (tweakness < 5) {
        g->on_display.tweak = NOTWEAK;
      } else if (tweakness < 10) {
        g->on_display.tweak = COPYSHAPE0 | COPYSHAPE1;
      } else if (tweakness < 15) {
        g->on_display.tweak = COPYSHAPE1 | COPYSHAPE2;
      } else if (tweakness < 20) {
        g->on_display.tweak = COPYSHAPE2 | COPYSHAPE3;
      } else if (tweakness < 25) {
        g->on_display.tweak = COPYSHAPE3 | COPYSHAPE0;
      } else if (tweakness < 30) {
        g->on_display.tweak = COPYSHAPE;
      } else if (tweakness < 35) {
        g->on_display.tweak = COPYSHAPE0;
      } else if (tweakness < 40) {
        g->on_display.tweak = COPYSHAPE1;
      } else if (tweakness < 45) {
        g->on_display.tweak = COPYSHAPE2;
      } else if (tweakness < 50) {
        g->on_display.tweak = COPYSHAPE3;
      } else if (tweakness < 70) {
        g->on_display.tweak = COPYALLBUTSYMMETRY;
      } else if (tweakness < 80) {
        g->on_display.tweak = COPYCOLOR;
      } else {
        g->on_display.tweak = COPYSYMMETRY;
      }
    } while (is_original(g->on_display));
  } else {
    g->on_display.tweak = COPYORIGINAL;
  }
  //printf("num: %d  original: %d  tweak %d\n",
  //       g->on_display.seed, g->on_display.original, g->on_display.tweak);
  SetFlame(&game, g->on_display);
}

void HandleLeft(SingleHistogramGame *g) {
  if (g->on_display.seed != g->original.seed) {
    if (is_original(g->on_display)) {
      g->false_negatives += 1;
    } else {
      g->true_negatives += 1;
    }
  }
  NextGuess(g);
}

void HandleRight(SingleHistogramGame *g) {
  if (g->on_display.seed != g->original.seed) {
    if (is_original(g->on_display)) {
      g->true_positives += 1;
    } else {
      g->false_positives += 1;
    }
  }
  if (g->true_positives == 3) {
    // We won a game!
    g->false_positives = g->true_positives = g->false_negatives = g->true_negatives = 0;
    g->games_won++;
    SetOriginal(g);
  } else {
    NextGuess(g);
  }
}

void HandleUp(SingleHistogramGame *g) {
  SetOriginal(g);
}

void HandleDown(SingleHistogramGame *g) {
  SetOriginal(g);
}

void HandleMouse(SingleHistogramGame *g, int x, int y) {
  SDL_Event event;
  int next_frame = 0;
  int oldx = g->x; // , oldy = g->y;
  const int topborder = (g->height - g->size)/2;
  const int leftborder = (g->width - g->size)/2;
  const int centerx = leftborder;
  const int centery = topborder;
  while (!SDL_AtomicGet(&game.done)) {
    int now = SDL_GetTicks();
    if (now >= next_frame) {
      Draw(&game);
      // set the next time to draw:
      next_frame = now + game.frame_time;
    }
    /* Check for new events */
    if (SDL_WaitEventTimeout(&event, next_frame - now)) {
      /* If a quit event has been sent */
      if (event.type == SDL_QUIT) {
        /* Quit the application */
        SDL_AtomicSet(&game.done, 1);
      }
      switch (event.type) {
      case SDL_KEYDOWN:
        HandleKey(&game, event.key.keysym.sym);
        break;
      case SDL_MOUSEMOTION:
        g->x = oldx + event.motion.x - x;
        //g->y = oldy + event.motion.y - y;
        break;
      case SDL_MOUSEBUTTONUP:
        // go back!
        g->x = oldx;
        if (event.button.x - x > g->width/2) {
          g->x = centerx;
          g->y = centery;
          HandleRight(g);
          return;
        }
        if (event.button.x - x < -g->width/2) {
          g->x = centerx;
          g->y = centery;
          HandleLeft(g);
          return;
        }
        if (event.button.y - y < -g->height/2) {
          g->x = centerx;
          g->y = centery;
          HandleUp(g);
          return;
        }
        if (event.button.y - y > g->height/2) {
          g->x = centerx;
          g->y = centery;
          HandleDown(g);
          return;
        }
        return;
      }
    }
  }
}
