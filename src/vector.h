/* #if BONSAI_EMCC */
/* #include <wasm_simd128.h> */
/* typedef v128_t __m128; */
/* #else */
/* #endif */

union v2i
{
  s32 E[2];
  struct { s32 x; s32 y; };
};

union v2
{
  r32 E[2];
  struct { r32 x; r32 y; };
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
union v3i
{
  s32 E[3];

  struct { s32 x; s32 y; s32 z; };
  struct { s32 r; s32 g; s32 b; };

  struct {
    v2i xy;
    s32 Ignored0_;
  };

  struct {
    s32 Ignored1_;
    v2i yz;
  };
};

union v3_u8
{
  u8 E[3];
  struct { u8 x; u8 y; u8 z; };
  struct { u8 r; u8 g; u8 b; };
};
CAssert(sizeof(v3_u8) == 3);

union v3
{
  r32 E[3];

  struct { r32 x; r32 y; r32 z; };
  struct { r32 r; r32 g; r32 b; };
  struct { r32 h; r32 s; r32 v; };

  struct {
    v2 xy;
    r32 Ignored0_;
  };

  struct {
    r32 Ignored1_;
    v2 yz;
  };
};


// Note: OpenGL matrices have x first
union v4
{
  r32 E[4];

  struct { r32 x; r32 y; r32 z; r32 w; };
  struct { r32 r; r32 g; r32 b; r32 a; };

  struct { r32 Left; r32 Top; r32 Right; r32 Bottom; };

  struct {
    v2 xy;
    r32 Ignored0_;
    r32 Ignored01_;
  };

  struct {
    v2 Ignored1_;
    v2 zw;
  };


  struct {
    v3 xyz;
    r32 Ignored03_;
  };

  struct {
    v3 rgb;
    r32 Ignored04_;
  };

  r32&
  operator[](s32 index)
  {
    r32& Result = this->E[index];
    return Result;
  }
};

union v4i
{
  s32 E[4];

  struct { s32 x; s32 y; s32 z; s32 w; };
  struct { s32 r; s32 g; s32 b; s32 a; };

  // NOTE(Jesse): This is arranged so the tensor layout NCHW (batch, channel,
  // height, width) accessors line up with their sensible counterparts in x, y, z, w
  /* struct { s32 W; s32 H; s32 C; s32 N; }; */

  struct { s32 Left; s32 Top; s32 Right; s32 Bottom; };

  struct {
    v2i xy;
    s32 Ignored0_;
    s32 Ignored01_;
  };

  struct {
    v2i Ignored1_;
    v2i zw;
  };


  struct {
    v3i xyz;
    s32 Ignored03_;
  };

  struct {
    v3i rgb;
    s32 Ignored04_;
  };

  s32&
  operator[](s32 index)
  {
    s32& Result = this->E[index];
    return Result;
  }
};


link_internal v4i
V4i(s32 x, s32 y, s32 z, s32 w)
{
  v4i Result = {{ x, y, z, w, }};
  return Result;
}

link_internal v4
V4(f32 x, v3 v)
{
  v4 Result = {{ x, v.x, v.y, v.z }};
  return Result;
}

link_internal v4
V4(v3 v, f32 w)
{
  v4 Result = {{ v.x, v.y, v.z, w }};
  return Result;
}

link_internal v4
V4(r32 Fill)
{
  v4 Result = {{ Fill, Fill, Fill, Fill }};
  return Result;
}

link_internal v4
V4( f32 x, f32 y, f32 z, f32 w)
{
  v4 Result = {{ x, y, z, w, }};
  return Result;
}


#pragma GCC diagnostic pop


typedef v3i voxel_position;
typedef v3i chunk_dimension;
typedef v3i world_position;

inline voxel_position
Voxel_Position(v3 Offset)
{
  voxel_position Result;

  Result.x = (s32)Offset.x;
  Result.y = (s32)Offset.y;
  Result.z = (s32)Offset.z;

  return Result;
}



poof(gen_vector_operators(v2))
#include <generated/gen_vector_operators_v2.h>

poof(gen_vector_operators(v2i))
#include <generated/gen_vector_operators_v2i.h>

poof(gen_vector_operators(v3))
#include <generated/gen_vector_operators_v3.h>

poof(gen_vector_operators(v3i))
#include <generated/gen_vector_operators_v3i.h>

poof(gen_vector_operators(v3_u8))
#include <generated/gen_vector_operators_v3_u8.h>

poof(gen_vector_operators(v4))
#include <generated/gen_vector_operators_v4.h>

poof(gen_hetero_vector_operators(v2, v2i))
#include <generated/gen_hetero_vector_operators_v2_v2i.h>

poof(gen_hetero_vector_operators(v2i, v2))
#include <generated/gen_hetero_vector_operators_v2i_v2.h>

poof(gen_hetero_vector_operators(v3, v3i))
#include <generated/gen_hetero_vector_operators_v3_v3i.h>

poof(gen_hetero_vector_operators(v3i, v3))
#include <generated/gen_hetero_vector_operators_v3i_v3.h>

poof(gen_hetero_vector_operators(v3_u8, v3i))
#include <generated/gen_hetero_vector_operators_v3_u8_v3i.h>

poof(gen_hetero_vector_operators(v3_u8, v3))
#include <generated/gen_hetero_vector_operators_v3_u8_v3.h>

inline v3_u8
V3U8(v3_u8 V)
{
  return V;
}

inline v3_u8
V3U8(v3 V)
{
  v3_u8 Result = {{ u8(V.x), u8(V.y), u8(V.z) }};
  return Result;
}

inline v3_u8
V3U8(v3i V)
{
  v3_u8 Result = {{ u8(V.x), u8(V.y), u8(V.z) }};
  return Result;
}



inline v3i
operator~(v3i P)
{
  v3i Result = {{ ~P.x, ~P.y, ~P.z }};
  return Result;
}

inline v3
operator*(voxel_position P1, f32 f)
{
  v3 Result;
  Result.x = (r32)P1.x * f;
  Result.y = (r32)P1.y * f;
  Result.z = (r32)P1.z * f;
  return Result;
}


inline v3i
V3i(v3 V)
{
  v3i Result = {{ (s32)V.x, (s32)V.y, (s32)V.z }};
  return Result;
}

inline v3i
V3i(v2i XY, s32 Z)
{
  v3i Result = {{ XY.x, XY.y, Z}};
  return Result;
}

inline v3i
V3i(f32 Flood)
{
  v3i Result = {{ s32(Flood), s32(Flood), s32(Flood) }};
  return Result;
}

inline v3i
V3i(s32 Flood)
{
  v3i Result = {{ Flood, Flood, Flood }};
  return Result;
}

inline v3i
V3i(s32 X, s32 Y, s32 Z)
{
  v3i Result = {{ X, Y, Z }};
  return Result;
}

inline v3
V3(v3 I)
{
  return I;
}

inline v3
V3(v3_u8 V)
{
  return {{f32(V.x), f32(V.y), f32(V.z)}};
}

inline v3
V3(r32 I)
{
  v3 Result;
  Result.x = I;
  Result.y = I;
  Result.z = I;
  return Result;
}

inline v3
V3(s32 I)
{
  v3 Result;
  Result.x = (r32)I;
  Result.y = (r32)I;
  Result.z = (r32)I;
  return Result;
}

inline v3
V3(v3i wp)
{
  v3 Result;
  Result.x = (f32)wp.x;
  Result.y = (f32)wp.y;
  Result.z = (f32)wp.z;
  return Result;
}

inline v3
V3(s32 x, s32 y, s32 z)
{
  v3 Result = {};
  Result.x = (f32)x;
  Result.y = (f32)y;
  Result.z = (f32)z;
  return Result;
}

inline v3
V3(v2 XY, f32 z)
{
  v3 Result = {{ XY.x, XY.y, z }};
  return Result;
}

inline v3
V3(v2i XY, s32 z)
{
  v3 Result = {{ f32(XY.x), f32(XY.y), f32(z) }};
  return Result;
}

inline v3
V3(f32 x, f32 y, f32 z)
{
  v3 Result = {};
  Result.x = x;
  Result.y = y;
  Result.z = z;
  return Result;
}

inline world_position
World_Position(v3 V)
{
  voxel_position Result;
  Result.x = (s32)V.x;
  Result.y = (s32)V.y;
  Result.z = (s32)V.z;
  return Result;
}

inline voxel_position
Voxel_Position(s32 P)
{
  voxel_position Result;
  Result.x = P;
  Result.y = P;
  Result.z = P;
  return Result;
}

inline voxel_position
Voxel_Position(s32 x, s32 y, s32 z)
{
  voxel_position Result;
  Result.x = x;
  Result.y = y;
  Result.z = z;
  return Result;
}

inline world_position
World_Position(s32 P)
{
  chunk_dimension Result = Voxel_Position(P);
  return Result;
}

inline world_position
World_Position(s32 x, s32 y, s32 z)
{
  chunk_dimension Result = Voxel_Position(x,y,z);
  return Result;
}


inline chunk_dimension
Chunk_Dimension(s32 flood)
{
  chunk_dimension Result = Voxel_Position(flood,flood,flood);
  return Result;
}

inline chunk_dimension
Chunk_Dimension(s32 x, s32 y, s32 z)
{
  chunk_dimension Result = Voxel_Position(x,y,z);
  return Result;
}

link_internal void
Truncate(v3 *Input)
{
  *Input = V3(V3i(*Input));
}

v3
Truncate(v3 Input)
{
  v3 Result = V3(V3i(Input));
  return Result;
}



inline s32
Volume(v2i Dim)
{
  Assert(Dim.x >= 0);
  Assert(Dim.y >= 0);

  Dim.x = Max(Dim.x, 0);
  Dim.y = Max(Dim.y, 0);

  s32 Result = (s32)(Dim.x*Dim.y);
  Assert(Result > 0);
  return Result;
}

inline s32
Volume(v4 Dim)
{
  Assert(Dim.x >= 0.f);
  Assert(Dim.y >= 0.f);
  Assert(Dim.z >= 0.f);
  Assert(Dim.w >= 0.f);

  /* Dim.x = Max(Dim.x, 0.f); */
  /* Dim.y = Max(Dim.y, 0.f); */
  /* Dim.z = Max(Dim.z, 0.f); */
  /* Dim.w = Max(Dim.w, 0.f); */

  s32 Result = (s32)(Dim.x*Dim.y*Dim.z*Dim.w);
  Assert(Result > 0);
  return Result;
}

inline s32
Volume(v4i Dim)
{
  Assert(Dim.x >= 0);
  Assert(Dim.y >= 0);
  Assert(Dim.z >= 0);
  Assert(Dim.w >= 0);

  /* Dim.x = Max(Dim.x, 0); */
  /* Dim.y = Max(Dim.y, 0); */
  /* Dim.z = Max(Dim.z, 0); */
  /* Dim.w = Max(Dim.w, 0); */
  s32 Result = (s32)(Dim.x*Dim.y*Dim.z*Dim.w);
  Assert(Result > 0);
  return Result;
}

inline r32
Volume(v3 Dim)
{
  Assert(Dim.x >= 0);
  Assert(Dim.y >= 0);
  Assert(Dim.z >= 0);

  /* Dim.x = Max(Dim.x, 0.f); */
  /* Dim.y = Max(Dim.y, 0.f); */
  /* Dim.z = Max(Dim.z, 0.f); */

  r32 Result = (Dim.x*Dim.y*Dim.z);
  Assert(Result > 0.f);
  return Result;
}

inline s32
Volume(v3i Dim)
{
  Assert(Dim.x >= 0);
  Assert(Dim.y >= 0);
  Assert(Dim.z >= 0);

  /* Dim.x = Max(Dim.x, 0); */
  /* Dim.y = Max(Dim.y, 0); */
  /* Dim.z = Max(Dim.z, 0); */

  /* Dim.x = Max(Dim.x, 0); */
  /* Dim.y = Max(Dim.y, 0); */
  /* Dim.z = Max(Dim.z, 0); */
  return (Dim.x*Dim.y*Dim.z);
}

inline v2
V2(v2i V)
{
  v2 Result;
  Result.x = (r32)V.x;
  Result.y = (r32)V.y;
  return Result;
}

inline v2
V2(s32 F)
{
  v2 Result = {{(r32)F, (r32)F}};
  return Result;
}

inline v2
V2(r32 F)
{
  v2 Result = {{F, F}};
  return Result;
}

v2
V2(f32 x,f32 y)
{
  v2 Result = {{x,y}};
  return Result;
}

v2i
V2i(v2 V)
{
  v2i Result;
  Result.x = (s32)V.x;
  Result.y = (s32)V.y;
  return Result;
}

v2i
V2i(s32 P)
{
  v2i Result = {{P,P}};
  return Result;
}

v2i
V2i(s32 x, s32 y)
{
  v2i Result = {{x,y}};
  return Result;
}

v2i
V2i(u32 x, u32 y)
{
  v2i Result = V2i((s32)x, (s32)y);
  return Result;
}

inline v3i
operator%(v3i A, v3i B)
{
  v3i Result;
  Result.x = A.x % B.x;
  Result.y = A.y % B.y;
  Result.z = A.z % B.z;
  return Result;
}

inline v3
operator/(v3 A, s32 B)
{
  v3 Result;
  Result.x = A.x / (f32)B;
  Result.y = A.y / (f32)B;
  Result.z = A.z / (f32)B;
  return Result;
}

inline v3
operator/(voxel_position A, r32 f)
{
  v3 Result;
  Result.x = (r32)A.x / f;
  Result.y = (r32)A.y / f;
  Result.z = (r32)A.z / f;
  return Result;
}


union f32_reg {
  r32 F[4];
  __m128 Sse;
};



// TODO(Jesse): Now that we're generating operators, it's pretty easy for us to
// do performance experiments by converting all operators to SIMD in one fell
// swoop..
#if 0
#define SIMD_OPERATORS 1
#define SANITY_CHECK_SIMD_OPERATORS 1
inline v3
operator*(v3 A, v3 B)
{

#if SIMD_OPERATORS
  __m128 Vec1 = _mm_set_ps(0, A.z, A.y, A.x);
  __m128 Vec2 = _mm_set_ps(0, B.z, B.y, B.x);

  f32_reg Res;
  Res.Sse = _mm_mul_ps(Vec1, Vec2);

  v3 Result = {{ Res.F[0], Res.F[1], Res.F[2] }};

#if SANITY_CHECK_SIMD_OPERATORS
  v3 Sanity;
  Sanity.x = A.x * B.x;
  Sanity.y = A.y * B.y;
  Sanity.z = A.z * B.z;
  Assert(Sanity.x == Result.x);
  Assert(Sanity.y == Result.y);
  Assert(Sanity.z == Result.z);
#endif

  return Result;

#else
  v3 Result;

  Result.x = A.x * B.x;
  Result.y = A.y * B.y;
  Result.z = A.z * B.z;

  return Result;
#endif

}
#endif

v3 operator+(v3 A, v2 B)
{
  v3 Result = {{ A.x+B.x, A.y+B.y, A.z }};
  return Result;
}

v3 &
operator+=(v3 &A, v2 B)
{
  A = A + B;
  return A;
}


v4
operator*(v4 A, s32 B)
{
  v4 Result = {};
  Result.x = A.x * (f32)B;
  Result.y = A.y * (f32)B;
  Result.z = A.z * (f32)B;
  Result.w = A.w * (f32)B;
  return Result;
}

inline v4
operator*=(v4 A, s32 B)
{
  A = A * B;
  return A;
}

inline v3
operator^(v3 P1, v3 P2)
{
  v3 Result;
  Result.x = (r32)((s32)P1.x ^ (s32)P2.x);
  Result.y = (r32)((s32)P1.y ^ (s32)P2.y);
  Result.z = (r32)((s32)P1.z ^ (s32)P2.z);
  return Result;
}

inline v3
operator%(v3 A, v3 i)
{
  v3 Result;
  Result.x = Mod(A.x, i.x);
  Result.y = Mod(A.y, i.y);
  Result.z = Mod(A.z, i.z);
  return Result;
}

inline v3
operator%(v3 A, s32 i)
{
  v3 Result;
  Result.x = (f32)((s32)A.x % i);
  Result.y = (f32)((s32)A.y % i);
  Result.z = (f32)((s32)A.z % i);
  return Result;
}

inline v3i
operator%(v3i A, s32 i)
{
  v3i Result;
  Result.x = ((s32)A.x % i);
  Result.y = ((s32)A.y % i);
  Result.z = ((s32)A.z % i);
  return Result;
}

poof(gen_vector_infix_operator(v3i, {^}))
#include <generated/gen_vector_infix_operator_v3i_688856449.h>

poof(gen_vector_infix_operator(v3i, {&}))
#include <generated/gen_vector_infix_operator_v3i_688856393.h>

poof(vector_interpolation_functions(v2))
#include <generated/gen_lerp_v2.h>

poof(vector_interpolation_functions(v3))
#include <generated/gen_lerp_v3.h>

// TODO(Jesse): The heck do we use this for?
// UPDATE(Jesse): Apparently nothing..
/* poof(vector_interpolation_functions(v4)) */
/* #include <generated/gen_lerp_v4.h> */

poof(gen_vector_area(v2))
#include <generated/gen_vector_area_v2.h>

poof(gen_vector_area(v2i))
#include <generated/gen_vector_area_v2i.h>

poof(gen_common_vector(v2))
#include <generated/gen_common_vector_v2.h>

poof(gen_common_vector(v3))
#include <generated/gen_common_vector_v3.h>

poof(gen_common_vector(v3i))
#include <generated/gen_common_vector_v3i.h>

poof(gen_vector_normalize(v2))
#include <generated/gen_vector_normalize_funcs_v2.h>

poof(gen_vector_normalize(v3))
#include <generated/gen_vector_normalize_funcs_v3.h>


inline v3
Ceil(v3 Vec)
{
  v3 Result = {{ Ceilf(Vec.x), Ceilf(Vec.y), Ceilf(Vec.z) }};
  return Result;
}

inline r32
Distance( v3 P1, v3 P2 )
{
  r32 Result = Length(P1-P2);
  return Result;
}

inline r32
DistanceSq( v3 P1, v3 P2 )
{
  r32 Result = LengthSq(P1-P2);
  return Result;
}


inline v3
Round( v3 A )
{
  v3 Result = {{
    Round(A.E[0]),
    Round(A.E[1]),
    Round(A.E[2]),
  }};
  return Result;
}

inline r32
Dot( v3 A, v3 B)
{
  r32 Result;
  Result = (A.x*B.x) + (A.y*B.y) + (A.z*B.z);
  return Result;
}

inline v3
Cross( voxel_position A, voxel_position B )
{
  v3 Result = {{
    (r32)(A.y*B.z)-(r32)(A.z*B.y),
    (r32)(A.z*B.x)-(r32)(A.x*B.z),
    (r32)(A.x*B.y)-(r32)(A.y*B.x)
  }};

  return Result;
}

inline v3
Cross( v3 A, v3 B )
{
  v3 Result = {{
    (A.y*B.z)-(A.z*B.y),
    (A.z*B.x)-(A.x*B.z),
    (A.x*B.y)-(A.y*B.x)
  }};

  return Result;
}

inline v3
Perp( v3 A )
{
  v3 B = V3(0.f, 0.f, 1.f);
  if (A == B)
  {
    B = V3(1.f, 0.f, 0.f);
  }

  v3 Result = Cross(A, B);
  return Result;
}


inline v3
RoundToMultiple(v3 N, v3i Thresh)
{
  v3i Ni = V3i(N);
  v3i Rem = Ni % Thresh;
  v3 Result = V3(Ni - Rem);
  return Result;
}

inline v3
RoundToMultiple(v3 N, s32 Thresh)
{
  v3i Ni = V3i(N);
  v3i Rem = Ni % Thresh;
  v3 Result = V3(Ni - Rem);
  return Result;
}


inline v3
SafeDivide0(v3 Dividend, r32 Divisor)
{
  v3 Result = {};

  if (Divisor != 0.0f)
    Result = Dividend/Divisor;

  return Result;
}

inline v3
SafeDivide0(v3 Dividend, v3 Divisor)
{
  v3 Result = {};

  if (Divisor.E[0] != 0.0f) { Result.E[0] = Dividend.E[0]/Divisor.E[0]; }
  if (Divisor.E[1] != 0.0f) { Result.E[1] = Dividend.E[1]/Divisor.E[1]; }
  if (Divisor.E[2] != 0.0f) { Result.E[2] = Dividend.E[2]/Divisor.E[2]; }

  return Result;
}

b32
PointsAreWithinDistance(v3 P1, v3 P2, f32 Thresh)
{
  v3 P1toP2 = P2-P1;
  f32 LenSq = LengthSq(P1toP2);

  b32 Result = LenSq < Square(Thresh);
  return Result;
}

inline v3
SafeDivide(v3 Dividend, r32 Divisor)
{
  v3 Result = Dividend;

  if (Divisor != 0.0f)
    Result = Dividend/Divisor;

  return Result;
}

inline voxel_position
ClampMinus1toInfinity( voxel_position V )
{
  voxel_position Result = V;

  if ( V.x < 0 )
    Result.x = -1;

  if ( V.y < 0 )
    Result.y = -1;

  if ( V.z < 0 )
    Result.z = -1;

  return Result;
}

inline v3i
GetSigni(v3 P)
{
  v3i Result = V3i( GetSign(P.x), GetSign(P.y), GetSign(P.z));
  return Result;
}

inline s32
GetIndexUnsafe(voxel_position P, chunk_dimension Dim)
{
  s32 i =
    (P.x) +
    (P.y*Dim.x) +
    (P.z*Dim.x*Dim.y);

  return i;
}

inline s32
TryGetIndex(s32 xIndex, s32 yIndex, s32 zIndex, v3i Dim)
{
  s32 Result = -1;
  if (xIndex >= 0 && yIndex >= 0 && zIndex >= 0 &&
      xIndex < Dim.x && yIndex < Dim.y && zIndex < Dim.z)
  {
    Result = xIndex + (yIndex*Dim.x) + (zIndex*Dim.x*Dim.y);
  }

  Assert(Result < Volume(Dim));
  return Result;
}

inline s32
TryGetIndex(s32 xIndex, s32 yIndex, v2i Dim)
{
  s32 Result = -1;
  if (xIndex >= 0 && yIndex >= 0 &&
      xIndex < Dim.x && yIndex < Dim.y)
  {
    Result = xIndex + (yIndex*Dim.x);
  }

  Assert(Result < Volume(Dim));
  return Result;
}

inline s32
TryGetIndex(v3i P, v3i Dim)
{
  s32 Result = TryGetIndex(P.x, P.y, P.z, Dim);
  return Result;
}

inline s32
TryGetIndex(v2i P, v2i Dim)
{
  s32 Result = TryGetIndex(P.x, P.y, Dim);
  return Result;
}

inline s32
GetIndex(s32 X, s32 Y, v2i Dim)
{
  Assert(X >= 0);
  Assert(Y >= 0);

  Assert(X < Dim.x);
  Assert(Y < Dim.y);

  s32 Result = X + (Y*Dim.x);

  Assert(Result >= 0);
  Assert(Result < Volume(Dim));

  return Result;
}

inline s32
GetIndex(s32 X, s32 Y, s32 Z, chunk_dimension Dim)
{
  /* Assert(X >= 0); */
  /* Assert(Y >= 0); */
  /* Assert(Z >= 0); */

  /* Assert(X < Dim.x); */
  /* Assert(Y < Dim.y); */
  /* Assert(Z < Dim.z); */

  s32 Result = X +
              (Y*Dim.x) +
              (Z*Dim.x*Dim.y);

  Assert(Result >= 0);
  Assert(Result < Volume(Dim));

  return Result;
}

inline s32
GetIndex(v2i P, v2i Dim)
{
  s32 Result = GetIndex(P.x, P.y, Dim);
  return Result;
}

inline s32
GetIndex(voxel_position P, chunk_dimension Dim)
{
  s32 Result = GetIndex(P.x, P.y, P.z, Dim);
  return Result;
}

inline s32
GetIndex(v3 Offset, chunk_dimension Dim)
{
  s32 Index = GetIndex( Voxel_Position(Offset), Dim);
  return Index;
}

inline s32
GetIndexUnsafe(s32 X, s32 Y, s32 Z, chunk_dimension Dim)
{
  s32 Result = X +
              (Y*Dim.x) +
              (Z*Dim.x*Dim.y);

  Assert(Result >= 0);
  return Result;
}


inline voxel_position
V3iFromIndex(s32 Index, chunk_dimension Dim)
{
  Assert(Index >= 0);
  int x = Index % Dim.x;
  int y = (Index/Dim.x) % Dim.y;
  int z = Index / (Dim.x*Dim.y);

  // TODO(Jesse): Should this acutally not be strictly less than ..?
  Assert(x < Dim.x);
  Assert(y < Dim.y);
  Assert(z < Dim.z);

  voxel_position Result = Voxel_Position(x,y,z);
  return Result;
}

inline voxel_position
GetPosition(s32 Index, chunk_dimension Dim)
{
  auto Result = V3iFromIndex(Index, Dim);
  return Result;
}

inline voxel_position
PositionFromIndex(s32 Index, chunk_dimension Dim)
{
  return GetPosition(Index, Dim);
}

poof(maybe(v3))
#include <generated/maybe_v3.h>
poof(maybe(v3i))
#include <generated/maybe_v3i.h>


