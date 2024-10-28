// NOTE(Jesse): These get switched between at startup
/* #version 310 es */
/* #version 330 core */

#extension GL_NV_shader_buffer_load : enable


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

#define f32_MAX (1E+37f)
#define f32_MIN (1E-37f)

#define v4 vec4
#define v3 vec3
#define v2 vec2

#define V4 vec4
#define V3 vec3
#define V2 vec2

#define r32 float
#define f32 float
#define u32 unsigned int
#define s32 int

#define link_internal

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
  /* if (NonlinearDepth < 0.01f) */
  {
    f32 ClipZ = (NonlinearDepth*2.f) - 1.f;
    v2 ClipXY = (ScreenUV*2.f)-1.f;

    v4 ClipP = V4(ClipXY.x, ClipXY.y, ClipZ, 1.f); // Correct
    v4 ViewP = InverseProjectionMatrix * ClipP;

    ViewP /= ViewP.w;

    WorldP = InverseViewMatrix * ViewP;
  }
  return WorldP.xyz;
}

link_internal v3
UnpackHSVColor(s32 Packed)
{
  s32 FiveBits = 31;
  s32 SixBits   = 63;

  r32 H = ((Packed >> 10) & SixBits) / r32(SixBits);
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
  v3 HSV = UnpackHSVColor(Packed);
  v3 Result = HSVtoRGB(HSV);
  return Result;
}

uint PackRGB(v3 Color)
{
  uint FiveBits    = (1u << 5) - 1u;
  uint FifteenBits = (1u << 15) - 1u;
  uint R = uint(Color.r * f32(FiveBits)) & FiveBits;
  uint G = uint(Color.g * r32(FiveBits)) & FiveBits;
  uint B = uint(Color.b * r32(FiveBits)) & FiveBits;
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

vec3 voronoi_noise( in vec3 x )
{
  vec3 ip = floor(x);
  vec3 fp = fract(x);

  //----------------------------------
  // first pass: regular voronoi
  //----------------------------------
  vec3 mg, mr;

  float md = 8.0;
  for( int k=-1; k<=1; k++ )
  for( int j=-1; j<=1; j++ )
  for( int i=-1; i<=1; i++ )
  {
    vec3 g = vec3(float(i),float(j),float(k));
    vec3 o = hash3( ip + g );
    vec3 r = g + o - fp;
    float d = dot(r,r);

      if( d<md )
      {
          md = d;
          mr = r;
          mg = g;
      }
  }

  //----------------------------------
  // second pass: distance to borders
  //----------------------------------
  md = 8.0;
  for( int k=-1; k<=1; k++ )
  for( int j=-1; j<=1; j++ )
  for( int i=-1; i<=1; i++ )
  {
    vec3 g = mg + vec3(float(i),float(j),float(k));
    vec3 o = hash3( ip + g );
    vec3 r = g + o - fp;

    if( dot(mr-r,mr-r)>0.00001 )
    md = min( md, dot( 0.5*(mr+r), normalize(r-mr) ) );
  }

  return vec3( md, mr );
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
