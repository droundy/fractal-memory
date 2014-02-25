/*  Fractal memory is a memory game that studies human memory
    Copyright (C) 2014 David Roundy

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
    TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.  */

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

void init_secure_random_from_both(SecureRandom *s, const char *str, int seed) {
  char *foo = malloc(500);
  sprintf(foo, "%s%d", str, seed);
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
