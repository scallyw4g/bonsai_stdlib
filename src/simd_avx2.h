
union f32_8x {
  __m256 Sse;
     r32 E[8];
  f32 operator[](s32 Index);
};

// TODO(Jesse): Rename to s32?
union u32_8x {
  __m256i Sse;
      u32 E[8];
  u32 operator[](s32 Index);
};


link_inline f32_8x
F32_8X(f32 A, f32 B, f32 C, f32 D, f32 E, f32 F, f32 G, f32 H)
{
  f32_8x Result = {{ _mm256_set_ps(H, G, F, E, D, C, B, A) }};
  return Result;
}

link_inline f32_8x
F32_8X(f32 A)
{
  f32_8x Result = {{ _mm256_set_ps(A, A, A, A, A, A, A, A) }};
  return Result;
}

link_inline f32_8x
F32_8X(u32_8x A)
{
  f32_8x Result = {{ _mm256_castsi256_ps(A.Sse) }};
  /* f32_8x Result = {{ _mm256_cvtepi32_ps(A.Sse) }}; */
  return Result;
}

link_inline u32_8x
U32_8X(u32 A, u32 B, u32 C, u32 D, u32 E, u32 F, u32 G, u32 H)
{
  u32_8x Result = {{ _mm256_set_epi32 (s32(H), s32(G), s32(F), s32(E), s32(D), s32(C), s32(B), s32(A)) }};
  return Result;
}

link_inline u32_8x
U32_8X(u32 A)
{
  u32_8x Result = U32_8X((A), (A), (A), (A), (A), (A), (A), (A));
  return Result;
}

link_inline u32_8x
U32_8X(f32_8x A)
{
  u32_8x Result = {{ _mm256_cvtps_epi32(A.Sse) }};
  return Result;
}




link_inline f32_8x
Select(u32_8x Mask, f32_8x A, f32_8x B)
{
  f32_8x Result = {{ _mm256_blendv_ps( B.Sse, A.Sse, Mask.Sse ) }};
  return Result;
}

link_inline u32_8x
Select(u32_8x Mask, u32_8x A, u32_8x B)
{
  // NOTE(Jesse): The blendv instruction is 3 cycles/cell faster for computing perlin noise in AVX
  /* u32_8x Result = {{_mm256_or_ps(_mm256_and_ps(Mask.Sse, A.Sse), _mm256_andnot_ps(Mask.Sse, B.Sse))}}; */
  u32_8x Result = {{ _mm256_blendv_ps( B.Sse, A.Sse, _mm256_castsi256_ps( Mask.Sse ) ) }};
  return Result;
}


#define StaticShuffle_avx(Source, Lane0Source, Lane1Source, Lane2Source, Lane3Source) \
  {{ _mm256_shuffle_ps(Source.Sse, Source.Sse, (Lane0Source) | (Lane1Source<<2) | (Lane2Source<<4) | (Lane3Source<<6)) }}



//
// f32_8x
//

#ifndef POOF_PREPROCESSOR
f32
f32_8x::operator[](s32 Index)
{
  f32 Result = this->E[Index];
  return Result;
}
#endif

link_inline f32_8x
operator+(f32_8x A, f32_8x B)
{
  f32_8x Result = {{ _mm256_add_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_8x
operator-(f32_8x A, f32_8x B)
{
  f32_8x Result = {{ _mm256_sub_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_8x
operator*(f32_8x A, f32_8x B)
{
  f32_8x Result = {{ _mm256_mul_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_8x
operator/(f32_8x A, f32_8x B)
{
  f32_8x Result = {{ _mm256_div_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_8x
operator^(f32_8x A, f32_8x B)
{
  f32_8x Result = {{ _mm256_xor_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator<<(u32_8x A, int B)
{
  u32_8x Result = {{ _mm256_slli_epi32(A.Sse, B) }};
  return Result;
}

link_inline u32_8x
operator<(f32_8x A, f32_8x B)
{
  u32_8x Result = {{ _mm256_cmp_ps( A.Sse, B.Sse, _CMP_LT_OS) }};
  return Result;
}






//
// u32_8x
//

#ifndef POOF_PREPROCESSOR
u32
u32_8x::operator[](s32 Index)
{
  u32 Result = this->E[Index];
  return Result;
}
#endif


link_inline u32_8x
operator+(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_add_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator-(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_sub_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator*(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_mul_epi32(A.Sse, B.Sse) }};
  return Result;
}

// NOTE(Jesse): Apparently this one doesn't exist?!
#if 0
link_inline u32_8x
operator/(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_div_epi32(A.Sse, B.Sse) }};
  return Result;
}
#endif

link_inline u32_8x
operator==(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_cmpeq_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator>(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_cmpgt_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator<(u32_8x A, u32_8x B)
{
  // NOTE(Jesse): There's no lt instruction; we issue a gt with operands reversed
  u32_8x Result = {{ _mm256_cmpgt_epi32(B.Sse, A.Sse) }};
  return Result;
}

link_inline u32_8x
operator|(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_or_si256(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator&(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_and_si256(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_8x
operator&(u32_8x A, s32 B)
{
  u32_8x B4 = U32_8X(u32(B));
  u32_8x Result = A & B4;
  return Result;
}

link_inline f32_8x
FMA(f32_8x MulLHS, f32_8x MulRHS, f32_8x AddRHS)
{
  f32_8x Result = {{ _mm256_fmadd_ps(MulLHS.Sse, MulRHS.Sse, AddRHS.Sse) }};
  return Result;
}

link_inline f32_8x
Floor(f32_8x A)
{
  f32_8x Result = {{ _mm256_round_ps( A.Sse, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC ) }};
  return Result;
}
