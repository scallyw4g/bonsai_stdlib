
typedef r32 (*noise_callback)(r32, r32, r32);
typedef r32 (*noise_callback_derivs)(r32, r32, r32, v3*);

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

global_variable u8 Global_PerlinIV[512] = {
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
  /* Assert(t<=1.01f); */
  /* Assert(t>=-0.01f); */
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

link_internal f32
PerlinNoise(f32 x, f32 y, f32 z)
{
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
hash(u8 x, u8 y, u8 z)
{
  return Global_PerlinIV[Global_PerlinIV[Global_PerlinIV[x] + y] + z];
}

// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L281
//
// Compute dot product between vector from cell corners to P with predefined gradient directions
//    perm: a value between 0 and 255
//    float x, float y, float z: coordinates of vector from cell corner to shaded point
//
float gradientDotV( uint8_t perm, float x, float y, float z)
{
  switch (perm & 15)
  {
    case  0: return  x + y; // (1,1,0)
    case  1: return -x + y; // (-1,1,0)
    case  2: return  x - y; // (1,-1,0)
    case  3: return -x - y; // (-1,-1,0)
    case  4: return  x + z; // (1,0,1)
    case  5: return -x + z; // (-1,0,1)
    case  6: return  x - z; // (1,0,-1)
    case  7: return -x - z; // (-1,0,-1)
    case  8: return  y + z; // (0,1,1),
    case  9: return -y + z; // (0,-1,1),
    case 10: return  y - z; // (0,1,-1),
    case 11: return -y - z; // (0,-1,-1)
    case 12: return  y + x; // (1,1,0)
    case 13: return -x + y; // (-1,1,0)
    case 14: return -y + z; // (0,-1,1)
    case 15: return -y - z; // (0,-1,-1)
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
  v3 c000 = Global_PerlinGradients[hash(xi, yi, zi)];
  v3 c100 = Global_PerlinGradients[hash(xi + 1, yi, zi)];
  v3 c010 = Global_PerlinGradients[hash(xi, yi + 1, zi)];
  v3 c110 = Global_PerlinGradients[hash(xi + 1, yi + 1, zi)];

  v3 c001 = Global_PerlinGradients[hash(xi, yi, zi + 1)];
  v3 c101 = Global_PerlinGradients[hash(xi + 1, yi, zi + 1)];
  v3 c011 = Global_PerlinGradients[hash(xi, yi + 1, zi + 1)];
  v3 c111 = Global_PerlinGradients[hash(xi + 1, yi + 1, zi + 1)];

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

  float a = gradientDotV(hash(xi0, yi0, zi0), x0, y0, z0);
  float b = gradientDotV(hash(xi1, yi0, zi0), x1, y0, z0);
  float c = gradientDotV(hash(xi0, yi1, zi0), x0, y1, z0);
  float d = gradientDotV(hash(xi1, yi1, zi0), x1, y1, z0);
  float e = gradientDotV(hash(xi0, yi0, zi1), x0, y0, z1);
  float f = gradientDotV(hash(xi1, yi0, zi1), x1, y0, z1);
  float g = gradientDotV(hash(xi0, yi1, zi1), x0, y1, z1);
  float h = gradientDotV(hash(xi1, yi1, zi1), x1, y1, z1);

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



#endif
