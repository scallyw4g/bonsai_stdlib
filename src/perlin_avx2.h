#pragma once

#include "primitives.h"
#include "vector.h"
#include "simd_avx2.h"

struct perlin_params
{
  u32_8x P0;
  u32_8x P1;
  f32_8x Fract0;
  f32_8x Fract1;
  f32_8x Fade;
};

struct perlin_inputs
{
  perlin_params *xParams;
  perlin_params *yParams;
  perlin_params *zParams;
  u32 *_xCoords;
  f32_8x *Hashes;
};


poof(gen_constructor(perlin_params))
#include <generated/gen_constructor_perlin_params.h>

link_inline perlin_params
PerlinParams(u32 P0, u32 P1, f32 Fract0, f32 Fract1, f32 Fade)
{
  perlin_params Result =
  {
    U32_8X(P0),
    U32_8X(P1),
    F32_8X(Fract0),
    F32_8X(Fract1),
    F32_8X(Fade),
  };
  return Result;
}

link_internal f32_8x
FadeQuintic(f32_8x t) {
  f32_8x _15 = F32_8X(15);
  f32_8x _10 = F32_8X(10);
  f32_8x _6  = F32_8X(6);
  f32_8x res = t * t * t * (t * (t * _6 - _15) + _10);
  return res;
}

link_internal f32
FadeQuintic(f32 t) {
  f32 res = t * t * t * (t * (t * 6 - 15) + 10);
  return res;
}

link_inline perlin_params
ComputePerlinParameters_scalar(u32 Basis, u32 Offset, u32 ChunkResolution, u32 Period, u32 Prime)
{
#if 1
  auto AbsoluteWorldP = (Basis + Offset*ChunkResolution);

  auto Cell = AbsoluteWorldP / Period;
  auto Rem = AbsoluteWorldP - (Cell * Period);

  auto Fract0 = f32(Rem)/f32(Period);
  auto Fract1 = Fract0 - 1.f;

#else
  auto Input = (f32(Basis) + f32(Offset)*f32(ChunkResolution)) 
                  / f32(Period);
  auto Cellf = Floorf(Input);
  auto Cell = f32(Cellf);
  auto Fract0 = Input-Cellf;
  auto Fract1 = Fract0 - 1.f;
#endif

  auto P0 = Cell * Prime;
  auto P1 = P0 + Prime;

  auto Fade = FadeQuintic(Fract0);

  perlin_params Result = PerlinParams( u32(P0), u32(P1), Fract0, Fract1, Fade );
  return Result;
}

link_inline perlin_params
ComputePerlinParameters_vector(u32_8x Basis, u32_8x Offset, u32_8x ChunkResolution, avx_divisor Period, u32_8x Prime)
{
#if 1
  auto AbsoluteWorldP = (Basis + Offset*ChunkResolution);

  auto Cell = AbsoluteWorldP / Period;
  auto Rem = AbsoluteWorldP - (Cell * U32_8X(Period.E));

  auto Fract0 = F32_8X(Rem)/F32_8X(Period.E);
  auto Fract1 = Fract0 - F32_8X(1);

#else
  f32_8x Input = (F32_8X(Basis) + F32_8X(Offset)*F32_8X(ChunkResolution)) 
                  / F32_8X(Period.E);
  f32_8x Cellf = Floor(Input);
  u32_8x Cell = U32_8X(Cellf);
  f32_8x Fract0 = Input-Cellf;
  f32_8x Fract1 = Fract0 - F32_8X(1);
#endif

  auto P0 = Cell * Prime;
  auto P1 = P0 + Prime;

  auto Fade = FadeQuintic(Fract0);

  perlin_params Result = PerlinParams( P0, P1, Fract0, Fract1, Fade );
  return Result;
}

link_internal f32_8x
Grad8x(u32_8x *hash_, f32_8x x, f32_8x y, f32_8x z)
{
  u32_8x hash = *hash_;

  u32_8x _15 = U32_8X(15);
  u32_8x _14 = U32_8X(14);
  u32_8x _12 = U32_8X(12);
  u32_8x _8  = U32_8X(8);
  u32_8x _4  = U32_8X(4);
  u32_8x _2  = U32_8X(2);
  u32_8x _1  = U32_8X(1);
  f32_8x _n1 = F32_8X(-1);

  auto h = hash & _15;

  u32_8x uSel = h < _8;
  u32_8x vSel = h < _4;
  f32_8x u  = Select(uSel, x, y);

#if 1
  u32_8x xSel = (h == _12 | h == _14 );
  f32_8x xz = Select(xSel, x, z);
  f32_8x v  = Select(vSel, y, xz);
#else
  f32_8x v  = Select(vSel, y, z);
#endif

  auto h1 = hash << 31;
  auto h2 = (hash & U32_8X(2)) << 30 ;
  f32_8x Result = ( u ^ Cast_f32_8x(h1) ) + ( v ^ Cast_f32_8x(h2) );

  return Result;
}

link_internal f32_8x
Grad8x(u32_8x hash, f32_8x x, f32_8x y, f32_8x z)
{
  u32_8x _15 = U32_8X(15);
  u32_8x _14 = U32_8X(14);
  u32_8x _12 = U32_8X(12);
  u32_8x _8  = U32_8X(8);
  u32_8x _4  = U32_8X(4);
  u32_8x _2  = U32_8X(2);
  u32_8x _1  = U32_8X(1);
  f32_8x _n1 = F32_8X(-1);

  auto h = hash & _15;

  u32_8x uSel = h < _8;
  u32_8x vSel = h < _4;
  f32_8x u  = Select(uSel, x, y);

#if 1
  u32_8x xSel = (h == _12 | h == _14 );
  f32_8x xz = Select(xSel, x, z);
  f32_8x v  = Select(vSel, y, xz);
#else
  f32_8x v  = Select(vSel, y, z);
#endif

  auto h1 = hash << 31;
  auto h2 = (hash & U32_8X(2)) << 30 ;
  f32_8x Result = ( u ^ Cast_f32_8x(h1) ) + ( v ^ Cast_f32_8x(h2) );

  return Result;
}

// @Pannoniae on discord suggested this alternative formulation for Grad8x,
// which produced a ~13.5% speedup, but may produce less entropy.
//
// > Pannoniae - Yesterday at 10:19 PM
// > well yeah, the only drawback is that this one only generates 8 unique grads
// > instead of the original one where you have 12 real ones and 4 repeated
// > ones slapped on the end lol
//
link_internal f32_8x
Grad8x_pannoniae(u32_8x hash, f32_8x x, f32_8x y, f32_8x z)
{
  u32_8x sx = (hash & U32_8X(1)) << 31;
  u32_8x sy = (hash & U32_8X(2)) << 30;
  u32_8x sz = (hash & U32_8X(4)) << 29;

  return (x ^ Cast_f32_8x(sx)) +
         (y ^ Cast_f32_8x(sy)) +
         (z ^ Cast_f32_8x(sz));
}


link_internal f32_8x
Lerp8x(f32_8x t, f32_8x a, f32_8x b)
{
  // Slowest
  /* f32_8x res = b*t + (F32_8X(1.f)-t)*a; */

  // Fast
  f32_8x res = FMA(b-a, t, a);

  // Also fast
  /* f32_8x res = FMA(t, b, FMA(F32_8X(-1)*t, a, a)); */

  return res;
}


void PerlinNoise(   f32 *NoiseValues,
                     v3  Period,
                    f32  Amplitude,
                    v3i  NoiseDim,
                    v3i  NoiseBasis,
                    s32  OctaveCount,
          perlin_inputs *Inputs);



