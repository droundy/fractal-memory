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

#include "fractal-flames.h"

void PrintAffine(AffineTransformation *t) {
  fprintf(stderr, "M:  [ %10g %10g ]\n", t->Mxx, t->Myx);
  fprintf(stderr, "    [ %10g %10g ]\n", t->Mxy, t->Myy);
  fprintf(stderr, "O:  [ %10g %10g ]\n", t->Ox, t->Oy);
}

void PrintTransform(Transformation *t, int i) {
  fprintf(stderr, "[%2d] Transform (types", i);
  if (t->NTypes == 0) {
    fprintf(stderr, " symmetry %s", show_type(t->Type[0]));
  } else {
    for (int i=0;i<t->NTypes;i++) {
      fprintf(stderr, " %s", show_type(t->Type[i]));
    }
  }
  fprintf(stderr, "):\n");
  PrintAffine(&t->Pre);
  fprintf(stderr, "RGBA: ( %10g %10g %10g %10g )\n", t->R, t->G, t->B, t->A);
  PrintAffine(&t->Post);
}

void PrintFlames(Flames *f) {
  fprintf(stderr, "Flames: (size %d)\n", f->N);
  for (int i=0; i<f->N; i++) {
    PrintTransform(&f->Transformations[i], i);
  }
}
