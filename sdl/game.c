#include "game.h"
#include "fractal-flames.h"
#include <stdlib.h>
#include <string.h>

int dirty = 1;

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
  SDL_Window *sdlWindow;
  SDL_Renderer *sdlRenderer;
  SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &sdlWindow, &sdlRenderer);
  if (!sdlRenderer || !sdlWindow) {
    exitMessage("Unable to create window!");
  }
  /* if (TTF_Init() < 0) { */
  /*   fprintf(stderr, "Unable to create text!\n"); */
  /*   return 1; */
  /* } */

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.

  //SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  //SDL_RenderClear(sdlRenderer);
  //SDL_RenderPresent(sdlRenderer);

  int width, height;
  SDL_GetWindowSize(sdlWindow, &width, &height);
  if (SDL_ASSERT_LEVEL > 2) printf("Size: %d, %d\n", width, height);
  SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer,
                                              SDL_PIXELFORMAT_ARGB8888,
                                              SDL_TEXTUREACCESS_STREAMING,
                                              width, height);

  Uint32 *myPixels = (Uint32 *)calloc(width*height, 4);
  for (int i=0; i<width*height; i++) {
    myPixels[i] = 0xFFFF0000 + (i % 256) + 256*(i/4 % 256);
  }
  SDL_UpdateTexture(sdlTexture, NULL, myPixels, width * sizeof (Uint32));

  SDL_RenderClear(sdlRenderer);
  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
  SDL_RenderPresent(sdlRenderer);

  /* A bool to check if the program has exited */
  int quit = 0;

  Flames f[4];
  const int size = height*7/16;
  const double quality = 2.5;
  HistogramEntry *hist[4];
  for (int i=0;i<4;i++) {
    InitFlames(&f[i]);
    hist[i] = (HistogramEntry *)calloc(size*size, sizeof(HistogramEntry));
    ComputeInThread(&f[i], size, quality, hist[i]);
  }

  SDL_Event event;
  int frame_time = 100;
  int next_frame = frame_time;
  /* While the program is running */
  while (!quit) {
    int now = SDL_GetTicks();
    if (now >= next_frame && dirty) {
      dirty = 0; // so we can avoid redrawing this.
      // update window graphics
      for (int i=0; i<width*height; i++) {
        myPixels[i] = 0xFFFF0000 + ((now/frame_time+i) % 256) + 256*((now/frame_time + i)/4 % 256);
      }
      ReadHistogram(size, width, hist[0], myPixels);
      ReadHistogram(size, width, hist[1], myPixels+width/2);
      ReadHistogram(size, width, hist[2], myPixels+height/2*width);
      ReadHistogram(size, width, hist[3], myPixels+height/2*width + width/2);

      SDL_UpdateTexture(sdlTexture, NULL, myPixels, width * sizeof (Uint32));

      SDL_RenderClear(sdlRenderer);
      SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
      SDL_RenderPresent(sdlRenderer);
      // set the next time to draw:
      next_frame = now + frame_time;
    }
    /* Check for new events */
    if (SDL_WaitEventTimeout(&event, next_frame - now)) {
      /* If a quit event has been sent */
      if (event.type == SDL_QUIT) {
        /* Quit the application */
        quit = 1;
      }
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_q:
          quit = 1;
          break;
        case SDLK_z:
          for (int i=0;i<4;i++) {
            bzero(hist[i], size*size*sizeof(HistogramEntry));
          }
          break;
        case SDLK_s:
          for (int i=0;i<4;i++) {
            InitFlames(&f[i]);
            bzero(hist[i], size*size*sizeof(HistogramEntry));
            ComputeInThread(&f[i], size, quality, hist[i]);
          }
          break;
        case SDLK_j:
          frame_time /= 2;
          break;
        case SDLK_k:
          frame_time *= 2;
          break;
          //default:
          //exitMessage("Unrecognized key pressed");
        }
      }
    }
  }
  for (int i=0;i<4;i++) {
    InitFlames(&f[i]); // this triggers compute threads to stop.
  }
  return 0;
}
