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
#include "fractal-flames.h"
#include <stdlib.h>
#include <string.h>

SingleHistogramGame game;

int main(int argc, char *argv[]) {
  Init(&game);

  SDL_Event event;
  int next_frame = game.frame_time;
  /* While the program is running */
  while (!SDL_AtomicGet(&game.done)) {
    int now = SDL_GetTicks();
    if (now >= next_frame) {
      if (SDL_AtomicGet(&game.display_on)) { // only do drawing when the window is visible
        UpdateFractalTexture(&game); // this can be slow, but this is the "slow" event loop.
        Draw(&game);
        // set the next time to draw:
        next_frame = now + game.frame_time;
      } else {
        next_frame = now + 100*game.frame_time;
      }
    }
    /* Check for new events */
    if (SDL_WaitEventTimeout(&event, next_frame - now)) {
      switch (event.type) {
      case SDL_QUIT:
        /* Quit the application */
        SDL_AtomicSet(&game.done, 1);
        break;
      case SDL_KEYDOWN:
        HandleKey(&game, event.key.keysym.sym);
        break;
      case SDL_MOUSEBUTTONDOWN:
        HandleMouse(&game, event.button.x, event.button.y);
        break;
      case SDL_FINGERDOWN:
        // Fingers are handled by mouse events
        break;
      case SDL_APP_WILLENTERBACKGROUND:
        PauseGame(&game);
        break;
      case SDL_APP_DIDENTERFOREGROUND:
        ResumeGame(&game);
        break;
      case SDL_WINDOWEVENT:
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SHOWN:
          ResumeGame(&game);
          break;
        case SDL_WINDOWEVENT_HIDDEN:
          PauseGame(&game);
          break;
        }
        break;
      }
    }
  }
  printf("All done!\n");
  return 0;
}
