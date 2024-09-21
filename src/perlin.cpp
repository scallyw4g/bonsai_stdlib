
u32_8x
jFashHash(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  // Bad visual artifacts
  /* u32_8x hash = (Seed + x + y + z) >> 15; */

  // Fast, reasonable visual entropy
  u32_8x hash = (x ^ y ^ z) >> 15;

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
ChrisWellonsIntegerHash_lowbias32(u32_8x x)
{
  x = x ^ (x >> 16);
  x = x * U32_8X(0x7feb352d);
  x = x ^ (x >> 15);
  x = x * U32_8X(0x846ca68b);
  x = x ^ (x >> 16);
  return x;
}

link_internal void
PerlinNoise_16x_avx2(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (u32 Index = 0; Index < 16; Index += 8)
  {

#if 0
    f32_8x G0 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0+perlinY->channel0+perlinZ->channel0), perlinX->f0, perlinY->f0, perlinZ->f0);
    f32_8x G1 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1+perlinY->channel0+perlinZ->channel0), perlinX->f1, perlinY->f0, perlinZ->f0);
    f32_8x G2 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0+perlinY->channel1+perlinZ->channel0), perlinX->f0, perlinY->f1, perlinZ->f0);
    f32_8x G3 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1+perlinY->channel1+perlinZ->channel0), perlinX->f1, perlinY->f1, perlinZ->f0);
                                                                                                                                                      
    f32_8x G4 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0+perlinY->channel0+perlinZ->channel1), perlinX->f0, perlinY->f0, perlinZ->f1);
    f32_8x G5 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1+perlinY->channel0+perlinZ->channel1), perlinX->f1, perlinY->f0, perlinZ->f1);
    f32_8x G6 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel0+perlinY->channel1+perlinZ->channel1), perlinX->f0, perlinY->f1, perlinZ->f1);
    f32_8x G7 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->channel1+perlinY->channel1+perlinZ->channel1), perlinX->f1, perlinY->f1, perlinZ->f1);
#endif

#if 0
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


#if 1
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

    auto Res = Lerp8x(perlinZ->Fade, L4, L5);
    /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
    Res = ((Res + F32_8X(1.f)) / F32_8X(2.f)) * F32_8X(Amplitude);

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


link_inline void
PerlinNoise_8x_avx2(f32 *xIn, f32 yIn, f32 zIn, f32 *Result)
{
  u32 Index = 0;

  auto Primex = U32_8X(501125321);
  auto Primey = U32_8X(1136930381);
  auto Primez = U32_8X(1720413743);

  auto x = F32_8X( xIn[Index+0], xIn[Index+1], xIn[Index+2], xIn[Index+3], xIn[Index+4], xIn[Index+5], xIn[Index+6], xIn[Index+7] );
  auto y = F32_8X(yIn);
  auto z = F32_8X(zIn);

  auto perlinX = ComputePerlinParameters(x, Primex);
  auto perlinY = ComputePerlinParameters(y, Primey);
  auto perlinZ = ComputePerlinParameters(z, Primez);

  /* for (u32 Index = 0; Index < 8; Index += 8) */
  {

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

    f32_8x G0 = Grad8x(HashPrimes(Seed, perlinX.channel0, perlinY.channel0, perlinZ.channel0), perlinX.f0, perlinY.f0, perlinZ.f0);
    f32_8x G1 = Grad8x(HashPrimes(Seed, perlinX.channel1, perlinY.channel0, perlinZ.channel0), perlinX.f1, perlinY.f0, perlinZ.f0);
    f32_8x G2 = Grad8x(HashPrimes(Seed, perlinX.channel0, perlinY.channel1, perlinZ.channel0), perlinX.f0, perlinY.f1, perlinZ.f0);
    f32_8x G3 = Grad8x(HashPrimes(Seed, perlinX.channel1, perlinY.channel1, perlinZ.channel0), perlinX.f1, perlinY.f1, perlinZ.f0);

    f32_8x G4 = Grad8x(HashPrimes(Seed, perlinX.channel0, perlinY.channel0, perlinZ.channel1), perlinX.f0, perlinY.f0, perlinZ.f1);
    f32_8x G5 = Grad8x(HashPrimes(Seed, perlinX.channel1, perlinY.channel0, perlinZ.channel1), perlinX.f1, perlinY.f0, perlinZ.f1);
    f32_8x G6 = Grad8x(HashPrimes(Seed, perlinX.channel0, perlinY.channel1, perlinZ.channel1), perlinX.f0, perlinY.f1, perlinZ.f1);
    f32_8x G7 = Grad8x(HashPrimes(Seed, perlinX.channel1, perlinY.channel1, perlinZ.channel1), perlinX.f1, perlinY.f1, perlinZ.f1);

#endif
    auto L0  = Lerp8x(perlinX.Fade, G0, G1);
    auto L1  = Lerp8x(perlinX.Fade, G2, G3);
    auto L2  = Lerp8x(perlinX.Fade, G4, G5);
    auto L3  = Lerp8x(perlinX.Fade, G6, G7);

    auto L4  = Lerp8x(perlinY.Fade, L0, L1);
    auto L5  = Lerp8x(perlinY.Fade, L2, L3);

    auto Res = Lerp8x(perlinZ.Fade, L4, L5);
    /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
    Res = (Res + F32_8X(1.f)) / F32_8X(2.f);

    Result[Index+0] = Res.E[0];
    Result[Index+1] = Res.E[1];
    Result[Index+2] = Res.E[2];
    Result[Index+3] = Res.E[3];
    Result[Index+4] = Res.E[4];
    Result[Index+5] = Res.E[5];
    Result[Index+6] = Res.E[6];
    Result[Index+7] = Res.E[7];
  }
}

link_internal void
PerlinNoise_16x2x2_avx2(perlin_params *xParams_in, perlin_params *yParams_in, perlin_params *zParams_in, f32 *Result)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (s32 zIndex = 0; zIndex <  2; zIndex += 1)
  {
    perlin_params *zParams = zParams_in+zIndex;
#pragma unroll(2)
    for (s32 yIndex = 0; yIndex <  2; yIndex += 1)
    {
      perlin_params *yParams = yParams_in+yIndex;
#pragma unroll(2)
      for (s32 xIndex = 0; xIndex < 2; xIndex += 1)
      {
        perlin_params *xParams = xParams_in+xIndex;
        u32_8x Seed = U32_8X(1066037191);

        f32_8x G0 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel0, zParams->channel0), xParams->f0, yParams->f0, zParams->f0);
        f32_8x G1 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel0, zParams->channel0), xParams->f1, yParams->f0, zParams->f0);
        f32_8x G2 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel1, zParams->channel0), xParams->f0, yParams->f1, zParams->f0);
        f32_8x G3 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel1, zParams->channel0), xParams->f1, yParams->f1, zParams->f0);

        f32_8x G4 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel0, zParams->channel1), xParams->f0, yParams->f0, zParams->f1);
        f32_8x G5 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel0, zParams->channel1), xParams->f1, yParams->f0, zParams->f1);
        f32_8x G6 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel1, zParams->channel1), xParams->f0, yParams->f1, zParams->f1);
        f32_8x G7 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel1, zParams->channel1), xParams->f1, yParams->f1, zParams->f1);

        auto L0  = Lerp8x(xParams->Fade, G0, G1);
        auto L1  = Lerp8x(xParams->Fade, G2, G3);
        auto L2  = Lerp8x(xParams->Fade, G4, G5);
        auto L3  = Lerp8x(xParams->Fade, G6, G7);

        auto L4  = Lerp8x(yParams->Fade, L0, L1);
        auto L5  = Lerp8x(yParams->Fade, L2, L3);

        auto Res = Lerp8x(zParams->Fade, L4, L5);
        /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
        Res = (Res + F32_8X(1.f)) / F32_8X(2.f);

        s32 destIndex = GetIndex(xIndex*8, yIndex, zIndex, V3i(16,2,2));
        Result[destIndex+0] = Res.E[0];
        Result[destIndex+1] = Res.E[1];
        Result[destIndex+2] = Res.E[2];
        Result[destIndex+3] = Res.E[3];
        Result[destIndex+4] = Res.E[4];
        Result[destIndex+5] = Res.E[5];
        Result[destIndex+6] = Res.E[6];
        Result[destIndex+7] = Res.E[7];
      }
    }
  }
}

link_internal void
PerlinNoise_16x2x1_avx2(perlin_params *xParams_in, perlin_params *yParams_in, perlin_params *zParams_in, f32 *Result)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (s32 zIndex = 0; zIndex < 1; zIndex += 1)
  {
    perlin_params *zParams = zParams_in+zIndex;
#pragma unroll(2)
    for (s32 yIndex = 0; yIndex <  2; yIndex += 1)
    {
      perlin_params *yParams = yParams_in+yIndex;
#pragma unroll(2)
      for (s32 xIndex = 0; xIndex < 2; xIndex += 1)
      {
        perlin_params *xParams = xParams_in+xIndex;
        u32_8x Seed = U32_8X(1066037191);

        f32_8x G0 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel0, zParams->channel0), xParams->f0, yParams->f0, zParams->f0);
        f32_8x G1 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel0, zParams->channel0), xParams->f1, yParams->f0, zParams->f0);
        f32_8x G2 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel1, zParams->channel0), xParams->f0, yParams->f1, zParams->f0);
        f32_8x G3 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel1, zParams->channel0), xParams->f1, yParams->f1, zParams->f0);

        f32_8x G4 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel0, zParams->channel1), xParams->f0, yParams->f0, zParams->f1);
        f32_8x G5 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel0, zParams->channel1), xParams->f1, yParams->f0, zParams->f1);
        f32_8x G6 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel1, zParams->channel1), xParams->f0, yParams->f1, zParams->f1);
        f32_8x G7 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel1, zParams->channel1), xParams->f1, yParams->f1, zParams->f1);

        auto L0  = Lerp8x(xParams->Fade, G0, G1);
        auto L1  = Lerp8x(xParams->Fade, G2, G3);
        auto L2  = Lerp8x(xParams->Fade, G4, G5);
        auto L3  = Lerp8x(xParams->Fade, G6, G7);

        auto L4  = Lerp8x(yParams->Fade, L0, L1);
        auto L5  = Lerp8x(yParams->Fade, L2, L3);

        auto Res = Lerp8x(zParams->Fade, L4, L5);
        /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
        Res = (Res + F32_8X(1.f)) / F32_8X(2.f);

        s32 destIndex = GetIndex(xIndex*8, yIndex, zIndex, V3i(16,2,1));
        Result[destIndex+0] = Res.E[0];
        Result[destIndex+1] = Res.E[1];
        Result[destIndex+2] = Res.E[2];
        Result[destIndex+3] = Res.E[3];
        Result[destIndex+4] = Res.E[4];
        Result[destIndex+5] = Res.E[5];
        Result[destIndex+6] = Res.E[6];
        Result[destIndex+7] = Res.E[7];
      }
    }
  }
}


link_internal void
PerlinNoise_16x1x1_avx2(perlin_params *xParams_in, perlin_params *yParams_in, perlin_params *zParams_in, f32 *Result)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (s32 zIndex = 0; zIndex < 1; zIndex += 1)
  {
    perlin_params *zParams = zParams_in+zIndex;
#pragma unroll(2)
    for (s32 yIndex = 0; yIndex < 1; yIndex += 1)
    {
      perlin_params *yParams = yParams_in+yIndex;
#pragma unroll(2)
      for (s32 xIndex = 0; xIndex < 2; xIndex += 1)
      {
        perlin_params *xParams = xParams_in+xIndex;
        u32_8x Seed = U32_8X(1066037191);

        f32_8x G0 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel0, zParams->channel0), xParams->f0, yParams->f0, zParams->f0);
        f32_8x G1 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel0, zParams->channel0), xParams->f1, yParams->f0, zParams->f0);
        f32_8x G2 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel1, zParams->channel0), xParams->f0, yParams->f1, zParams->f0);
        f32_8x G3 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel1, zParams->channel0), xParams->f1, yParams->f1, zParams->f0);

        f32_8x G4 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel0, zParams->channel1), xParams->f0, yParams->f0, zParams->f1);
        f32_8x G5 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel0, zParams->channel1), xParams->f1, yParams->f0, zParams->f1);
        f32_8x G6 = Grad8x(HashPrimes(Seed, xParams->channel0, yParams->channel1, zParams->channel1), xParams->f0, yParams->f1, zParams->f1);
        f32_8x G7 = Grad8x(HashPrimes(Seed, xParams->channel1, yParams->channel1, zParams->channel1), xParams->f1, yParams->f1, zParams->f1);

        auto L0  = Lerp8x(xParams->Fade, G0, G1);
        auto L1  = Lerp8x(xParams->Fade, G2, G3);
        auto L2  = Lerp8x(xParams->Fade, G4, G5);
        auto L3  = Lerp8x(xParams->Fade, G6, G7);

        auto L4  = Lerp8x(yParams->Fade, L0, L1);
        auto L5  = Lerp8x(yParams->Fade, L2, L3);

        auto Res = Lerp8x(zParams->Fade, L4, L5);
        /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
        Res = (Res + F32_8X(1.f)) / F32_8X(2.f);

        s32 destIndex = GetIndex(xIndex*8, yIndex, zIndex, V3i(16,1,1));
        Result[destIndex+0] = Res.E[0];
        Result[destIndex+1] = Res.E[1];
        Result[destIndex+2] = Res.E[2];
        Result[destIndex+3] = Res.E[3];
        Result[destIndex+4] = Res.E[4];
        Result[destIndex+5] = Res.E[5];
        Result[destIndex+6] = Res.E[6];
        Result[destIndex+7] = Res.E[7];
      }
    }
  }
}

link_internal void
PerlinNoise_32x_avx2(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(4)
  for (u32 Index = 0; Index < 32; Index += 8)
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
