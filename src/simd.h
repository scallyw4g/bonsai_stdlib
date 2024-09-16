
union f32_4x {
  __m128 Sse;
     r32 E[4];
};

union u32_4x {
  __m128i Sse;
      u32 E[4];
};


link_inline f32_4x
F32_4X(f32 A, f32 B, f32 C, f32 D)
{
  f32_4x Result = {{ _mm_set_ps(D, C, B, A) }};
  return Result;
}

link_inline f32_4x
F32_4X(f32 A)
{
  f32_4x Result = {{ _mm_set_ps1(A) }};
  return Result;
}

link_inline u32_4x
U32_4X(u32 A, u32 B, u32 C, u32 D)
{
  u32_4x Result = {{ _mm_set_epi32 (s32(D), s32(C), s32(B), s32(A)) }};
  return Result;
}

link_inline u32_4x
U32_4X(u32 A)
{
  u32_4x Result = U32_4X(A, A, A, A);
  return Result;
}



link_inline u32_4x
Select(u32_4x Mask, u32_4x A, u32_4x B)
{
  u32_4x Result = {};
  Result.Sse = _mm_or_ps(_mm_andnot_ps(Mask.Sse, A.Sse), _mm_and_ps(Mask.Sse, B.Sse));
  return Result;
}

link_inline f32_4x
Select(u32_4x Mask, f32_4x A, f32_4x B)
{
  f32_4x Result = {};
  Result.Sse = _mm_or_si128(_mm_andnot_si128(Mask.Sse, B.Sse), _mm_and_si128(Mask.Sse, A.Sse));
  return Result;
}

#define StaticShuffle(Source, Lane0Source, Lane1Source, Lane2Source, Lane3Source) \
  {{ _mm_shuffle_ps(Source.Sse, Source.Sse, (Lane0Source) | (Lane1Source<<2) | (Lane2Source<<4) | (Lane3Source<<6)) }}



//
// f32_4x
//

link_inline f32_4x
operator+(f32_4x A, f32_4x B)
{
  f32_4x Result = {{ _mm_add_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_4x
operator-(f32_4x A, f32_4x B)
{
  f32_4x Result = {{ _mm_sub_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_4x
operator*(f32_4x A, f32_4x B)
{
  f32_4x Result = {{ _mm_mul_ps(A.Sse, B.Sse) }};
  return Result;
}

link_inline f32_4x
operator/(f32_4x A, f32_4x B)
{
  f32_4x Result = {{ _mm_div_ps(A.Sse, B.Sse) }};
  return Result;
}






//
// u32_4x
//


link_inline u32_4x
operator+(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_add_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator-(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_sub_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator*(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_mul_epi32(A.Sse, B.Sse) }};
  return Result;
}

// NOTE(Jesse): Apparently this one doesn't exist?!
#if 0
link_inline u32_4x
operator/(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_div_epi32(A.Sse, B.Sse) }};
  return Result;
}
#endif

link_inline u32_4x
operator==(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_cmpeq_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator>(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_cmpgt_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator<(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_cmplt_epi32(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator|(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_or_si128(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator&(u32_4x A, u32_4x B)
{
  u32_4x Result = {{ _mm_and_si128(A.Sse, B.Sse) }};
  return Result;
}

link_inline u32_4x
operator&(u32_4x A, s32 B)
{
  u32_4x B4 = U32_4X(u32(B));
  u32_4x Result = A & B4;
  return Result;
}
