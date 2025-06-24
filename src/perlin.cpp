
#if 0
u32_8x
SM4Hash(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  __m256 hash = _mm256_sm4rnds4_epi32(x.Sse, y.Sse);
         hash = _mm256_sm4rnds4_epi32(z.Sse, hash);

  return {{ hash }};
}
#endif

u32_8x
AESHash(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  // 1.87 cyc/cell
  /* u32_8x hash = U32_8X(_mm256_aesenc_epi128(x.Sse, y.Sse)) ^ z; */

  // 2.01 cyc/cell
  u32_8x hash = U32_8X(_mm256_aesenc_epi128(x.Sse, y.Sse));
         hash = U32_8X(_mm256_aesenc_epi128(z.Sse, hash.Sse));

  return hash;
}

u32_8x
FNVHash(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  u32_8x hash = U32_8X(0x811c9dc5);

  hash = (hash ^ x) * U32_8X(0x01000193);
  hash = (hash ^ y) * U32_8X(0x01000193);
  hash = (hash ^ z) * U32_8X(0x01000193);

  return hash;
}

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
PerlinNoise_16x_avx2_x(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (u32 Index = 0; Index < 16; Index += 8)
  {

#if 0
    // 7.6 cyc/cell
    f32_8x G0 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif


#if 0
    // 3.2 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 0
    // Unfortunately this produces artifacts; for it to work properly we need
    // the SIMD lanes to be packed with XYZ of a single point .. I think..
    // which would pretty much negate the benefit of using AVX in the first place
    //
    // 1.87 cyc/cell
    // or
    // 2.01 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 0
    // my chip doesn't support SM4 :(
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 0
    // 2.17 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 1
    // 6.2 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif


#if 1
    auto L0  = Lerp8x(perlinX->Fade, G0, G1);
    auto L1  = Lerp8x(perlinX->Fade, G2, G3);
    auto L2  = Lerp8x(perlinX->Fade, G4, G5);
    auto L3  = Lerp8x(perlinX->Fade, G6, G7);

    auto L4  = Lerp8x(perlinY->Fade, L0, L1);
    auto L5  = Lerp8x(perlinY->Fade, L2, L3);

    auto Res = Lerp8x(perlinZ->Fade, L4, L5) * F32_8X(Amplitude);

#else
    /* Vxyz =
     * V000 (1 - x) (1 - y) (1 - z) + */
    /* V100      x  (1 - y) (1 - z) + */
    /* V010 (1 - x)      y  (1 - z) + */
    /* V001 (1 - x) (1 - y)      z  + */
    /* V101      x  (1 - y)      z  + */
    /* V011 (1 - x)      y       z  + */
    /* V110      x       y  (1 - z) + */
    /* V111      x       y       z */

    auto Res = G0*G2*G4 +
               G1*G2*G4 +
               G0*G3*G4 +
               G0*G2*G5 +
               G1*G2*G5 +
               G0*G3*G5 +
               G1*G3*G4 +
               G1*G3*G5;

#endif

    /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
    /* Res = ((Res + F32_8X(1.f)) / F32_8X(2.f)) * F32_8X(Amplitude); */

    f32_8x Current = {{ _mm256_load_ps(Dest+Index) }};
    f32_8x Total = Res + Current;
    _mm256_store_ps(Dest+Index, Total.Sse);

    ++perlinX;
  }
}
link_internal void
PerlinNoise_16x_avx2_y(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  auto PrimeX = U32_8X(501125321);

#pragma unroll(2)
  for (u32 Index = 0; Index < 16; Index += 8)
  {

#if 0
    // 7.6 cyc/cell
    f32_8x G0 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x(ChrisWellonsIntegerHash_lowbias32(perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif


#if 0
    // 3.2 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(FNVHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(FNVHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 0
    // Unfortunately this produces artifacts; for it to work properly we need
    // the SIMD lanes to be packed with XYZ of a single point .. I think..
    // which would pretty much negate the benefit of using AVX in the first place
    //
    // 1.87 cyc/cell
    // or
    // 2.01 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(AESHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(AESHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 0
    // my chip doesn't support SM4 :(
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(SM4Hash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(SM4Hash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 0
    // 2.17 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x_fast(jFashHash(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x_fast(jFashHash(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif

#if 1
    // 6.2 cyc/cell
    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G1 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
    f32_8x G2 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
    f32_8x G3 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

    f32_8x G4 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G5 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
    f32_8x G6 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
    f32_8x G7 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);
#endif


#if 1
    auto L0  = Lerp8x(perlinX->Fade, G0, G1);
    auto L1  = Lerp8x(perlinX->Fade, G2, G3);
    auto L2  = Lerp8x(perlinX->Fade, G4, G5);
    auto L3  = Lerp8x(perlinX->Fade, G6, G7);

    auto L4  = Lerp8x(perlinY->Fade, L0, L1);
    auto L5  = Lerp8x(perlinY->Fade, L2, L3);

    auto Res = Lerp8x(perlinZ->Fade, L4, L5) * F32_8X(Amplitude);

#else
    /* Vxyz =
     * V000 (1 - x) (1 - y) (1 - z) + */
    /* V100      x  (1 - y) (1 - z) + */
    /* V010 (1 - x)      y  (1 - z) + */
    /* V001 (1 - x) (1 - y)      z  + */
    /* V101      x  (1 - y)      z  + */
    /* V011 (1 - x)      y       z  + */
    /* V110      x       y  (1 - z) + */
    /* V111      x       y       z */

    auto Res = G0*G2*G4 +
               G1*G2*G4 +
               G0*G3*G4 +
               G0*G2*G5 +
               G1*G2*G5 +
               G0*G3*G5 +
               G1*G3*G4 +
               G1*G3*G5;

#endif

    /* Res = Res * F32_8X( 0.964921414852142333984375f ); */
    /* Res = ((Res + F32_8X(1.f)) / F32_8X(2.f)) * F32_8X(Amplitude); */

    f32_8x Current = {{ _mm256_load_ps(Dest+Index) }};
    f32_8x Total = Res + Current;
    _mm256_store_ps(Dest+Index, Total.Sse);

    ++perlinY;
  }
}

link_internal void
PerlinNoise_8x_avx2_pannoniae(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  u32_8x Seed = U32_8X(1066037191);

  f32_8x G0 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
  f32_8x G1 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
  f32_8x G2 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
  f32_8x G3 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

  f32_8x G4 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
  f32_8x G5 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
  f32_8x G6 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
  f32_8x G7 = Grad8x_pannoniae(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);

  auto L0  = Lerp8x(perlinX->Fade, G0, G1);
  auto L1  = Lerp8x(perlinX->Fade, G2, G3);
  auto L2  = Lerp8x(perlinX->Fade, G4, G5);
  auto L3  = Lerp8x(perlinX->Fade, G6, G7);

  auto L4  = Lerp8x(perlinY->Fade, L0, L1);
  auto L5  = Lerp8x(perlinY->Fade, L2, L3);

  auto Res = Lerp8x(perlinZ->Fade, L4, L5) * F32_8X(Amplitude);

  f32_8x Current = {{ _mm256_load_ps(Dest) }};
  f32_8x Total = Res + Current;
  _mm256_store_ps(Dest, Total.Sse);
}

link_internal void
PerlinNoise_8x_avx2(perlin_params *perlinX, perlin_params *perlinY, perlin_params *perlinZ, f32 *Dest, f32 Amplitude)
{
  u32_8x Seed = U32_8X(1066037191);

  f32_8x G0 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P0), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract0);
  f32_8x G1 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P0), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract0);
  f32_8x G2 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P0), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract0);
  f32_8x G3 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P0), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract0);

  f32_8x G4 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P0, perlinZ->P1), perlinX->Fract0, perlinY->Fract0, perlinZ->Fract1);
  f32_8x G5 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P0, perlinZ->P1), perlinX->Fract1, perlinY->Fract0, perlinZ->Fract1);
  f32_8x G6 = Grad8x(HashPrimes(Seed, perlinX->P0, perlinY->P1, perlinZ->P1), perlinX->Fract0, perlinY->Fract1, perlinZ->Fract1);
  f32_8x G7 = Grad8x(HashPrimes(Seed, perlinX->P1, perlinY->P1, perlinZ->P1), perlinX->Fract1, perlinY->Fract1, perlinZ->Fract1);

  auto L0  = Lerp8x(perlinX->Fade, G0, G1);
  auto L1  = Lerp8x(perlinX->Fade, G2, G3);
  auto L2  = Lerp8x(perlinX->Fade, G4, G5);
  auto L3  = Lerp8x(perlinX->Fade, G6, G7);

  auto L4  = Lerp8x(perlinY->Fade, L0, L1);
  auto L5  = Lerp8x(perlinY->Fade, L2, L3);

  auto Res = Lerp8x(perlinZ->Fade, L4, L5) * F32_8X(Amplitude);

  f32_8x Current = {{ _mm256_load_ps(Dest) }};
  f32_8x Total = Res + Current;
  _mm256_store_ps(Dest, Total.Sse);
}

link_internal void
Terrain_FBM3D( f32 *NoiseValues,
                v3  Period,
               v3i  NoiseDim,
               v3i  NoiseBasis,
               s32  OctaveCount )
{
  Period = Max(Period, V3(1.f));

  auto PrimeX = U32_8X(501125321);
  auto PrimeY = 1136930381u;
  auto PrimeZ = 1720413743u;

  memory_arena *TempArena = GetTranArena();


  s32  NoiseValuesCount  = Volume(NoiseDim);

  //
  // NOTE(Jesse): Must be true to use _mm256_store_ps
  Assert(u64(NoiseValues) % 32 == 0);

  v3i WorldBasis = NoiseBasis;
  {
    u32_8x xChunkResolution = U32_8X(1);
       u32 yChunkResolution = u32(1);
       u32 zChunkResolution = u32(1);

            u32 *zPeriods = Allocate(u32, TempArena, OctaveCount);
            u32 *yPeriods = Allocate(u32, TempArena, OctaveCount);
    avx_divisor *xPeriods = AllocateAligned(avx_divisor, TempArena, OctaveCount, 32);

    perlin_params *xParams = AllocateAligned(perlin_params, TempArena, NoiseDim.x*OctaveCount*2, 32);
    perlin_params *yParams = AllocateAligned(perlin_params, TempArena, NoiseDim.y*OctaveCount*2, 32);
    perlin_params *zParams = AllocateAligned(perlin_params, TempArena, NoiseDim.z*OctaveCount*2, 32);

    v3i InteriorPeriod = V3i(Period);
    RangeIterator(OctaveIndex, OctaveCount)
    {
      zPeriods[OctaveIndex] = u32(InteriorPeriod.z);
      yPeriods[OctaveIndex] = u32(InteriorPeriod.y);
      xPeriods[OctaveIndex] = AvxDivisor(u32(InteriorPeriod.x));
      InteriorPeriod = Max(V3i(1), InteriorPeriod/2);
    }

    u64 StartingCycles = __rdtsc();
    {
      TIMED_NAMED_BLOCK(precalculate_parameters);

      for ( s32 zNoise = 0; zNoise < NoiseDim.z; ++ zNoise)
      {
        RangeIterator(OctaveIndex, OctaveCount)
        {
          s32 i = OctaveIndex+(zNoise*OctaveCount);
          zParams[i] = ComputePerlinParameters_scalar(u32(WorldBasis.z), u32(zNoise), zChunkResolution, zPeriods[OctaveIndex], PrimeZ);
        }
      }

      for ( s32 yNoise = 0; yNoise < NoiseDim.y; ++ yNoise)
      {
        RangeIterator(OctaveIndex, OctaveCount)
        {
          s32 i = OctaveIndex+(yNoise*OctaveCount);
          yParams[i] = ComputePerlinParameters_scalar(u32(WorldBasis.y), u32(yNoise), yChunkResolution, yPeriods[OctaveIndex], PrimeY);
        }
      }

      u32 *_xCoords = AllocateAligned(u32, TempArena, NoiseDim.x, 32);
      for ( s32 xNoise = 0; xNoise < NoiseDim.x; ++ xNoise )
      {
        _xCoords[xNoise] = u32(xNoise-1);
      }

      for ( s32 xNoise = 0; xNoise < NoiseDim.x; xNoise += 8 )
      {
        auto _x = U32_8X(_xCoords+xNoise);
        RangeIterator(OctaveIndex, OctaveCount)
        {
          s32 i = OctaveIndex+(xNoise*OctaveCount);
          xParams[i] = ComputePerlinParameters_vector(U32_8X(WorldBasis.x), _x, xChunkResolution, xPeriods[OctaveIndex], PrimeX);
        }
      }
    }

    f32 OctaveAmplitudeMax = {};

    f32 Amplitude = 1.f;
    {
      r32 InteriorAmp = r32(Amplitude);
      RangeIterator(OctaveIndex, OctaveCount)
      {
        OctaveAmplitudeMax += InteriorAmp;
        InteriorAmp = Max(1.f, InteriorAmp/2.f);
      }
    }

    {
      TIMED_NAMED_BLOCK(OctaveCount);
      for ( s32 zNoise = 0; zNoise < NoiseDim.z; ++ zNoise)
      {
        for ( s32 yNoise = 0; yNoise < NoiseDim.y; ++ yNoise)
        {
          for ( s32 xNoise = 0; xNoise < NoiseDim.x; xNoise += 8 )
          {
            s32 NoiseIndex = GetIndex(xNoise, yNoise, zNoise, NoiseDim);

            r32 InteriorAmp = r32(Amplitude);
            RangeIterator(OctaveIndex, OctaveCount)
            {
              auto zParam = zParams+OctaveIndex+(zNoise*OctaveCount);
              auto yParam = yParams+OctaveIndex+(yNoise*OctaveCount);
              auto xParam = xParams+OctaveIndex+(xNoise*OctaveCount);
              PerlinNoise_8x_avx2_pannoniae(xParam, yParam, zParam, NoiseValues+NoiseIndex, InteriorAmp);
              InteriorAmp = Max(1.f, InteriorAmp/2.f);
            }
          }
        }
      }
    }
  }

}

struct perlin_inputs
{
  perlin_params *xParams;
  perlin_params *yParams;
  perlin_params *zParams;
  u32 *_xCoords;
};

link_internal perlin_inputs
AllocatePerlinParams(v3i NoiseDim, memory_arena *Arena)
{
  perlin_params *xParams = AllocateAligned(perlin_params, Arena, NoiseDim.x*2, 32);
  perlin_params *yParams = AllocateAligned(perlin_params, Arena, NoiseDim.y*2, 32);
  perlin_params *zParams = AllocateAligned(perlin_params, Arena, NoiseDim.z*2, 32);
  u32 *_xCoords = AllocateAligned(u32, Arena, NoiseDim.x, 32);

  perlin_inputs Result = {xParams, yParams, zParams, _xCoords};
  return Result;
}

link_internal void
PerlinNoise(   f32 *NoiseValues,
                v3  Period,
               v3i  NoiseDim,
               v3i  NoiseBasis,
               s32  OctaveCount,
               perlin_inputs *Inputs)
{
  // NOTE(Jesse): Must be true to use _mm256_store_ps
  Assert(u64(NoiseValues) % 32 == 0);

  // NOTE(Jesse): We're doing 8-wide, so we need this to be true.
  Assert(NoiseDim.x % 8 == 0);

  auto PrimeX = U32_8X(501125321);
  auto PrimeY = 1136930381u;
  auto PrimeZ = 1720413743u;

  v3i WorldBasis = NoiseBasis;
  {
    u32_8x xChunkResolution = U32_8X(1);
       u32 yChunkResolution = u32(1);
       u32 zChunkResolution = u32(1);

    perlin_params *xParams = Inputs->xParams;
    perlin_params *yParams = Inputs->yParams;
    perlin_params *zParams = Inputs->zParams;
    auto _xCoords = Inputs->_xCoords;

    u32 zPeriods = u32(Period.z);
    u32 yPeriods = u32(Period.y);
    avx_divisor xPeriods = AvxDivisor(u32(Period.x));

    {
      for ( s32 zNoise = 0; zNoise < NoiseDim.z; ++ zNoise)
      {
        s32 i = zNoise;
        zParams[i] = ComputePerlinParameters_scalar(u32(WorldBasis.z), u32(zNoise), zChunkResolution, zPeriods, PrimeZ);
      }

      for ( s32 yNoise = 0; yNoise < NoiseDim.y; ++ yNoise)
      {
        s32 i = yNoise;
        yParams[i] = ComputePerlinParameters_scalar(u32(WorldBasis.y), u32(yNoise), yChunkResolution, yPeriods, PrimeY);
      }

      for ( s32 xNoise = 0; xNoise < NoiseDim.x; ++ xNoise )
      {
        _xCoords[xNoise] = u32(xNoise-1);
      }

      for ( s32 xNoise = 0; xNoise < NoiseDim.x; xNoise += 8 )
      {
        auto _x = U32_8X(_xCoords+xNoise);
        s32 i = xNoise;
        xParams[i] = ComputePerlinParameters_vector(U32_8X(WorldBasis.x), _x, xChunkResolution, xPeriods, PrimeX);
      }
    }

    f32 Amplitude = 1.f;

    {
      for ( s32 zNoise = 0; zNoise < NoiseDim.z; ++ zNoise)
      {
        for ( s32 yNoise = 0; yNoise < NoiseDim.y; ++ yNoise)
        {
          for ( s32 xNoise = 0; xNoise < NoiseDim.x; xNoise += 8 )
          {
            s32 NoiseIndex = GetIndex(xNoise, yNoise, zNoise, NoiseDim);

            auto zParam = zParams+zNoise;
            auto yParam = yParams+yNoise;
            auto xParam = xParams+xNoise;
            PerlinNoise_8x_avx2_pannoniae(xParam, yParam, zParam, NoiseValues+NoiseIndex, Amplitude);
          }
        }
      }
    }
  }

}
