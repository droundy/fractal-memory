#include "game.h"

static inline int min(int a, int b) {
  return (a<b) ? a : b;
}

int FillBuffer(SingleHistogramGame *g) {
  while (!SDL_AtomicGet(&g->done)) {
    ReadHistogram(g->size, 0, 0, g->size, g->size, g->hist, g->buffer);
    SDL_Delay(g->frame_time);
  }
  return 0;
}

void Init(SingleHistogramGame *g) {
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
  SDL_Window *sdlWindow;
  SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
                              &sdlWindow, &g->sdlRenderer);
  if (!g->sdlRenderer || !sdlWindow) {
    exitMessage("Unable to create window!");
  }

  SDL_GetWindowSize(sdlWindow, &g->width, &g->height);
  if (SDL_ASSERT_LEVEL > 2) printf("Size: %d, %d\n", g->width, g->height);
  g->sdlTexture = SDL_CreateTexture(g->sdlRenderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    g->width, g->height);

  g->myPixels = (Uint32 *)calloc(g->width*g->height, sizeof(Uint32));

  bzero(&g->f, sizeof(Flames));

  g->size = 50*min(g->width, g->height)/100;

  const int topborder = (g->height - g->size)/2;
  const int leftborder = (g->width - g->size)/2;
  g->x = leftborder;
  g->y = topborder;

  g->hist = (HistogramEntry *)calloc(g->size*g->size, sizeof(HistogramEntry));
  g->renderme = NULL;

  g->original = SDL_GetTicks();
  g->on_display = g->original;
  SetFlame(g, "", g->on_display);

  g->frame_time = 100;

  SDL_AtomicSet(&g->done, 0);
  SDL_AtomicSet(&g->dirty, 1);

  g->buffer = (Uint32 *)calloc(g->size*g->size, sizeof(Uint32));
  g->buffer_filler = SDL_CreateThread((SDL_ThreadFunction)FillBuffer, "Fill buffer", (void *)g);
}

void SetFlame(SingleHistogramGame *g, const char *seed, int num) {
  /* static SDL_Haptic *haptic = (void *)-1; */
  /* if (haptic == (void *)-1) { */
  /*   haptic = SDL_HapticOpen(0); */
  /*   if (SDL_HapticRumbleInit(haptic)) haptic = NULL; */
  /* } */
  /* if (haptic != NULL) { */
  /*   SDL_HapticRumblePlay(haptic, 1.0, 100); */
  /* } */
  SecureRandom s;
  init_secure_random_from_both(&s, seed, num);
  // initialize the flame in a temporary so that we won't temporarily
  // set it to something invalid that confuses the render thread.
  Flames newf;
  InitFlames(&newf, &s);
  g->f = newf;
  bzero(g->hist, g->size*g->size*sizeof(HistogramEntry));
  const double quality = 3;
  if (g->renderme == NULL) {
    // create a new render thread
    g->renderme = ComputeInThread(&g->done, &g->dirty, &g->f, g->size, quality, g->hist);
  } else {
    // inform the existing thread that we need to render this
    SDL_SemPost(g->renderme);
  }
}

void Draw(SingleHistogramGame *g) {
  static int count = 0;
  if (SDL_AtomicGet(&g->dirty)) {
    count++;
    SDL_AtomicSet(&g->dirty, 0);

    const int gray = (count & 0) ? 0xFF : 30;
    memset(g->myPixels, gray, sizeof(Uint32)*g->width*g->height);
    /* ReadHistogram(g->size, g->x, g->y, g->width, g->height, g->hist, g->myPixels); */
    for (int ix=0; ix<g->size; ix++)
      if (ix + g->x < g->width && ix + g->x >= 0)
        for (int iy=0; iy<g->size; iy++)
          if (iy + g->y < g->height && iy + g->y >= 0)
            g->myPixels[(g->x+ix)+g->width*(g->y+iy)] = g->buffer[ix + g->size*iy];
    SDL_UpdateTexture(g->sdlTexture, NULL, g->myPixels, g->width * sizeof (Uint32));

    SDL_RenderClear(g->sdlRenderer);
    SDL_RenderCopy(g->sdlRenderer, g->sdlTexture, NULL, NULL);
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
    SetFlame(&game, "", ++g->on_display);
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
  case SDLK_k:
    g->frame_time *= 2;
    break;
    //default:
    //exitMessage("Unrecognized key pressed");
  }
}

void HandleLeft(SingleHistogramGame *g) {
  SetFlame(&game, "", --g->on_display);
}

void HandleRight(SingleHistogramGame *g) {
  SetFlame(&game, "", ++g->on_display);
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
        if (event.motion.x - x > g->width/2) {
          g->x = centerx;
          g->y = centery;
          HandleRight(g);
          return;
        }
        if (event.motion.x - x < -g->width/2) {
          g->x = centerx;
          g->y = centery;
          HandleLeft(g);
          return;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        // go back!
        g->x = oldx;
        return;
      }
    }
  }
}
