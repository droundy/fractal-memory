#pragma once

// This code is translated from the Wikipedia page on random
// number generation.

typedef struct {
  uint32_t m_w, m_z;
} QuickRandom;

static inline void init_quick(QuickRandom *s) {
  s->m_w = 1;
  s->m_z = 2;
}
static inline uint32_t quickrand32(QuickRandom *s) {
  s->m_z = 36969 * (s->m_z & 65535) + (s->m_z >> 16);
  s->m_w = 18000 * (s->m_w & 65535) + (s->m_w >> 16);
  return (s->m_z << 16) + s->m_w;  // 32-bit result
}
