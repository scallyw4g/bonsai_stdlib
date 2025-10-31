

#extension GL_ARB_explicit_uniform_location : enable


precision highp float;
precision highp sampler2DShadow;
precision highp sampler2D;
precision highp sampler3D;

// NOTE(Jesse): Must match defines in render.h
#define VERTEX_POSITION_LAYOUT_LOCATION 0
#define VERTEX_NORMAL_LAYOUT_LOCATION 1
#define VERTEX_COLOR_LAYOUT_LOCATION 2
#define VERTEX_TRANS_EMISS_LAYOUT_LOCATION 3

// NOTE(Jesse): Must match defines in render.h
#define RENDERER_MAX_LIGHT_EMISSION_VALUE (5.f)




// https://thebookofshaders.com/10/
float rand(vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}


#define Floor floor
#define Abs abs
#define Clamp01(x) clamp(x, 0.f, 1.f)
#define LengthSq(V) (V.x*V.x + V.y*V.y + V.z*V.z)
#define Normalize normalize
#define Dot dot
#define Min min

#define True true
#define False false
#define b32 bool

#define f32_MAX (1E+37f)
#define f32_MIN (1E-37f)

#define v4 vec4
#define v3 vec3
#define v2 vec2
#define V4 vec4
#define V3 vec3
#define V2 vec2

#define v2i ivec2
#define v3i ivec3
#define v4i ivec4
#define V2i ivec2
#define V3i ivec3
#define V4i ivec4


#define r32 float
#define f32 float
#define u32 unsigned int
#define s32 int

#define PI64 (3.1415926535897932384626433832795028841971693993)
#define PI32 (float(PI64))

#define link_internal

// @braindead_truncate
r32 Truncate(r32 Input)
{
  s32 Truncated = s32(Input);
  r32 Result = r32(Truncated);
  return Result;
}

f32 SafeDivide0(f32 Dividend, f32 Divisor) { return Divisor != 0.f ? Dividend/Divisor : 0.f; }

v3 SafeDivide0(v3 Dividend, v3 Divisor) { return V3( SafeDivide0(Dividend.x, Divisor.x),
                                                     SafeDivide0(Dividend.y, Divisor.y),
                                                     SafeDivide0(Dividend.z, Divisor.z) ); }


float hash(float x)  { return fract(x + 1.32154 * 1.2151); }
vec3 RandomV3FromFloat(float x) { return vec3(hash(((x   + 0.5283) * 59.3829) * 274.3487), hash(((x   + 0.8192) * 83.6621) * 345.3871), hash(((x   + 0.2157f) * 36.6521f) * 458.3971f)); }
vec3 RandomV3FromV3(v3 V)       { return vec3(rand(V.xy), rand(V.yz), rand(V.xz)); }

#define PoissonDiskSize 16
vec2 poissonDisk[PoissonDiskSize] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2(  0.94558609, -0.76890725 ),
   vec2( -0.09418410, -0.92938870 ),
   vec2(  0.34495938,  0.29387760 ),
   vec2( -0.91588581,  0.45771432 ),
   vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543,  0.27676845 ),
   vec2(  0.97484398,  0.75648379 ),

   vec2(  0.44323325, -0.97511554 ),
   vec2(  0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ),
   vec2(  0.79197514,  0.19090188 ),
   vec2( -0.24188840,  0.99706507 ),
   vec2( -0.81409955,  0.91437590 ),
   vec2(  0.19984126,  0.78641367 ),
   vec2(  0.14383161, -0.14100790 )
);

// TODO(Jesse): Metaprogram these.  I've already got them in the C++ math lib
float MapValueToRange(float value, float inMin, float inMax, float outMin, float outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec2 MapValueToRange(vec2 value, vec2 inMin, vec2 inMax, vec2 outMin, vec2 outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec3 MapValueToRange(vec3 value, vec3 inMin, vec3 inMax, vec3 outMin, vec3 outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

vec4 MapValueToRange(vec4 value, vec4 inMin, vec4 inMax, vec4 outMin, vec4 outMax) {
  return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

#if 0
v4 BravoilMyersWeightedAverage(v4 Accum, float Count)
{
  Count = max(1.f, Count);

  // Have to clamp because this is > 1.f for emissive surfaces, which breaks the following equation
  float Alpha = clamp(Accum.a, 0.f, 1.f);

  v3 ColorResult = Accum.rgb/max(Accum.a, 0.00001f);

  float AlphaResult = pow(max(0.0, 1.0-Alpha/Count), Count);
  return v4(ColorResult * AlphaResult, AlphaResult);
}

v4 BravoilMcGuireDepthWeights(v4 Accum, float Revealage)
{
  v3 ColorResult = (Accum.rgb / clamp(Accum.a, 1e-4, 5e4));
  return V4(ColorResult, Revealage);
}
#endif


float Linearize(float zDepth, float Far, float Near)
{
  float Result = ((2.0 * Near) / (Far + Near - zDepth * (Far - Near)));
  return Result;
}

v3 WorldPositionFromNonlinearDepth(float NonlinearDepth, v2 ScreenUV, mat4 InverseProjectionMatrix, mat4 InverseViewMatrix)
{
  v4 WorldP = V4(0,0,0,0);
  f32 ClipZ = (NonlinearDepth*2.f) - 1.f;
  v2 ClipXY = (ScreenUV*2.f)-1.f;

  v4 ClipP = V4(ClipXY.x, ClipXY.y, ClipZ, 1.f);
  v4 ViewP = InverseProjectionMatrix * ClipP;

  ViewP /= ViewP.w;

  WorldP = InverseViewMatrix * ViewP;
  return WorldP.xyz;
}

link_internal v3
UnpackV3_15b(s32 Packed)
{
  s32 FiveBits = 31;

  r32 H = ((Packed >> 10) & FiveBits) / r32(FiveBits);
  r32 S = ((Packed >> 5) & FiveBits) / r32(FiveBits);
  r32 V =  (Packed & FiveBits) / r32(FiveBits);
  v3 Result = V3(H, S, V);
  return Result;
}

//
// https://github.com/Inseckto/HSV-to-RGB/blob/master/HSV2RGB.c
//
v3
HSVtoRGB(f32 H, f32 S, f32 V)
{
  f32 r = 0, g = 0, b = 0;

  f32 h = H;
  f32 s = S;
  f32 v = V;

  int i = s32(floor(h * 6));
  f32 f = h * 6 - i;
  f32 p = v * (1 - s);
  f32 q = v * (1 - f * s);
  f32 t = v * (1 - (1 - f) * s);

  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }

  v3 Result = V3(r,g,b);
  return Result;
}

v3
HSVtoRGB(v3 HSV)
{
  return HSVtoRGB(HSV.r, HSV.g, HSV.b);
}

v3
UnpackHSVColorToRGB(s32 Packed)
{
  v3 HSV = UnpackV3_15b(Packed);
  v3 Result = HSVtoRGB(HSV);
  return Result;
}

uint PackRGB(v3 Color)
{
  uint FiveBits    = (1u <<  5) - 1u;
  uint FifteenBits = (1u << 15) - 1u;
  uint R = uint(round(Color.r * f32(FiveBits))) & FiveBits;
  uint G = uint(round(Color.g * r32(FiveBits))) & FiveBits;
  uint B = uint(round(Color.b * r32(FiveBits))) & FiveBits;
  uint Result = uint((R << 10) | (G << 5) | B) & FifteenBits;
  return Result;
}

vec3 ivhash( ivec3 p )     // this hash is not production ready, please
{                        // replace this by something better
  ivec3 n = ivec3( p.x*127 + p.y*311 + p.z*74,
                   p.x*269 + p.y*183 + p.z*246,
                   p.x*113 + p.y*271 + p.z*124);

  // 1D hash by Hugo Elias
  n = (n << 13) ^ n;
  n = n * (n * n * 15731 + 789221) + 1376312589;
  return -1.0+2.0*vec3( n & ivec3(0x0fffffff))/float(0x0fffffff);
}

vec3 vhash( vec3 p )      // this hash is not production ready, please
{                        // replace this by something better
  p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
            dot(p,vec3(269.5,183.3,246.1)),
            dot(p,vec3(113.5,271.9,124.6)));

  return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}


uint
ChrisWellonsIntegerHash_lowbias32(uint x)
{
  uint Result = x;

  Result = Result ^ (Result >> 16);
  Result = Result ^ uint(0x7feb352d);
  Result = Result ^ (Result >> 15);
  Result = Result * uint(0x846ca68b);
  Result = Result ^ (Result >> 16);

  return Result;
}

// TODO(Jesse): I would expect this to produce a good result .. but it doesn't
vec3 bad_hash(vec3 Input)
{
  uint PrimeX = 501125321u;
  uint PrimeY = 1136930381u;
  uint PrimeZ = 1720413743u;

  uint xI = uint(floatBitsToInt(Input.x)) * PrimeX;
  uint yI = uint(floatBitsToInt(Input.y)) * PrimeY;
  uint zI = uint(floatBitsToInt(Input.z)) * PrimeZ;

  uint xH = ChrisWellonsIntegerHash_lowbias32(xI);
  uint yH = ChrisWellonsIntegerHash_lowbias32(yI);
  uint zH = ChrisWellonsIntegerHash_lowbias32(zI);

  uint X = ChrisWellonsIntegerHash_lowbias32(xH ^ yH) & 0xFFFFu;
  uint Y = ChrisWellonsIntegerHash_lowbias32(xH ^ zH) & 0xFFFFu;
  uint Z = ChrisWellonsIntegerHash_lowbias32(yH ^ zH) & 0xFFFFu;

  return fract(vec3(f32(X)/f32(0xFFFF), f32(Y)/f32(0xFFFF), f32(Z)/f32(0xFFFF)));
}

vec3 hash3( vec3 p )
{
  // procedural white noise	
	return fract(sin(
        vec3(
          dot(p,vec3(127.1,311.7,531.9)),
          dot(p,vec3(722.1,117.7,438.9)),
          dot(p,vec3(269.5,183.3,22.5))))*43758.5453);
}

float hashf( float f )
{
  return -1.0 + 2.0*fract(sin(f)*43758.5453123);
}

// https://github.com/scratchapixel/code/blob/ce4fc22659db55a92c094373dc306ac3e261601b/perlin-noise-part-2/perlinnoise.cpp#L94
link_internal f32
CubicInterpolate(f32 N)
{
  f32 Result = N * N * (3.f - 2.f * N);
  return Result;
}

link_internal f32
Smoothstep(f32 N)
{
  return CubicInterpolate(N);
}

link_internal f32
QuinticInterpolate(f32 N)
{
  f32 u = N*N*N*(N*(N*6.0-15.0)+10.0);
  return u;
}

float
CosineInterpolate( float t, f32 y1, f32 y2 )
{
  float t2 = (1.f-cos(t*PI32))/2.f;
  float nt2 = (1.f-t2); 

  f32 a = y1*nt2;
  f32 b = y2*t2;
  return(a+b);
}


float white_noise(v2 P)
{
  float Res = hashf(P.x);
        Res = hashf(P.y + Res);
  return Res;
}

float white_noise(v3 P)
{
  float Res = hashf(P.x);
        Res = hashf(P.y + Res);
        Res = hashf(P.z + Res);
  return Res;
}

link_internal v3
voronoi_noise(v3 Texel, f32 Squareness)
{
  v3 baseCell = Floor(Texel);

  v3 CellOffsets[27];

  // first pass to find the closest cell
  //
  f32 minDistToCellSq = 100;
  v3 toClosestCell;
  v3 closestCell;
  s32 CellIndex = 0;
  for( s32 x1 = -1;
           x1 <= 1;
         ++x1 )
  {
    for(s32 y1 = -1;
            y1 <= 1;
          ++y1 )
    {
      for( s32 z1 = -1;
               z1 <= 1;
             ++z1 )
      {
        v3 cell      =  baseCell + V3(x1, y1, z1);
        v3 offset    = Clamp01(RandomV3FromV3(cell) - Squareness);
        v3 cellPosition = cell + offset;

        v3 toCell = cellPosition - Texel;
        f32 distToCellSq = LengthSq(toCell);
        if(distToCellSq < minDistToCellSq)
        {
          minDistToCellSq = distToCellSq;
          closestCell = cell;
          toClosestCell = toCell;
        }

        CellOffsets[CellIndex++] = offset;
      }
    }
  }
  //
  // TODO(Jesse): This seems like you'd just want to do it in-line in the first
  // loop ..?
  //
  // second pass to find the distance to the closest edge
  //
  f32 minEdgeDistance = 10;
  CellIndex = 0;
  for( s32 x2 = -1;
           x2 <= 1;
         ++x2 )
  {
    for(s32 y2 = -1;
            y2 <= 1;
          ++y2 )
    {
      for( s32 z2 = -1;
               z2 <= 1;
             ++z2 )
      {
        /* v3 cell = (baseCell + V3(x2, y2, z2)); */
        v3 cell = baseCell + V3(x2, y2, z2);
        v3 offset = CellOffsets[CellIndex++];

        v3 cellPosition = cell + offset;
        v3 toCell = cellPosition - Texel;

        v3 diffToClosestCell = Abs(closestCell - cell);
        b32 isClosestCell = diffToClosestCell.x + diffToClosestCell.y + diffToClosestCell.z < 0.1f;
        if(isClosestCell == False)
        {
          v3 toCenter = (toClosestCell + toCell) * 0.5;
          v3 cellDifference = Normalize(toCell - toClosestCell);
          f32 edgeDistance = Dot(toCenter, cellDifference);
          minEdgeDistance = Min(minEdgeDistance, edgeDistance);
        }
      }
    }
  }

  return V3(minEdgeDistance, minDistToCellSq, closestCell);
}

link_internal v3
voronoi_noise(v3 Texel)
{
  return voronoi_noise(Texel, 1.f);
}


vec4 value_noise_derivs( in vec3 x )
{
    vec3 p = floor(x);
    vec3 w = fract(x);
    vec3 u = w*w*(3.0-2.0*w);
    vec3 du = 6.0*w*(1.0-w);
    
    float n = p.x + p.y*157.0 + 113.0*p.z;
    
    float a = hashf(n+  0.0);
    float b = hashf(n+  1.0);
    float c = hashf(n+157.0);
    float d = hashf(n+158.0);
    float e = hashf(n+113.0);
    float f = hashf(n+114.0);
    float g = hashf(n+270.0);
    float h = hashf(n+271.0);
	
    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    return vec4( k0 + k1*u.x + k2*u.y + k3*u.z + k4*u.x*u.y + k5*u.y*u.z + k6*u.z*u.x + k7*u.x*u.y*u.z, 
                 du * (vec3(k1,k2,k3) + u.yzx*vec3(k4,k5,k6) + u.zxy*vec3(k6,k4,k5) + k7*u.yzx*u.zxy ));
}

// return value noise (in x) and its derivatives (in yzw)
//
// return (-1, 1)
//
vec4 gradient_noise_derivs( in vec3 x )
{
  // grid
  ivec3 i = ivec3(floor(x));
  vec3 f = fract(x);
  
  #if INTERPOLANT==1
  // quintic interpolant
  vec3 u = f*f*f*(f*(f*6.0-15.0)+10.0);
  vec3 du = 30.0*f*f*(f*(f-2.0)+1.0);
  #else
  // cubic interpolant
  vec3 u = f*f*(3.0-2.0*f);
  vec3 du = 6.0*f*(1.0-f);
  #endif    
  
  // gradients
  vec3 ga = ivhash( i+ivec3(0,0,0) );
  vec3 gb = ivhash( i+ivec3(1,0,0) );
  vec3 gc = ivhash( i+ivec3(0,1,0) );
  vec3 gd = ivhash( i+ivec3(1,1,0) );
  vec3 ge = ivhash( i+ivec3(0,0,1) );
  vec3 gf = ivhash( i+ivec3(1,0,1) );
  vec3 gg = ivhash( i+ivec3(0,1,1) );
  vec3 gh = ivhash( i+ivec3(1,1,1) );

  // projections
  float va = dot( ga, f-vec3(0.0,0.0,0.0) );
  float vb = dot( gb, f-vec3(1.0,0.0,0.0) );
  float vc = dot( gc, f-vec3(0.0,1.0,0.0) );
  float vd = dot( gd, f-vec3(1.0,1.0,0.0) );
  float ve = dot( ge, f-vec3(0.0,0.0,1.0) );
  float vf = dot( gf, f-vec3(1.0,0.0,1.0) );
  float vg = dot( gg, f-vec3(0.0,1.0,1.0) );
  float vh = dot( gh, f-vec3(1.0,1.0,1.0) );

  // interpolations
  return vec4( va + u.x*(vb-va) + u.y*(vc-va) + u.z*(ve-va) + u.x*u.y*(va-vb-vc+vd) + u.y*u.z*(va-vc-ve+vg) + u.z*u.x*(va-vb-ve+vf) + (-va+vb+vc-vd+ve-vf-vg+vh)*u.x*u.y*u.z,    // value
               ga + u.x*(gb-ga) + u.y*(gc-ga) + u.z*(ge-ga) + u.x*u.y*(ga-gb-gc+gd) + u.y*u.z*(ga-gc-ge+gg) + u.z*u.x*(ga-gb-ge+gf) + (-ga+gb+gc-gd+ge-gf-gg+gh)*u.x*u.y*u.z +   // derivatives
               du * (vec3(vb,vc,ve) - va + u.yzx*vec3(va-vb-vc+vd,va-vc-ve+vg,va-vb-ve+vf) + u.zxy*vec3(va-vb-ve+vf,va-vb-vc+vd,va-vc-ve+vg) + u.yzx*u.zxy*(-va+vb+vc-vd+ve-vf-vg+vh) ));
}

// return (0, 1)
//
f32 billow_noise( in vec3 x, int octaves)
{
  f32 Result = 0;
  for (int i = 0; i < octaves; ++i)
  {
    f32 V = abs(gradient_noise_derivs(x).x);
    Result = max(V, Result);
    x /= 2.f;
  }
  return Result;
}

// return (0, 1)
//
f32 billow_noise( in vec3 x )
{
  f32 V = gradient_noise_derivs(x).x;
  f32 Result = Abs(V);
  return Result;
}

// return (0, 1)
//
f32 ridge_noise( in vec3 x )
{
  f32 V = gradient_noise_derivs(x).x;
  f32 Result = (Abs(V)*-1.f) + 1.f;
  return Result;
}

f32 RemapSample(in f32 SampleX, in v2 Points[16], in u32 PointCount)
{
  f32 Result = -1.f;
  v2 PrevP = Points[0];

  for (u32 PointIndex = 1u; PointIndex < PointCount; ++PointIndex)
  {
    v2 NextP = Points[PointIndex];

    if (SampleX >= PrevP.x && SampleX < NextP.x)
    {


      r32 Range = PrevP.x - NextP.x;
      r32 t = Clamp01((SampleX-NextP.x) / Range);

      Result = CosineInterpolate(t, NextP.y, PrevP.y);

      /* Result = mix(NextP.y, PrevP.y, t); */

      /* Result = QuinticInterpolate(Result); */
      /* Result = QuinticInterpolate(Result); */

      /* Result = CubicInterpolate(Result); */
      /* Result = Smoothstep(Result); */
      /* Result = Smoothstep(Result); */

      break;
    }

    PrevP = NextP;
  }

  // TODO(Jesse): do we do this?  I think this should always be true ..
  //
  // I'm going to leave it out till I find a good reason.
  //
  /* Result = abs(min(Result, 1.0)); */
  return Result;
}


// return unit vector in range (-1, 1)
//
v3 ComputeNormal(sampler2D InputTex, v2 FragCoord, ivec2 InputTexDim, ivec2 OutputTexDim, f32 ChunkResolutionZ)
{
  v2i InputTexOffset = (InputTexDim - OutputTexDim)/2;
  /* v2i InputTexOffset = v2i(1); */

  /* s32 x = s32(floor(FragCoord.x)); */
  s32 z = s32(floor(FragCoord.y / OutputTexDim.x));
  s32 y = s32(floor(FragCoord.y - (z*OutputTexDim.y)));

  ivec2 InputTexCoord = InputTexOffset + ivec2( FragCoord.x,
                                                 y +
                                                (z * InputTexDim.y) + (InputTexOffset.y*InputTexDim.y) );
  /* ivec2 InputTexCoord = ivec2(FragCoord.x + 1, (y + 1) + (z * InputTexDim.y) + InputTexDim.y ); */
  f32 CurrentNoiseValue = texelFetch(InputTex, InputTexCoord, 0).a;

  v3 Normal = v3(0.f);

  for ( s32 dz = -1; dz < 2; ++ dz)
  for ( s32 dy = -1; dy < 2; ++ dy)
  for ( s32 dx = -1; dx < 2; ++ dx)
  {
    if (dz == 0 && dy == 0 && dx == 0) continue; // Skip the middle-most voxel

      s32 xCoord = InputTexCoord.x + dx;
      s32 yCoord = InputTexCoord.y + dy;
      s32 zCoord =                   dz * InputTexDim.y;

      f32 Next = texelFetch(InputTex, ivec2(xCoord, yCoord+zCoord), 0).a;
      f32 Diff = Next-(dz*ChunkResolutionZ) - Truncate(CurrentNoiseValue);
      // NOTE(Jesse): Can't do this because we end up with complete nonsense on
      // the edges for some unknown reason
      /* if (Diff > 0) */

      // TODO(Jesse): Recompute with a small random variance to the weight if this is 0?
      // EDIT(Jesse): Why exactly would we do this?? Sounds like a bad idea.. ?
      Normal += V3(dx,dy,dz)*Diff;
  }

  // Invert because we accumulate the value by pointing 'at' the cell we're checking
  //
  v3 Result = -1.f * Normalize(Normal);
  return Result;
}

