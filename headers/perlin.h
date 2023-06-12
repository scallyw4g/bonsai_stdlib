#ifndef BONSAI_PREPROCESSOR

// This is a modified version of the java implementation of the improved perlin
// function (see http://mrl.nyu.edu/~perlin/noise/)
//
// The original Java implementation is copyright 2002 Ken Perlin

debug_global u32 DEBUG_NOISE_SEED = 64324;

global_variable int Global_PerlinIV[512] = {
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

link_internal f32
fade(f32 t) {
  f32 res = t * t * t * (t * (t * 6 - 15) + 10);
  return res;
}

link_internal f32
lerp(f32 t, f32 a, f32 b) {
  Assert(t<=1);
  Assert(t>=0);
  f32 res = a + t * (b - a);
  return res;
  /* return Lerp(a, t, b); */
}

link_internal f32
grad(int hash, f32 x, f32 y, f32 z) {
  int h = hash & 15;
  // Convert lower 4 bits of hash into 12 gradient directions
  f32 u = h < 8 ? x : y;
  f32 v = h < 4 ? y : h == 12 || h == 14 ? x : z;
  f32 res = ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  return res;
}

f32
PerlinNoise(f32 x, f32 y, f32 z) {
  // Find the unit cube that contains the point
  u32 X = (u32) Floorf(x) & 255;
  u32 Y = (u32) Floorf(y) & 255;
  u32 Z = (u32) Floorf(z) & 255;

  // Find relative x, y,z of point in cube
  f32 floorX = Floorf(x);
  f32 floorY = Floorf(y);
  f32 floorZ = Floorf(z);

  x -= floorX;
  y -= floorY;
  z -= floorZ;

  // Compute fade curves for each of x, y, z
  f32 u = fade(x);
  f32 v = fade(y);
  f32 w = fade(z);

  // Hash coordinates of the 8 cube corners
  u32 A  = (u32)Global_PerlinIV[X]   + Y;
  u32 AA = (u32)Global_PerlinIV[A]   + Z;
  u32 AB = (u32)Global_PerlinIV[A+1] + Z;
  u32 B  = (u32)Global_PerlinIV[X+1] + Y;
  u32 BA = (u32)Global_PerlinIV[B]   + Z;
  u32 BB = (u32)Global_PerlinIV[B+1] + Z;

  Assert(A  >= 0);
  Assert(AA >= 0);
  Assert(AB >= 0);
  Assert(B  >= 0);
  Assert(BA >= 0);
  Assert(BB >= 0);

  Assert(A  <= 511);
  Assert(AA <= 511);
  Assert(AB <= 511);
  Assert(B  <= 511);
  Assert(BA <= 511);
  Assert(BB <= 511);

  // Add blended results from 8 corners of cube
  f32 res = lerp(w,
                 lerp(v,
                      lerp(u,
                           grad(Global_PerlinIV[AA], x, y, z),
                           grad(Global_PerlinIV[BA], x-1, y, z)),
                      lerp(u,
                           grad(Global_PerlinIV[AB], x, y-1, z),
                           grad(Global_PerlinIV[BB], x-1, y-1, z))),
                 lerp(v,
                      lerp(u,
                           grad(Global_PerlinIV[AA+1], x, y, z-1),
                           grad(Global_PerlinIV[BA+1], x-1, y, z-1)),
                      lerp(u,
                           grad(Global_PerlinIV[AB+1], x, y-1, z-1),
                           grad(Global_PerlinIV[BB+1], x-1, y-1, z-1))
                      )
                 );

  res = (res + 1.0f)/2.0f;
  return res;
}


#endif
