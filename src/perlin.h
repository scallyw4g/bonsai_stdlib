struct perlin_params
{
  u32_8x P0;
  u32_8x P1;
  f32_8x Fract0;
  f32_8x Fract1;
  f32_8x Fade;
};
typedef r32 (*noise_callback)(r32, r32, r32);
typedef r32 (*noise_callback_derivs)(r32, r32, r32, v3*);

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
    

// NOTE(Jesse): This is very slow and for debug only, calculate normals analytically!
// https://www.shadertoy.com/view/XttSz2
link_internal v3
CalcNormal( v3 P, r32 NoiseValue, noise_callback_derivs NoiseFn )
{
  /* f32 eps = 1.f; */
  f32 eps = 0.0002f;

  r32 pX = P.x + eps;
  r32 pY = P.y + eps;
  r32 pZ = P.z + eps;

  v3 Ignored;
  v3 nor = V3( NoiseFn(pX, P.y, P.z, &Ignored) - NoiseValue,
               NoiseFn(P.x, pY, P.z, &Ignored) - NoiseValue,
               NoiseFn(P.x, P.y, pZ, &Ignored) - NoiseValue );

  return Normalize(nor);
}

link_internal v3
CalcNormal( v3 P, r32 NoiseValue, noise_callback NoiseFn )
{
  /* f32 eps = 1.f; */
  f32 eps = 0.0002f;

  r32 pX = P.x + eps;
  r32 pY = P.y + eps;
  r32 pZ = P.z + eps;

  v3 nor = V3( NoiseFn(pX, P.y, P.z) - NoiseValue,
               NoiseFn(P.x, pY, P.z) - NoiseValue,
               NoiseFn(P.x, P.y, pZ) - NoiseValue );

  return Normalize(nor);
}


// TODO(Jesse): Where should this go?
link_internal void
CalculateTBN(v3 derivs, v3 *Tangent, v3 *Bitangent, v3 *Normal)
{
  /* *Tangent = V3(1.f, derivs.x, 0.f); */
  /* *Bitangent = V3(0.f, derivs.y, 1.f); */

  /* *Tangent = V3(1.f, derivs.x, 0.f); */
  /* *Bitangent = V3(0.f, 1.f, derivs.y); */

  *Tangent = Normalize(V3(0.f, derivs.z, 1.f));
  *Bitangent = Normalize(V3(1.f, derivs.x, 0.f));

  // equivalent to Cross(bitangent, tangent)
  *Normal = Normalize(V3(-derivs.x, 1.f, -derivs.z));
  /* *Normal = Normalize(Cross(*Bitangent, *Tangent)); */
}

#ifndef BONSAI_PREPROCESSOR
// This is a modified version of the java implementation of the improved perlin
// function (see http://mrl.nyu.edu/~perlin/noise/)
//
// The original Java implementation is copyright 2002 Ken Perlin

debug_global u32 DEBUG_NOISE_SEED = 64324;

global_variable b32 Global_GradientTableInitialized = False;
global_variable v3 Global_PerlinGradients[256] = {};

global_variable u32 Global_PerlinIV[512] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
        8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
        35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
        134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
        55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
        18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
        250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
        189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,
        43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
        97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
        107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
        8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
        35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
        134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
        55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
        18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
        250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
        189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,
        43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
        97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
        107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
        138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

// TODO(Jesse): Rename/remove these
link_internal f32
fade(f32 t) {
  f32 res = t * t * t * (t * (t * 6 - 15) + 10);
  return res;
}

link_internal f32_4x
Fade4x(f32_4x t) {
  f32_4x _15 = F32_4X(15);
  f32_4x _10 = F32_4X(10);
  f32_4x _6  = F32_4X(6);
  f32_4x res = t * t * t * (t * (t * _6 - _15) + _10);
  return res;
}

link_internal f32_8x
Fade8x(f32_8x t) {
  f32_8x _15 = F32_8X(15);
  f32_8x _10 = F32_8X(10);
  f32_8x _6  = F32_8X(6);
  f32_8x res = t * t * t * (t * (t * _6 - _15) + _10);
  return res;
}


link_internal f32
FadeQuintic(f32 t) {
  return fade(t);
}

link_internal f32_8x
FadeQuintic(f32_8x t) {
  return Fade8x(t);
}



link_internal f32
lerp(f32 t, f32 a, f32 b) {
  f32 res = a + t * (b - a);
  return res;
}

link_internal f32_4x
Lerp4x(f32_4x t, f32_4x a, f32_4x b)
{
  f32_4x res = a + t * (b - a);
  return res;
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



link_internal f32
grad(u32 hash, f32 x, f32 y, f32 z)
{
  switch (hash & 15)
  {
    case 0 : return  x + y; // ( 1, 1, 0)
    case 1 : return  x + z; // ( 1, 0, 1)
    case 8 : return  x - y; // ( 1,-1, 0)
    case 9 : return  x - z; // ( 1, 0,-1)
    case 12: return -x - y; // (-1,-1, 0)
    case 13: return -x - z; // (-1, 0,-1)

    case 3 : return  y + x; // ( 1, 1, 0)
    case 2 : return  y + z; // ( 0, 1, 1)
    case 7 : return  y - x; // (-1, 1, 0)
    case 10: return  y - z; // ( 0, 1,-1)
    case 14: return -y - z; // ( 0,-1,-1)
    case 15: return -y - z; // ( 0,-1,-1)

    case 5 : return  z - x; // (-1, 0, 1)
    case 6 : return  z - y; // ( 0,-1, 1)
    case 11: return  z - y; // ( 0,-1, 1)
  }
  return -1;
}

link_internal f32
grad_(u32 hash, f32 x, f32 y, f32 z)
{
  switch (hash & 15)
  {
    case  0: return  x + y; // ( 1, 1, 0)
    case  1: return -x + y; // (-1, 1, 0)
    case  2: return  x - y; // ( 1,-1, 0)
    case  3: return -x - y; // (-1,-1, 0)

    case  4: return  x + z; // ( 1, 0, 1)
    case  5: return -x + z; // (-1, 0, 1)
    case  6: return  x - z; // ( 1, 0,-1)
    case  7: return -x - z; // (-1, 0,-1)
                            //
    case  8: return  y + z; // ( 0, 1, 1)
    case  9: return -y + z; // ( 0,-1, 1)
    case 10: return  y - z; // ( 0, 1,-1)
    case 11: return -y - z; // ( 0,-1,-1)
                            //
    case 12: return  y + x; // ( 1, 1, 0)
    case 13: return -x + y; // (-1, 1, 0)
    case 14: return -y + z; // ( 0,-1, 1)
    case 15: return -y - z; // ( 0,-1,-1)
  }
  return f32_MAX;
}

link_internal f32_4x
Grad4x(u32_4x hash, f32_4x x, f32_4x y, f32_4x z)
{
  u32_4x _15 = U32_4X(15);
  u32_4x _14 = U32_4X(14);
  u32_4x _12 = U32_4X(12);
  u32_4x _8  = U32_4X(8);
  u32_4x _4  = U32_4X(4);
  u32_4x _2  = U32_4X(2);
  u32_4x _1  = U32_4X(1);
  f32_4x _n1 = F32_4X(-1);

  auto h = hash & _15;

  u32_4x uSel = h < _8;
  u32_4x vSel = h < _4;
  u32_4x xSel = (h == _12 | h == _14 );

  f32_4x u  = Select(uSel, x, y);
  f32_4x xz = Select(xSel, x, z);
  f32_4x v  = Select(vSel, y, xz);

  auto uFlip = (h & _1) == _1;
  auto vFlip = (h & _2) == _2;

  f32_4x R0 = Select(uFlip, u*_n1, u);
  f32_4x R1 = Select(vFlip, v*_n1, v);
  f32_4x Result = R0 + R1;

  return Result;
}


link_internal f32_8x
Grad8x_fast(u32_8x hash, f32_8x x, f32_8x y, f32_8x z)
{
#if 0
  u32_8x Trunc = hash & U32_8X(0xFFFF);
  f32_8x Result =  (F32_8X(Trunc) / F32_8X(0xFFFF/2)) - F32_8X(1.f);
#endif

#if 0
  u32_8x Trunc = hash & U32_8X(0xFFFF);
  f32_8x Result =  ((F32_8X(Trunc) / F32_8X(0xFFFF)) * F32_8X(2.f)) - F32_8X(1.f);
#endif

#if 1
  /* f32_8x Result =  F32_8X(hash) / F32_8X(0xFFFFFFFF); */
  /* f32_8x Result =  (F32_8X(hash) / F32_8X(0xFFFFFFFF/2)) - F32_8X(1.f); */
  f32_8x Result =  F32_8X(hash) / F32_8X(0xFFFFFFFF/2);
#endif
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
  /* u32_8x xSel = (h == _12 | h == _14 ); */

  f32_8x u  = Select(uSel, x, y);
  /* f32_8x xz = Select(xSel, x, z); */
  f32_8x v  = Select(vSel, y, z);

  auto h1 = hash << 31;
  auto h2 = (hash & U32_8X(2)) << 30 ;
  f32_8x Result = ( u ^ Cast_f32_8x(h1) ) + ( v ^ Cast_f32_8x(h2) );

  return Result;
}


link_internal f32
PerlinNoise(f32 x, f32 y, f32 z)
{
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

  f32 L4 = lerp(v, L0, L1);
  f32 L5 = lerp(v, L2, L3);
  f32 res = lerp(w, L4, L5 );

  res = (res + 1.0f)/2.0f;

  /* Assert(res <= 1.05f); */
  /* Assert(res > -1.05f); */
  return res;
}

link_internal f32
PerlinNoise(v3 P)
{
  return PerlinNoise(P.x, P.y, P.z);
}

#endif


// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L94
link_internal f32
Smoothstep(f32 N)
{
  f32 Result = N * N * (3.f - 2.f * N);
  return Result;
}

// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L106
link_internal f32
SmoothstepDeriv(f32 N)
{
  f32 Result = N * (6.f - 6.f * N);
  return Result;
}

// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L276
link_internal u8
hash_u8(u8 x, u8 y, u8 z)
{
  return Cast(u8, Global_PerlinIV[Global_PerlinIV[Global_PerlinIV[x] + y] + z]);
}

// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L281
//
// Compute dot product between vector from cell corners to P with predefined gradient directions
//    perm: a value between 0 and 255
//    float x, float y, float z: coordinates of vector from cell corner to shaded point
//
float gradientDotV( u8 perm, float x, float y, float z)
{
  switch (perm & 15)
  {
    case  0: return  x + y; // ( 1, 1, 0)
    case  1: return -x + y; // (-1, 1, 0)
    case  2: return  x - y; // ( 1,-1, 0)
    case  3: return -x - y; // (-1,-1, 0)

    case  4: return  x + z; // ( 1, 0, 1)
    case  5: return -x + z; // (-1, 0, 1)
    case  6: return  x - z; // ( 1, 0,-1)
    case  7: return -x - z; // (-1, 0,-1)
                            //
    case  8: return  y + z; // ( 0, 1, 1)
    case  9: return -y + z; // ( 0,-1, 1)
    case 10: return  y - z; // ( 0, 1,-1)
    case 11: return -y - z; // ( 0,-1,-1)
                            //
    case 12: return  y + x; // ( 1, 1, 0)
    case 13: return -x + y; // (-1, 1, 0)
    case 14: return -y + z; // ( 0,-1, 1)
    case 15: return -y - z; // ( 0,-1,-1)
  }
  return f32_MAX;
}

// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L136C1-L142C43
link_internal void
InitGlobalGradientTable()
{
  random_series Entropy = {DEBUG_NOISE_SEED};

  RangeIterator(i, 256)
  {
    float theta = ArcCos(RandomBilateral(&Entropy));
    float phi = 2.f*PI32*RandomUnilateral(&Entropy);

    float x = Cos(phi) * Sin(theta);
    float y = Sin(phi) * Sin(theta);
    float z = Cos(theta);
    Global_PerlinGradients[i] = V3(x, y, z);
  }

  Global_GradientTableInitialized = True;
}

#if 1
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2/perlin-noise-computing-derivatives.html
link_internal f32
PerlinNoise_Derivitives0(f32 px, f32 py, f32 pz, v3 *derivs, v3 *Normal)
{
  if (Global_GradientTableInitialized == False) { InitGlobalGradientTable(); }

  u8 xi = u8(((u32)Floorf(px)) & 255);
  u8 yi = u8(((u32)Floorf(py)) & 255);
  u8 zi = u8(((u32)Floorf(pz)) & 255);

  float tx = px - xi;
  float ty = py - yi;
  float tz = pz - zi;

  float u = Smoothstep(tx);
  float v = Smoothstep(ty);
  float w = Smoothstep(tz);

  float du = SmoothstepDeriv(tx);
  float dv = SmoothstepDeriv(ty);
  float dw = SmoothstepDeriv(tz);

  // gradients at the corner of the cell
  v3 c000 = Global_PerlinGradients[hash_u8(xi, yi, zi)];
  v3 c100 = Global_PerlinGradients[hash_u8(xi + 1, yi, zi)];
  v3 c010 = Global_PerlinGradients[hash_u8(xi, yi + 1, zi)];
  v3 c110 = Global_PerlinGradients[hash_u8(xi + 1, yi + 1, zi)];

  v3 c001 = Global_PerlinGradients[hash_u8(xi, yi, zi + 1)];
  v3 c101 = Global_PerlinGradients[hash_u8(xi + 1, yi, zi + 1)];
  v3 c011 = Global_PerlinGradients[hash_u8(xi, yi + 1, zi + 1)];
  v3 c111 = Global_PerlinGradients[hash_u8(xi + 1, yi + 1, zi + 1)];

  // generate vectors going from the grid points to p
  float x0 = tx, x1 = tx - 1;
  float y0 = ty, y1 = ty - 1;
  float z0 = tz, z1 = tz - 1;

  v3 p000 = V3(x0, y0, z0);
  v3 p100 = V3(x1, y0, z0);
  v3 p010 = V3(x0, y1, z0);
  v3 p110 = V3(x1, y1, z0);

  v3 p001 = V3(x0, y0, z1);
  v3 p101 = V3(x1, y0, z1);
  v3 p011 = V3(x0, y1, z1);
  v3 p111 = V3(x1, y1, z1);

  float a = Dot(c000, p000);
  float b = Dot(c100, p100);
  float c = Dot(c010, p010);
  float d = Dot(c110, p110);
  float e = Dot(c001, p001);
  float f = Dot(c101, p101);
  float g = Dot(c011, p011);
  float h = Dot(c111, p111);

  float k0 = (b - a);
  float k1 = (c - a);
  float k2 = (e - a);
  float k3 = (a + d - b - c);
  float k4 = (a + f - b - e);
  float k5 = (a + g - c - e);
  float k6 = (b + c + e + h - a - d - f - g);

  derivs->x = du *(k0 + v * k3 + w * k4 + v * w * k6);
  derivs->y = dv *(k1 + u * k3 + w * k5 + u * w * k6);
  derivs->z = dw *(k2 + u * k4 + v * k5 + u * v * k6);


  v3 tangent = V3(1.f, derivs->x, 0.f);
  v3 bitangent = V3(0.f, derivs->z, 1.f);

  // equivalent to Cross(bitangent, tangent)
  *Normal = Normalize(V3(-derivs->x, 1.f, -derivs->z));

  return a + u * k0 + v * k1 + w * k2 + u * v * k3 + u * w * k4 + v * w * k5 + u * v * w * k6;
}
#endif

link_internal float
quintic(float t)
{
  return t * t * t * (t * (t * 6.f - 15.f) + 10.f);
}

link_internal float
quinticDeriv(float t)
{
  return 30 * t * t * (t * (t - 2) + 1);
}


link_internal f32
PerlinNoise_Derivitives1(r32 px, r32 py, r32 pz, v3 *derivs)
{
  u8 tableSizeMask = 0xff;

  u8 xi0 = u8(px);
  u8 yi0 = u8(py);
  u8 zi0 = u8(pz);

  u8 xi1 = u8(xi0 + 1);
  u8 yi1 = u8(yi0 + 1);
  u8 zi1 = u8(zi0 + 1);

#if 0
  float tx = Fract(px);
  float ty = Fract(py);
  float tz = Fract(pz);
#else
  /* float tx = (px - Floorf(px)) / 100000.f; */
  /* float ty = (py - Floorf(py)) / 100000.f; */
  /* float tz = (pz - Floorf(pz)) / 100000.f; */

  float tx = (px - Floorf(px));
  float ty = (py - Floorf(py));
  float tz = (pz - Floorf(pz));

  /* float tx = 0.f; // (px - Floorf(px)); */
  /* float ty = 0.f; // (py - Floorf(py)); */
  /* float tz = 0.f; // (pz - Floorf(pz)); */
#endif

  float u = quintic(tx);
  float v = quintic(ty);
  float w = quintic(tz);

  // generate vectors going from the grid points to p
  float x0 = tx, x1 = tx - 1;
  float y0 = ty, y1 = ty - 1;
  float z0 = tz, z1 = tz - 1;

  float a = gradientDotV(hash_u8(xi0, yi0, zi0), x0, y0, z0);
  float b = gradientDotV(hash_u8(xi1, yi0, zi0), x1, y0, z0);
  float c = gradientDotV(hash_u8(xi0, yi1, zi0), x0, y1, z0);
  float d = gradientDotV(hash_u8(xi1, yi1, zi0), x1, y1, z0);
  float e = gradientDotV(hash_u8(xi0, yi0, zi1), x0, y0, z1);
  float f = gradientDotV(hash_u8(xi1, yi0, zi1), x1, y0, z1);
  float g = gradientDotV(hash_u8(xi0, yi1, zi1), x0, y1, z1);
  float h = gradientDotV(hash_u8(xi1, yi1, zi1), x1, y1, z1);

  float du = quinticDeriv(tx);
  float dv = quinticDeriv(ty);
  float dw = quinticDeriv(tz);

  float k0 = a;
  float k1 = (b - a);
  float k2 = (c - a);
  float k3 = (e - a);
  float k4 = (a + d - b - c);
  float k5 = (a + f - b - e);
  float k6 = (a + g - c - e);
  float k7 = (b + c + e + h - a - d - f - g);

  derivs->x = du * (k1 + k4 * v + k5 * w + k7 * v * w);
  derivs->y = dv * (k2 + k4 * u + k6 * w + k7 * v * w);
  derivs->z = dw * (k3 + k5 * u + k6 * v + k7 * v * w);

  /* derivs->x = v; */
  /* derivs->y = u; */
  /* derivs->z = w; */

  /* derivs->x = Fract(px); */
  /* derivs->y = Fract(py); */
  /* derivs->z = Fract(pz); */

  /* derivs->x = Floorf(px); */
  /* derivs->y = Floorf(py); */
  /* derivs->z = Floorf(pz); */

  /* derivs->x = px; */
  /* derivs->y = py; */
  /* derivs->z = pz; */

  /* derivs->x = tx; */
  /* derivs->y = ty; */
  /* derivs->z = tz; */

  return k0 + k1 * u + k2 * v + k3 * w + k4 * u * v + k5 * u * w + k6 * v * w + k7 * u * v * w;
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

#if 0
link_inline perlin_params
ComputePerlinParameters(u32_8x Basis, u32_8x Offset, u32_8x ChunkResolution, u32_8x Period, u32_8x Prime)
{
#if 1
  u32_8x AbsoluteWorldP = (Basis + Offset*ChunkResolution);

  u32_8x Cell = AbsoluteWorldP / Period;

  u32_8x Rem = AbsoluteWorldP % Period;
  f32_8x Fract0 = F32_8X(Rem)/F32_8X(Period);
  f32_8x Fract1 = Fract0 - F32_8X(1);
#else
  f32_8x Input = (F32_8X(Basis) + F32_8X(Offset)*F32_8X(ChunkResolution)) 
                  / F32_8X(Period);
  f32_8x Cellf = Floor(Input);
  u32_8x Cell = U32_8X(Cellf);
  f32_8x Fract0 = Input-Cellf;
  f32_8x Fract1 = Fract0 - F32_8X(1);
#endif

  u32_8x P0 = Cell * Prime;
  u32_8x P1 = P0 + Prime;

  f32_8x Fade = Fade8x(Fract0);

  perlin_params Result =
  {
    P0,
    P1,
    Fract0,
    Fract1,
    Fade,
  };
  return Result;
}
#endif
