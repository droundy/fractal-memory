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
    if (SDL_AtomicGet(&g->display_on)) {
      ReadHistogram(g->size, 0, 0, g->size, g->size, g->hist, g->buffer);
    }
    SDL_Delay(g->frame_time);
  }
  return 0;
}

void Init(SingleHistogramGame *g) {
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

  if(TTF_Init()==-1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }

  SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
                              &g->sdlWindow, &g->sdlRenderer);
  if (!g->sdlRenderer || !g->sdlWindow) {
    exitMessage("Unable to create window!");
  }

  SDL_GetWindowSize(g->sdlWindow, &g->width, &g->height);
  if (SDL_ASSERT_LEVEL > 2) printf("Size: %d, %d\n", g->width, g->height);
  g->screen_texture = SDL_CreateTexture(g->sdlRenderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        g->width, g->height);

  g->myPixels = (Uint32 *)calloc(g->width*g->height, sizeof(Uint32));

  bzero(&g->f, sizeof(Flames));

  g->size = 2*min(g->width, g->height)/3;
  g->fontsize = min(g->width/40, g->height/40);
  printf("fontsize %d\n", g->fontsize);
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

  SDL_AtomicSet(&g->done, 0);
  ResumeGame(g); // sets frame_time, and dirty buffers, etc.

  g->buffer = (Uint32 *)calloc(g->size*g->size, sizeof(Uint32));
  g->buffer_filler = SDL_CreateThread((SDL_ThreadFunction)FillBuffer, "Fill buffer", (void *)g);

  g->font = TTF_OpenFont( "LiberationMono-Regular.ttf", g->fontsize );
  if (!g->font)
    g->font = TTF_OpenFont( "/usr/share/fonts/truetype/ttf-liberation/LiberationMono-Regular.ttf",
                            g->fontsize );
  if (!g->font) exitMessage("Unable to open font");

  g->false_positives = g->true_positives = g->false_negatives = g->true_negatives = 0;
  g->games_won = 0;

  SetOriginal(g);
}

void UpdateFractalTexture(SingleHistogramGame *g) {
  if (SDL_AtomicGet(&g->dirty) && SDL_AtomicGet(&g->display_on)) {
    SDL_UpdateTexture(g->fractal_texture, NULL, g->buffer, g->size * sizeof (Uint32));
    SDL_AtomicSet(&g->bufferdirty, 1);
  }
}

void SaveToFile(SingleHistogramGame *g, const char *fname) {
  /* SDL_Surface *surface = */
  /*   SDL_CreateRGBSurfaceFrom(g->buffer, g->size, g->size, */
  /*                            32, g->size*4, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); */
  /* SDL_SaveBMP(surface, fname); */
  /* SDL_FreeSurface(surface); */
  SaveHistogram(g->size, g->hist, fname);
}

void Draw(SingleHistogramGame *g) {
  static int count = 0;
  if (SDL_AtomicGet(&g->bufferdirty) && SDL_AtomicGet(&g->display_on)) {
    count++;
    SDL_AtomicSet(&g->bufferdirty, 0);

    const int gray = (count & 0) ? 0xFF : 30;

    SDL_SetRenderDrawColor(g->sdlRenderer, gray, gray, gray, 255);
    SDL_RenderClear(g->sdlRenderer);

    {
      SDL_SetRenderDrawColor(g->sdlRenderer, 190, gray, gray, 255);
      SDL_Rect dst = {g->width/2, g->height*17/20, g->width/4, g->height/10};
      SDL_RenderFillRect(g->sdlRenderer, &dst);
      renderTextAt(g, "Good", g->width/2+10, g->height*17/20+10);
    }
    {
      SDL_Rect dst = {g->width/4, g->height*17/20, g->width/4, g->height/10};
      SDL_SetRenderDrawColor(g->sdlRenderer, gray, 190, gray, 255);
      SDL_RenderFillRect(g->sdlRenderer, &dst);
      renderTextAt(g, "Bad", g->width/4+10, g->height*17/20+10);
    }

    SDL_Rect dst = {g->x, g->y, g->size, g->size};
    //SDL_SetRenderDrawColor(g->sdlRenderer, 0, 0, 0, 255);
    //SDL_RenderFillRect(g->sdlRenderer, &dst);
    //SDL_SetTextureBlendMode(g->fractal_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(g->fractal_texture, SDL_BLENDMODE_NONE);
    SDL_RenderCopy(g->sdlRenderer, g->fractal_texture, NULL, &dst);
    //SDL_RenderCopy(g->sdlRenderer, g->screen_texture, NULL, NULL);
    char *buffer = malloc(1024);
    ShowTweaked(buffer, g->on_display);
    renderTextAt(g, buffer, g->fontsize, g->fontsize);
    if (g->on_display.seed == g->original.seed) {
      sprintf(buffer, "Remember this shape carefully!");
    } else {
      sprintf(buffer, "false negatives:  %2d/%2d   false positives:  %2d/%2d",
              g->false_negatives, g->true_negatives,
              g->false_positives, g->true_positives);
    }
    renderTextAt(g, buffer, g->fontsize, 5*g->fontsize/2);
    sprintf(buffer, "games won:  %2d", g->games_won);
    renderTextAt(g, buffer, g->fontsize, 8*g->fontsize/2);
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
    SaveToFile(g, "fractal.pam");
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

void PauseGame(SingleHistogramGame *g) {
  SDL_AtomicSet(&game.display_on, 0);
  g->frame_time = 1000; // once per second.
}

void ResumeGame(SingleHistogramGame *g) {
  SDL_AtomicSet(&g->bufferdirty, 1);
  SDL_AtomicSet(&g->dirty, 1);
  SDL_AtomicSet(&game.display_on, 1);
  g->frame_time = 300;
}
