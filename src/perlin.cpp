
#if 0
#define SETUP_HASH_DATA_FOR_SIMD_LANE(lane_number) \
    u32 Xi_##lane_number = _Xi[Index+lane_number]; \
    f32  x_##lane_number = _xF[Index+lane_number]; \
    u32  A_##lane_number = Global_PerlinIV[ Xi_##lane_number  ] + Yi;      \
    u32 AA_##lane_number = Global_PerlinIV[  A_##lane_number  ] + Zi;      \
    u32 AB_##lane_number = Global_PerlinIV[  A_##lane_number+1] + Zi;      \
    u32  B_##lane_number = Global_PerlinIV[ Xi_##lane_number+1] + Yi;      \
    u32 BA_##lane_number = Global_PerlinIV[  B_##lane_number  ] + Zi;      \
    u32 BB_##lane_number = Global_PerlinIV[  B_##lane_number+1] + Zi;      \
    \
    u32 H0_##lane_number = Global_PerlinIV[ AA_##lane_number  ];            \
    u32 H1_##lane_number = Global_PerlinIV[ BA_##lane_number  ];            \
    u32 H2_##lane_number = Global_PerlinIV[ AB_##lane_number  ];            \
    u32 H3_##lane_number = Global_PerlinIV[ BB_##lane_number  ];            \
    u32 H4_##lane_number = Global_PerlinIV[ AA_##lane_number+1];            \
    u32 H5_##lane_number = Global_PerlinIV[ BA_##lane_number+1];            \
    u32 H6_##lane_number = Global_PerlinIV[ AB_##lane_number+1];            \
    u32 H7_##lane_number = Global_PerlinIV[ BB_##lane_number+1]
#else
u32_8x
HashPrimes(u32_8x Seed, u32_8x x, u32_8x y, u32_8x z)
{
  u32_8x hash = Seed;

  hash = hash ^ x;
  hash = hash ^ y;
  hash = hash ^ z;

  hash = hash * U32_8X( 0x27d4eb2d );
  return (hash >> 15) ^ hash;
}
#endif

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

link_internal void
PerlinNoise_8x_avx2(f32 *xIn, f32 yIn, f32 zIn, f32 *Result)
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

  f32_8x v4 = F32_8X(v);
  f32_8x w4 = F32_8X(w);

  auto x = F32_8X( xIn[0], xIn[1], xIn[2], xIn[3], xIn[4], xIn[5], xIn[6], xIn[7] );
  auto y = F32_8X(_y);
  auto z = F32_8X(_z);


  auto xf = Floor(x);
  auto yf = F32_8X(Floorf(_y));
  auto zf = F32_8X(Floorf(_z));
  auto xm1 = xf - F32_8X(1.f);

  u32_8x ltMask = x < F32_8X(0.f);
  u32_8x _Xi = Select(ltMask, U32_8X(xm1), U32_8X(xf)) & 255;

  f32_8x u4 = Fade8x(x - xf);

  auto PrimeX = U32_8X(501125321);
  auto PrimeY = U32_8X(1136930381);
  auto PrimeZ = U32_8X(1720413743);
  auto PrimeW = U32_8X(1066037191);

  auto x0 = U32_8X(xf) * PrimeX;
  auto x1 = x0 + PrimeX;

  auto y0 = U32_8X(yf) * PrimeY;
  auto y1 = y0 + PrimeY;

  auto z0 = U32_8X(zf) * PrimeZ;
  auto z1 = z0 + PrimeZ;

  auto xGrad0 = x - xf;
  auto yGrad0 = y - yf;
  auto zGrad0 = z - zf;

  auto xGrad1 = xGrad0 - F32_8X(1.f);
  auto yGrad1 = yGrad0 - F32_8X(1.f);
  auto zGrad1 = zGrad0 - F32_8X(1.f);


  u32 Index = 0;
  /* for (u32 Index = 0; Index < 8; Index += 4) */
  {

    u32_8x Seed = U32_8X(1066037191);
    u32_8x H0 = HashPrimes(Seed, x0, y0, z0);
    u32_8x H1 = HashPrimes(Seed, x1, y0, z0);
    u32_8x H2 = HashPrimes(Seed, x0, y1, z0);
    u32_8x H3 = HashPrimes(Seed, x1, y1, z0);

    u32_8x H4 = HashPrimes(Seed, x0, y0, z1);
    u32_8x H5 = HashPrimes(Seed, x1, y0, z1);
    u32_8x H6 = HashPrimes(Seed, x0, y1, z1);
    u32_8x H7 = HashPrimes(Seed, x1, y1, z1);

    f32_8x G0 = Grad8x(H0, xGrad0, yGrad0, zGrad0);
    f32_8x G1 = Grad8x(H1, xGrad1, yGrad0, zGrad0);
    f32_8x G2 = Grad8x(H2, xGrad0, yGrad1, zGrad0);
    f32_8x G3 = Grad8x(H3, xGrad1, yGrad1, zGrad0);

    f32_8x G4 = Grad8x(H4, xGrad0, yGrad0, zGrad1);
    f32_8x G5 = Grad8x(H5, xGrad1, yGrad0, zGrad1);
    f32_8x G6 = Grad8x(H6, xGrad0, yGrad1, zGrad1);
    f32_8x G7 = Grad8x(H7, xGrad1, yGrad1, zGrad1);

    auto L0  = Lerp8x(u4, G0, G1);
    auto L1  = Lerp8x(u4, G2, G3);
    auto L2  = Lerp8x(u4, G4, G5);
    auto L3  = Lerp8x(u4, G6, G7);

    auto L4  = Lerp8x(v4, L0, L1);
    auto L5  = Lerp8x(v4, L2, L3);

    auto Res = Lerp8x(w4, L4, L5 );
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
