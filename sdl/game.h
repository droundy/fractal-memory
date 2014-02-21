#pragma once

#include <stdio.h>
#include <SDL.h>

static void exitMessage(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                           "Fatal error!",
                           msg,
                           NULL);
  SDL_Quit();
  exit(1);
}
