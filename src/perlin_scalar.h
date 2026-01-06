#pragma once

#include "primitives.h"

#ifndef BONSAI_PREPROCESSOR
// This is a modified version of the java implementation of the improved perlin
// function (see http://mrl.nyu.edu/~perlin/noise/)
//
// The original Java implementation is copyright 2002 Ken Perlin

debug_global u32 DEBUG_NOISE_SEED = 64324;

/* global_variable b32 Global_GradientTableInitialized = False; */
/* global_variable v3 Global_PerlinGradients[256] = {}; */

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
