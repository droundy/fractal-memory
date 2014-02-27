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

void NextGuess(SingleHistogramGame *g) {
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

int is_original(TweakedSeed seed) {
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
