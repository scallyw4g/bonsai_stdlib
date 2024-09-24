
u32_8x
jFashHash(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  // Bad visual artifacts
  /* u32_8x hash = (Seed + x + y + z) >> 15; */

  // Fast, reasonable visual entropy, but stil artifacts compared to ChrisWellonsIntegerHash_lowbias32
  u32_8x hash = (x ^ y ^ z);
         hash = (hash >> 15) ^ hash;

  return hash;
}

u32_8x
HashPrimes(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  u32_8x hash = Seed;
  hash = hash ^ x;
  hash = hash ^ y;
  hash = hash ^ z;
  hash = hash * U32_8X( 0x27d4eb2d );
  hash = (hash >> 15) ^ hash;
  return hash;
}

// https://nullprogram.com/blog/2018/07/31/
//
u32_8x
ChrisWellonsIntegerHash_lowbias32(u32_8x x, u32_8x y, u32_8x z)
{
  u32_8x Result = x ^ y ^ z;

  Result = Result ^ (Result >> 16);
  Result = Result ^ U32_8X(0x7feb352d);
  Result = Result ^ (Result >> 15);
  Result = Result * U32_8X(0x846ca68b);
  Result = Result ^ (Result >> 16);
  return Result;
}

link_internal void
PerlinNoise_16x_avx2(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (u32 Index = 0; Index < 16; Index += 8)
  {

#if 0
    f32_8x G0 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0, perlinY->channel0, perlinZ->channel0), perlinX->f0, perlinY->f0, perlinZ->f0);
    f32_8x G1 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1, perlinY->channel0, perlinZ->channel0), perlinX->f1, perlinY->f0, perlinZ->f0);
    f32_8x G2 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0, perlinY->channel1, perlinZ->channel0), perlinX->f0, perlinY->f1, perlinZ->f0);
    f32_8x G3 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1, perlinY->channel1, perlinZ->channel0), perlinX->f1, perlinY->f1, perlinZ->f0);

    f32_8x G4 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0, perlinY->channel0, perlinZ->channel1), perlinX->f0, perlinY->f0, perlinZ->f1);
    f32_8x G5 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1, perlinY->channel0, perlinZ->channel1), perlinX->f1, perlinY->f0, perlinZ->f1);
    f32_8x G6 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0, perlinY->channel1, perlinZ->channel1), perlinX->f0, perlinY->f1, perlinZ->f1);
    f32_8x G7 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1, perlinY->channel1, perlinZ->channel1), perlinX->f1, perlinY->f1, perlinZ->f1);
#endif

#if 1
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel0, perlinZ->channel0), perlinX->f0, perlinY->f0, perlinZ->f0);
    f32_8x G1 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel0, perlinZ->channel0), perlinX->f1, perlinY->f0, perlinZ->f0);
    f32_8x G2 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel1, perlinZ->channel0), perlinX->f0, perlinY->f1, perlinZ->f0);
    f32_8x G3 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel1, perlinZ->channel0), perlinX->f1, perlinY->f1, perlinZ->f0);

    f32_8x G4 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel0, perlinZ->channel1), perlinX->f0, perlinY->f0, perlinZ->f1);
    f32_8x G5 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel0, perlinZ->channel1), perlinX->f1, perlinY->f0, perlinZ->f1);
    f32_8x G6 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel1, perlinZ->channel1), perlinX->f0, perlinY->f1, perlinZ->f1);
    f32_8x G7 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel1, perlinZ->channel1), perlinX->f1, perlinY->f1, perlinZ->f1);
#endif


#if 0
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x(jFashHash(Seed, perlinX->channel0, perlinY->channel0, perlinZ->channel0), perlinX->f0, perlinY->f0, perlinZ->f0);
    f32_8x G1 = Grad8x(jFashHash(Seed, perlinX->channel1, perlinY->channel0, perlinZ->channel0), perlinX->f1, perlinY->f0, perlinZ->f0);
    f32_8x G2 = Grad8x(jFashHash(Seed, perlinX->channel0, perlinY->channel1, perlinZ->channel0), perlinX->f0, perlinY->f1, perlinZ->f0);
    f32_8x G3 = Grad8x(jFashHash(Seed, perlinX->channel1, perlinY->channel1, perlinZ->channel0), perlinX->f1, perlinY->f1, perlinZ->f0);

    f32_8x G4 = Grad8x(jFashHash(Seed, perlinX->channel0, perlinY->channel0, perlinZ->channel1), perlinX->f0, perlinY->f0, perlinZ->f1);
    f32_8x G5 = Grad8x(jFashHash(Seed, perlinX->channel1, perlinY->channel0, perlinZ->channel1), perlinX->f1, perlinY->f0, perlinZ->f1);
    f32_8x G6 = Grad8x(jFashHash(Seed, perlinX->channel0, perlinY->channel1, perlinZ->channel1), perlinX->f0, perlinY->f1, perlinZ->f1);
    f32_8x G7 = Grad8x(jFashHash(Seed, perlinX->channel1, perlinY->channel1, perlinZ->channel1), perlinX->f1, perlinY->f1, perlinZ->f1);
#endif

    auto L0  = Lerp8x(perlinX->Fade, G0, G1);
    auto L1  = Lerp8x(perlinX->Fade, G2, G3);
    auto L2  = Lerp8x(perlinX->Fade, G4, G5);
    auto L3  = Lerp8x(perlinX->Fade, G6, G7);

    auto L4  = Lerp8x(perlinY->Fade, L0, L1);
    auto L5  = Lerp8x(perlinY->Fade, L2, L3);

    auto Res = Lerp8x(perlinZ->Fade, L4, L5) * F32_8X(Amplitude);
    /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
    /* Res = ((Res + F32_8X(1.f)) / F32_8X(2.f)) * F32_8X(Amplitude); */

    f32_8x Current = {{ _mm256_load_ps(Dest+Index) }};
    f32_8x Total = Res + Current;
    _mm256_store_ps(Dest+Index, Total.Sse);

    ++perlinX;
  }
}

link_internal void
PerlinNoise_8x_avx2(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  auto PrimeX = U32_8X(501125321);

/* #pragma unroll(2) */
    u32 Index = 0;
/*   for (u32 Index = 0; Index < 16; Index += 8) */
  {
    // Fire off the load for the current values so it has time to cook
    f32_8x Current = {{ _mm256_load_ps(Dest+Index) }};

#if 0
    f32_8x G0 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x0+y0+z0), xf0, yf0, zf0);
    f32_8x G1 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x1+y0+z0), xf1, yf0, zf0);
    f32_8x G2 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x0+y1+z0), xf0, yf1, zf0);
    f32_8x G3 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x1+y1+z0), xf1, yf1, zf0);

    f32_8x G4 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x0+y0+z1), xf0, yf0, zf1);
    f32_8x G5 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x1+y0+z1), xf1, yf0, zf1);
    f32_8x G6 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x0+y1+z1), xf0, yf1, zf1);
    f32_8x G7 = Grad8x(ChrisWellonsIntegerHash_lowbias32(x1+y1+z1), xf1, yf1, zf1);

#else
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel0, perlinZ->channel0), perlinX->f0, perlinY->f0, perlinZ->f0);
    f32_8x G1 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel0, perlinZ->channel0), perlinX->f1, perlinY->f0, perlinZ->f0);
    f32_8x G2 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel1, perlinZ->channel0), perlinX->f0, perlinY->f1, perlinZ->f0);
    f32_8x G3 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel1, perlinZ->channel0), perlinX->f1, perlinY->f1, perlinZ->f0);

    f32_8x G4 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel0, perlinZ->channel1), perlinX->f0, perlinY->f0, perlinZ->f1);
    f32_8x G5 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel0, perlinZ->channel1), perlinX->f1, perlinY->f0, perlinZ->f1);
    f32_8x G6 = Grad8x(HashPrimes(Seed, perlinX->channel0, perlinY->channel1, perlinZ->channel1), perlinX->f0, perlinY->f1, perlinZ->f1);
    f32_8x G7 = Grad8x(HashPrimes(Seed, perlinX->channel1, perlinY->channel1, perlinZ->channel1), perlinX->f1, perlinY->f1, perlinZ->f1);
#endif

    auto L0  = Lerp8x(perlinX->Fade, G0, G1);
    auto L1  = Lerp8x(perlinX->Fade, G2, G3);
    auto L2  = Lerp8x(perlinX->Fade, G4, G5);
    auto L3  = Lerp8x(perlinX->Fade, G6, G7);

    auto L4  = Lerp8x(perlinY->Fade, L0, L1);
    auto L5  = Lerp8x(perlinY->Fade, L2, L3);

    auto Res = Lerp8x(perlinZ->Fade, L4, L5);
    /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
    Res = ((Res + F32_8X(1.f)) / F32_8X(2.f)) * F32_8X(Amplitude);

    f32_8x Total = Res + Current;
    _mm256_store_ps(Dest+Index, Total.Sse);

    ++perlinX;
  }
}
