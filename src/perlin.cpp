
link_internal void
PerlinNoise_8x(f32 *_x, f32 yIn, f32 zIn, f32 *Result)
{
#if 0
  RangeIterator(Index, 8)
  {
    f32 x = _x[Index];
    f32 y = yIn;
    f32 z = zIn;

    // Find the unit cube that contains the point
    if (x < 0) x -= 1.f;
    if (y < 0) y -= 1.f;
    if (z < 0) z -= 1.f;

    u32 Xi = u32(x) & 255;
    u32 Yi = u32(y) & 255;
    u32 Zi = u32(z) & 255;

    // Find relative x,y,z of point in cube
    x -= Floorf(x);
    y -= Floorf(y);
    z -= Floorf(z);

    // Compute fade curves for each of x, y, z
    f32 u = fade(x);
    f32 v = fade(y);
    f32 w = fade(z);

    // Hash coordinates of the 8 cube corners
    u32 A  = (u32)Global_PerlinIV[Xi]  + Yi;
    u32 AA = (u32)Global_PerlinIV[A]   + Zi;
    u32 AB = (u32)Global_PerlinIV[A+1] + Zi;
    u32 B  = (u32)Global_PerlinIV[Xi+1]+ Yi;
    u32 BA = (u32)Global_PerlinIV[B]   + Zi;
    u32 BB = (u32)Global_PerlinIV[B+1] + Zi;

    // Add blended results from 8 corners of cube

    u32 H0 = Global_PerlinIV[AA];
    u32 H1 = Global_PerlinIV[BA];
    u32 H2 = Global_PerlinIV[AB];
    u32 H3 = Global_PerlinIV[BB];
    u32 H4 = Global_PerlinIV[AA+1];
    u32 H5 = Global_PerlinIV[BA+1];
    u32 H6 = Global_PerlinIV[AB+1];
    u32 H7 = Global_PerlinIV[BB+1];

    f32 G0 = grad(H0,  x,    y,    z);
    f32 G1 = grad(H1,  x-1,  y,    z);
    f32 G2 = grad(H2,  x,    y-1,  z);
    f32 G3 = grad(H3,  x-1,  y-1,  z);

    f32 G4 = grad(H4,  x,    y,    z-1);
    f32 G5 = grad(H5,  x-1,  y,    z-1);
    f32 G6 = grad(H6,  x,    y-1,  z-1);
    f32 G7 = grad(H7,  x-1,  y-1,  z-1);

    f32 L0 = lerp(u, G0, G1);
    f32 L1 = lerp(u, G2, G3);
    f32 L2 = lerp(u, G4, G5);
    f32 L3 = lerp(u, G6, G7);

    f32_4x u_4x = F32_4X(u,u,u,u);

    f32_4x LHS = F32_4X(G0, G2, G4, G6); // left-hand column of operands
    f32_4x RHS = F32_4X(G1, G3, G5, G7); // right-hand column of operands

    f32_4x L0123 = Lerp4x(u_4x, LHS, RHS);

    f32 L4 = lerp(v, L0123.E[0], L0123.E[1]);
    f32 L5 = lerp(v, L0123.E[2], L0123.E[3]);
    f32 res = lerp(w, L4, L5 );

    res = (res + 1.0f)/2.0f;

    /* Assert(res <= 1.05f); */
    /* Assert(res > -1.05f); */
    Result[Index] = res;
  }
#else
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
    u32 Xi_0 = _Xi[Index];
    f32  x_0 = _xF[Index];

    u32 Xi_1 = _Xi[Index+1];
    f32  x_1 = _xF[Index+1];

    u32 Xi_2 = _Xi[Index+2];
    f32  x_2 = _xF[Index+2];

    u32 Xi_3 = _Xi[Index+3];
    f32  x_3 = _xF[Index+3];


    // Hash coordinates of the 8 cube corners
    u32  A_0 = Global_PerlinIV[ Xi_0  ] + Yi;
    u32 AA_0 = Global_PerlinIV[  A_0  ] + Zi;
    u32 AB_0 = Global_PerlinIV[  A_0+1] + Zi;
    u32  B_0 = Global_PerlinIV[ Xi_0+1] + Yi;
    u32 BA_0 = Global_PerlinIV[  B_0  ] + Zi;
    u32 BB_0 = Global_PerlinIV[  B_0+1] + Zi;

    u32 H0_0 = Global_PerlinIV[AA_0  ];
    u32 H1_0 = Global_PerlinIV[BA_0  ];
    u32 H2_0 = Global_PerlinIV[AB_0  ];
    u32 H3_0 = Global_PerlinIV[BB_0  ];
    u32 H4_0 = Global_PerlinIV[AA_0+1];
    u32 H5_0 = Global_PerlinIV[BA_0+1];
    u32 H6_0 = Global_PerlinIV[AB_0+1];
    u32 H7_0 = Global_PerlinIV[BB_0+1];


    u32  A_1 = Global_PerlinIV[ Xi_1  ] + Yi;
    u32 AA_1 = Global_PerlinIV[  A_1  ] + Zi;
    u32 AB_1 = Global_PerlinIV[  A_1+1] + Zi;
    u32  B_1 = Global_PerlinIV[ Xi_1+1] + Yi;
    u32 BA_1 = Global_PerlinIV[  B_1  ] + Zi;
    u32 BB_1 = Global_PerlinIV[  B_1+1] + Zi;
 
    u32 H0_1 = Global_PerlinIV[AA_1  ];
    u32 H1_1 = Global_PerlinIV[BA_1  ];
    u32 H2_1 = Global_PerlinIV[AB_1  ];
    u32 H3_1 = Global_PerlinIV[BB_1  ];
    u32 H4_1 = Global_PerlinIV[AA_1+1];
    u32 H5_1 = Global_PerlinIV[BA_1+1];
    u32 H6_1 = Global_PerlinIV[AB_1+1];
    u32 H7_1 = Global_PerlinIV[BB_1+1];

    u32  A_2 = Global_PerlinIV[ Xi_2  ] + Yi;
    u32 AA_2 = Global_PerlinIV[  A_2  ] + Zi;
    u32 AB_2 = Global_PerlinIV[  A_2+1] + Zi;
    u32  B_2 = Global_PerlinIV[ Xi_2+1] + Yi;
    u32 BA_2 = Global_PerlinIV[  B_2  ] + Zi;
    u32 BB_2 = Global_PerlinIV[  B_2+1] + Zi;

    u32 H0_2 = Global_PerlinIV[AA_2  ];
    u32 H1_2 = Global_PerlinIV[BA_2  ];
    u32 H2_2 = Global_PerlinIV[AB_2  ];
    u32 H3_2 = Global_PerlinIV[BB_2  ];
    u32 H4_2 = Global_PerlinIV[AA_2+1];
    u32 H5_2 = Global_PerlinIV[BA_2+1];
    u32 H6_2 = Global_PerlinIV[AB_2+1];
    u32 H7_2 = Global_PerlinIV[BB_2+1];


    u32  A_3 = Global_PerlinIV[ Xi_3  ] + Yi;
    u32 AA_3 = Global_PerlinIV[  A_3  ] + Zi;
    u32 AB_3 = Global_PerlinIV[  A_3+1] + Zi;
    u32  B_3 = Global_PerlinIV[ Xi_3+1] + Yi;
    u32 BA_3 = Global_PerlinIV[  B_3  ] + Zi;
    u32 BB_3 = Global_PerlinIV[  B_3+1] + Zi;

    u32 H0_3 = Global_PerlinIV[AA_3  ];
    u32 H1_3 = Global_PerlinIV[BA_3  ];
    u32 H2_3 = Global_PerlinIV[AB_3  ];
    u32 H3_3 = Global_PerlinIV[BB_3  ];
    u32 H4_3 = Global_PerlinIV[AA_3+1];
    u32 H5_3 = Global_PerlinIV[BA_3+1];
    u32 H6_3 = Global_PerlinIV[AB_3+1];
    u32 H7_3 = Global_PerlinIV[BB_3+1];


#define SIMD_PERLIN (1)
#if SIMD_PERLIN


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


#else
    f32 G0 = grad(H0,  x,    y,    z);
    f32 G1 = grad(H1,  x-1,  y,    z);
    f32 G2 = grad(H2,  x,    y-1,  z);
    f32 G3 = grad(H3,  x-1,  y-1,  z);

    f32 G4 = grad(H4,  x,    y,    z-1);
    f32 G5 = grad(H5,  x-1,  y,    z-1);
    f32 G6 = grad(H6,  x,    y-1,  z-1);
    f32 G7 = grad(H7,  x-1,  y-1,  z-1);

    f32 L0 = lerp(u, G0, G1);
    f32 L1 = lerp(u, G2, G3);
    f32 L2 = lerp(u, G4, G5);
    f32 L3 = lerp(u, G6, G7);
    f32 L4 = lerp(v, L0, L1);
    f32 L5 = lerp(v, L2, L3);
    f32 res = lerp(w, L4, L5 );
#endif



    /* res_0 = (res_0 + 1.0f)/2.0f; */
    /* res_1 = (res_1 + 1.0f)/2.0f; */

    /* Result[Index] = res_0; */
    /* Result[Index+1] = res_1; */
  }
#endif
}
