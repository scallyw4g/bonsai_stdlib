
u32_8x
HashPrimes(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  u32_8x hash = Seed;

#if 0
  hash = hash ^ x ^ y ^ z;
#else
  hash = hash ^ x;
  hash = hash ^ y;
  hash = hash ^ z;
  hash = hash * U32_8X( 0x27d4eb2d );
#endif
  return (hash >> 15) ^ hash;
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

#if 0
link_internal void
PerlinNoise_8x_sse(f32 *_x, f32 yIn, f32 zIn, f32 *Result)
{
  f32 _y = yIn;
  f32 _z = zIn;

  if (_y < 0) _y -= 1.f;
  if (_z < 0) _z -= 1.f;

  u32 Yi = u32(_y) & 255;
  u32 Zi = u32(_z) & 255;

  _y -= Floorf(_y);
  _z -= Floorf(_z);

  f32 v = fade(_y);
  f32 w = fade(_z);

  f32_4x v4 = F32_4X(v);
  f32_4x w4 = F32_4X(w);

  u32 _Xi[8] =
  {
    _x[0] < 0 ? u32(_x[0]-1.f)&255 : u32(_x[0])&255,
    _x[1] < 0 ? u32(_x[1]-1.f)&255 : u32(_x[1])&255,
    _x[2] < 0 ? u32(_x[2]-1.f)&255 : u32(_x[2])&255,
    _x[3] < 0 ? u32(_x[3]-1.f)&255 : u32(_x[3])&255,
    _x[4] < 0 ? u32(_x[4]-1.f)&255 : u32(_x[4])&255,
    _x[5] < 0 ? u32(_x[5]-1.f)&255 : u32(_x[5])&255,
    _x[6] < 0 ? u32(_x[6]-1.f)&255 : u32(_x[6])&255,
    _x[7] < 0 ? u32(_x[7]-1.f)&255 : u32(_x[7])&255,
  };

  f32 _xF[8] =
  {
    _x[0]-Floorf(_x[0]),
    _x[1]-Floorf(_x[1]),
    _x[2]-Floorf(_x[2]),
    _x[3]-Floorf(_x[3]),
    _x[4]-Floorf(_x[4]),
    _x[5]-Floorf(_x[5]),
    _x[6]-Floorf(_x[6]),
    _x[7]-Floorf(_x[7]),
  };

  f32_4x _u4[2] =
  {
    Fade4x(F32_4X(_xF[0], _xF[1], _xF[2], _xF[3])),
    Fade4x(F32_4X(_xF[4], _xF[5], _xF[6], _xF[7])),
  };



  for (u32 Index = 0; Index < 8; Index += 4)
  {
    SETUP_HASH_DATA_FOR_SIMD_LANE(0);
    SETUP_HASH_DATA_FOR_SIMD_LANE(1);
    SETUP_HASH_DATA_FOR_SIMD_LANE(2);
    SETUP_HASH_DATA_FOR_SIMD_LANE(3);

    u32_4x H0 = U32_4X(H0_0, H0_1, H0_2, H0_3);
    u32_4x H1 = U32_4X(H1_0, H1_1, H1_2, H1_3);
    u32_4x H2 = U32_4X(H2_0, H2_1, H2_2, H2_3);
    u32_4x H3 = U32_4X(H3_0, H3_1, H3_2, H3_3);

    u32_4x H4 = U32_4X(H4_0, H4_1, H4_2, H4_3);
    u32_4x H5 = U32_4X(H5_0, H5_1, H5_2, H5_3);
    u32_4x H6 = U32_4X(H6_0, H6_1, H6_2, H6_3);
    u32_4x H7 = U32_4X(H7_0, H7_1, H7_2, H7_3);



    f32_4x u4 = _u4[Index/4];

    f32_4x  x = F32_4X(x_0, x_1, x_2, x_3);
    f32_4x nx = F32_4X(x_0-1, x_1-1, x_2-1, x_3-1);

    f32_4x  y = F32_4X(_y);
    f32_4x ny = F32_4X(_y-1);

    f32_4x  z = F32_4X(_z);
    f32_4x nz = F32_4X(_z-1);
    

    f32_4x G0 = Grad4x(H0,  x,  y, z);
    f32_4x G1 = Grad4x(H1, nx,  y, z);
    f32_4x G2 = Grad4x(H2,  x, ny, z);
    f32_4x G3 = Grad4x(H3, nx, ny, z);

    f32_4x G4 = Grad4x(H4,  x,  y, nz);
    f32_4x G5 = Grad4x(H5, nx,  y, nz);
    f32_4x G6 = Grad4x(H6,  x, ny, nz);
    f32_4x G7 = Grad4x(H7, nx, ny, nz);

    auto L0  = Lerp4x(u4, G0, G1);
    auto L1  = Lerp4x(u4, G2, G3);
    auto L2  = Lerp4x(u4, G4, G5);
    auto L3  = Lerp4x(u4, G6, G7);
    auto L4  = Lerp4x(v4, L0, L1);
    auto L5  = Lerp4x(v4, L2, L3);

    auto Res = Lerp4x(w4, L4, L5 );
    Res = (Res + F32_4X(1.f)) / F32_4X(2.f);

    Result[Index+0] = Res.E[0];
    Result[Index+1] = Res.E[1];
    Result[Index+2] = Res.E[2];
    Result[Index+3] = Res.E[3];
  }
}
#endif

#if 0
link_internal void
PerlinNoise_8x_avx2(f32 *xIn, f32 yIn, f32 zIn, f32 *Result)
{
  auto PrimeX = U32_8X(501125321);
  auto PrimeY = U32_8X(1136930381);
  auto PrimeZ = U32_8X(1720413743);
  auto PrimeW = U32_8X(1066037191);

  auto x = F32_8X( xIn[0], xIn[1], xIn[2], xIn[3], xIn[4], xIn[5], xIn[6], xIn[7] );
  /* auto x = F32_8X( xIn[0] ); */
  auto y = F32_8X(yIn);
  auto z = F32_8X(zIn);

  auto xs = Floor(x);
  auto ys = Floor(y);
  auto zs = Floor(z);

  auto x0 = U32_8X(xs) * PrimeX;
  auto y0 = U32_8X(ys) * PrimeY;
  auto z0 = U32_8X(zs) * PrimeZ;

  auto x1 = x0 + PrimeX;
  auto y1 = y0 + PrimeY;
  auto z1 = z0 + PrimeZ;

  auto xf0 = xs = x - xs;
  auto yf0 = ys = y - ys;
  auto zf0 = zs = z - zs;

  auto xf1 = xf0 - F32_8X(1.f);
  auto yf1 = yf0 - F32_8X(1.f);
  auto zf1 = zf0 - F32_8X(1.f);

  xs = Fade8x(xs);
  ys = Fade8x(ys);
  zs = Fade8x(zs);

  u32 Index = 0;
  /* for (u32 Index = 0; Index < 8; Index += 4) */
  {

    u32_8x Seed = U32_8X(1066037191);

    f32_8x G0 = Grad8x(HashPrimes(Seed, x0, y0, z0), xf0, yf0, zf0);
    f32_8x G1 = Grad8x(HashPrimes(Seed, x1, y0, z0), xf1, yf0, zf0);
    f32_8x G2 = Grad8x(HashPrimes(Seed, x0, y1, z0), xf0, yf1, zf0);
    f32_8x G3 = Grad8x(HashPrimes(Seed, x1, y1, z0), xf1, yf1, zf0);

    f32_8x G4 = Grad8x(HashPrimes(Seed, x0, y0, z1), xf0, yf0, zf1);
    f32_8x G5 = Grad8x(HashPrimes(Seed, x1, y0, z1), xf1, yf0, zf1);
    f32_8x G6 = Grad8x(HashPrimes(Seed, x0, y1, z1), xf0, yf1, zf1);
    f32_8x G7 = Grad8x(HashPrimes(Seed, x1, y1, z1), xf1, yf1, zf1);

    auto L0  = Lerp8x(xs, G0, G1);
    auto L1  = Lerp8x(xs, G2, G3);
    auto L2  = Lerp8x(xs, G4, G5);
    auto L3  = Lerp8x(xs, G6, G7);

    auto L4  = Lerp8x(ys, L0, L1);
    auto L5  = Lerp8x(ys, L2, L3);

    auto Res = Lerp8x(zs, L4, L5);
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
#endif

link_internal void
PerlinNoise_16x_avx2(perlin_params *perlinX,perlin_params *perlinY, perlin_params *perlinZ, f32 *Result)
{
  auto PrimeX = U32_8X(501125321);

  for (u32 Index = 0; Index < 16; Index += 8)
  {


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
