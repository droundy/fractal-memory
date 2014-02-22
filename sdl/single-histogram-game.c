#include "game.h"

static inline int min(int a, int b) {
  return (a<b) ? a : b;
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

  g->size = 80*min(g->width, g->height)/100;
  g->hist = (HistogramEntry *)calloc(g->size*g->size, sizeof(HistogramEntry));
  g->renderme = NULL;
  SetFlame(g, "", 0);

  g->frame_time = 1;

  SDL_AtomicSet(&g->done, 0);
  SDL_AtomicSet(&g->dirty, 1);
}

void SetFlame(SingleHistogramGame *g, const char *seed, int num) {
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
    //bzero(g->myPixels, sizeof(Uint32)*g->width*g->height);
    const int gray = (count & 0) ? 0xFF : 0;
    memset(g->myPixels, gray, sizeof(Uint32)*g->width*g->height);
    const int topborder = (g->height - g->size)/2;
    const int leftborder = (g->width - g->size)/2;
    ReadHistogram(g->size, g->width, g->hist,
                  g->myPixels + leftborder + topborder*g->width);

    SDL_UpdateTexture(g->sdlTexture, NULL, g->myPixels, g->width * sizeof (Uint32));

    SDL_RenderClear(g->sdlRenderer);
    SDL_RenderCopy(g->sdlRenderer, g->sdlTexture, NULL, NULL);
    SDL_RenderPresent(g->sdlRenderer);
  }
}

void HandleKey(SingleHistogramGame *g, SDL_Keycode c) {
  static int histnum = 1;
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
    SetFlame(&game, "", histnum++);
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
  case SDLK_k:
    g->frame_time *= 2;
    break;
    //default:
    //exitMessage("Unrecognized key pressed");
  }
}
