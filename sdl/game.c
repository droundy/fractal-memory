#include <stdio.h>
#include <SDL2/SDL.h>

void exitMessage(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                           "Fatal error!",
                           msg,
                           NULL);
  SDL_Quit();
  exit(1);
}

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

  SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(sdlRenderer);
  SDL_RenderPresent(sdlRenderer);

  int width, height;
  SDL_GetWindowSize(sdlWindow, &width, &height);
  printf("Size: %d, %d\n", width, height);
  SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer,
                                              SDL_PIXELFORMAT_ARGB8888,
                                              SDL_TEXTUREACCESS_STREAMING,
                                              width, height);

  Uint32 *myPixels = (Uint32 *)malloc(4*width*height);
  for (int i=0; i<width*height; i++) {
    myPixels[i] = 0xFFFF0000 + (i % 256) + 256*(i/4 % 256);
  }
  SDL_UpdateTexture(sdlTexture, NULL, myPixels, width * sizeof (Uint32));

  SDL_RenderClear(sdlRenderer);
  SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
  SDL_RenderPresent(sdlRenderer);

  /* A bool to check if the program has exited */
  int quit = 0;

  SDL_Event event;
  const int frame_time = 100;
  int next_frame = frame_time;
  /* While the program is running */
  while (!quit) {
    int now = SDL_GetTicks();
    if (now >= next_frame) {
      // update window graphics
      for (int i=0; i<width*height; i++) {
        myPixels[i] = 0xFFFF0000 + ((now/frame_time+i) % 256) + 256*((now/frame_time + i)/4 % 256);
      }
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
    }
  }

  exitMessage("This program doesn't work. Please fix it.");
  return 0;
}
