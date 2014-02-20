#include <inttypes.h>

typedef struct {
  uint8_t randombytes[64];
  int bytes_left;
  const char *seed;
} SecureRandom;

void init_secure_random(SecureRandom *, const char *);
void init_secure_random_from_int(SecureRandom *, int);
uint8_t secure_random(SecureRandom *);
uint16_t secure_random16(SecureRandom *);
uint32_t secure_random32(SecureRandom *);
