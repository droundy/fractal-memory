#include "secure-random.h"
#include "sha512.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void init_secure_random(SecureRandom *s, const char *seed) {
  int len = strlen(seed)+1;
  char *seedcpy = malloc(len);
  s->seed = seedcpy;
  memcpy(seedcpy, seed, len);

  SHA512Context sc;
  SHA512Init(&sc);
  SHA512Update(&sc, s->seed, len);
  SHA512Final(&sc, s->randombytes);
  s->bytes_left = 64;
}
void init_secure_random_from_int(SecureRandom *s, int seed) {
  char *foo = malloc(100);
  sprintf(foo, "%d", seed);
  init_secure_random(s, foo);
  free(foo);
}

uint8_t secure_random(SecureRandom *s) {
  if (!s->bytes_left) {
    // hash our last hash to get the next set of bytes
    SHA512Context sc;
    SHA512Init(&sc);
    SHA512Update(&sc, s->randombytes, 64);
    SHA512Final(&sc, s->randombytes);
    s->bytes_left = 64;
  }
  s->bytes_left--;
  return s->randombytes[s->bytes_left];
}
uint16_t secure_random16(SecureRandom *s) {
  return secure_random(s) + (1<<8)*(uint16_t)secure_random(s);
}
uint32_t secure_random32(SecureRandom *s) {
  return secure_random16(s) + (1<<16)*(uint32_t)secure_random16(s);
}
