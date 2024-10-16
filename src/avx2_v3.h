
union v3_8x
{
  f32_8x Raw[3]; 

  struct
  {
    f32_8x x;
    f32_8x y;
    f32_8x z;
  } avx;

  struct
  {
     r32 x[8];
     r32 y[8];
     r32 z[8];
  } E;
};

/* #ifndef POOF_PREPROCESSOR */
/* f32 */
/* v3_8x::operator[](s32 Index) */
/* { */
/*   f32 Result = this->E[Index]; */
/*   return Result; */
/* } */
/* #endif */

/* poof(gen_vector_operators(v3_8x)) */
#include <generated/gen_vector_operators_v3_8x.h>

link_inline v3_8x
V3_8X(f32_8x x, f32_8x y, f32_8x z)
{
  v3_8x Result = {{
    x, y, z
  }};
  return Result;
}

link_inline v3_8x
V3_8X(f32 x, f32 y, f32 z)
{
  v3_8x Result = {{
    F32_8X(x),
    F32_8X(y),
    F32_8X(z),
  }};
  return Result;
}

link_inline v3_8x
V3_8X(u32 x, u32 y, u32 z)
{
  v3_8x Result = {{
    F32_8X(x),
    F32_8X(y),
    F32_8X(z),
  }};
  return Result;
}

link_inline v3_8x
V3_8X(s32 x, s32 y, s32 z)
{
  v3_8x Result = {{
    F32_8X(x),
    F32_8X(y),
    F32_8X(z),
  }};
  return Result;
}

link_inline v3_8x
V3_8X(s32 Scalar)
{
  v3_8x Result = {{
    F32_8X(Scalar),
    F32_8X(Scalar),
    F32_8X(Scalar),
  }};
  return Result;
}




link_inline v3_8x
Select(u32_8x Mask, v3_8x A, v3_8x B)
{
  v3_8x Result = {{
    {Select(Mask, A.avx.x, B.avx.x).Sse},
    {Select(Mask, A.avx.y, B.avx.y).Sse},
    {Select(Mask, A.avx.z, B.avx.z).Sse},
  }};
  return Result;
}



link_inline f32_8x
InverseSquareRoot(f32_8x A)
{
  /* if (x == 0) return 0.f; */
#if BONSAI_FAST_MATH__INVSQRT 
  f32_8x Result = {{ _mm256_rsqrt_ps(A) }};
#else
  f32_8x Result = {{ 1.f/_mm256_sqrt_ps(A.Sse) }};
#endif
  return Result;
}

link_inline f32_8x
SquareRoot(f32_8x F)
{
#if BONSAI_FAST_MATH__SQRT 
  f32_8x Result = {{ InverseSquareRoot(F).Sse * F }};
#else
  f32_8x Result = {{ _mm256_sqrt_ps(F.Sse) }};
#endif
  /* Assert(!isnan(Result)); */
  return Result;
}


inline f32_8x
LengthSq( v3_8x Vec )
{
  f32_8x Result = Vec.Raw[0]*Vec.Raw[0] + Vec.Raw[1]*Vec.Raw[1] + Vec.Raw[2]*Vec.Raw[2];
  return Result;
}

inline f32_8x
Length( v3_8x Vec )
{
  f32_8x Result = SquareRoot(LengthSq(Vec));
  return Result;
}

inline v3_8x
Normalize(v3_8x Vec, f32_8x Length)
{
  /* if (Length == 0.f) return {}; */
  v3_8x Result = Vec / Length;
  return Result;
}


inline v3_8x
Normalize( v3_8x Vec )
{
  v3_8x Result = Normalize(Vec, Length(Vec));
  return Result;
}

