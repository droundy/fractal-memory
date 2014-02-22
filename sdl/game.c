#include "game.h"
#include "fractal-flames.h"
#include <stdlib.h>
#include <string.h>

SingleHistogramGame game;

int main(int argc, char *argv[]) {
  Init(&game);

  int histnum = 1;

  SDL_Event event;
  int next_frame = game.frame_time;
  /* While the program is running */
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
      case SDL_MOUSEBUTTONDOWN:
        //SetFlame(&game, "", histnum++);
        HandleMouse(&game, event.button.x, event.button.y);
        break;
      case SDL_FINGERDOWN:
        //printf("Got finger down\n");
        //SetFlame(&game, "", histnum++);
        break;
      }
    }
  }
  printf("All done!\n");
  return 0;
}
