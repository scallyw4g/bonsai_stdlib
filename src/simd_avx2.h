
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
  f32_8x Result = {{ _mm256_cvtepi32_ps(A.Sse) }};
  return Result;
}

f32_8x
Cast_f32_8x(u32_8x A)
{
  f32_8x Result = {{ _mm256_castsi256_ps(A.Sse) }};
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
U32_8X(s32 A)
{
  u32_8x Result = U32_8X(u32(A), u32(A), u32(A), u32(A), u32(A), u32(A), u32(A), u32(A));
  return Result;
}


link_inline u32_8x
U32_8X(__m256i A)
{
  u32_8x Result = { .Sse = A };
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


#define StaticShuffle_avx(Source, Lane0Source, Lane1Source, Lane2Source, Lane3Source, Lane4Source, Lane5Source, Lane6Source, Lane7Source) \
  {{ _mm256_permutevar8x32_ps(Source.Sse, U32_8X(Lane0Source, Lane1Source, Lane2Source, Lane3Source, Lane4Source, Lane5Source, Lane6Source, Lane7Source).Sse )  }}



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
  u32_8x Result = {{ _mm256_mullo_epi32(A.Sse, B.Sse) }};
  return Result;
}

static inline u32 bit_scan_reverse(u32 a) __attribute__((pure));
static inline u32 bit_scan_reverse(u32 a) {
    u32 r;
    __asm("bsrl %1, %0" : "=r"(r) : "r"(a) : );
    return r;
}

#if 1
struct avx_divisor
{
  u32 E;
  __m128i multiplier, shift1, shift2;
};

link_inline avx_divisor
AvxDivisor(u32 D)
{
  u32 L, L2, sh1, sh2, m;
  switch (D) {
  case 0:
      m = sh1 = sh2 = 1 / D;                         // provoke error for D = 0
      break;
  case 1:
      m = 1; sh1 = sh2 = 0;                          // parameters for D = 1
      break;
  case 2:
      m = 1; sh1 = 1; sh2 = 0;                       // parameters for D = 2
      break;
  default:                                           // general case for D > 2
      L = bit_scan_reverse(D - 1) + 1;               // ceil(log2(D))
      L2 = u32(L < 32 ? 1 << L : 0);            // 2^L, overflow to 0 if L = 32
      m = 1 + u32((u64(L2 - D) << 32) / D);     // multiplier
      sh1 = 1;  sh2 = L - 1;                         // shift counts
  }

  avx_divisor Result = {
    .E = D,
    .multiplier = _mm_set1_epi32((s32)m),
    .shift1 = _mm_setr_epi32((s32)sh1, 0, 0, 0),
    .shift2 = _mm_setr_epi32((s32)sh2, 0, 0, 0),
  };
  return Result;
}

link_inline u32_8x
operator/(u32_8x A, avx_divisor B)
{
    __m256i m   = _mm256_broadcastq_epi64(B.multiplier);  // broadcast multiplier
    __m256i t1  = _mm256_mul_epu32(A.Sse,m);              // 32x32->64 bit unsigned multiplication of even elements of A
    __m256i t2  = _mm256_srli_epi64(t1,32);               // high dword of even numbered results
    __m256i t3  = _mm256_srli_epi64(A.Sse,32);            // get odd elements of A into position for multiplication
    __m256i t4  = _mm256_mul_epu32(t3,m);                 // 32x32->64 bit unsigned multiplication of odd elements
    __m256i t7  = _mm256_blend_epi32(t2,t4,0xAA);
    __m256i t8  = _mm256_sub_epi32(A.Sse,t7);             // subtract
    __m256i t9  = _mm256_srl_epi32(t8,B.shift1);          // shift right logical
    __m256i t10 = _mm256_add_epi32(t7,t9);                // add
    __m256i res = _mm256_srl_epi32(t10,B.shift2);         // shift right logical
    return U32_8X(res);
}

#else
link_inline u32_8x
operator/(u32_8x A, u32_8x B)
{
  u32 C[8];

  C[0] = A[0] / B[0];
  C[1] = A[1] / B[1];
  C[2] = A[2] / B[2];
  C[3] = A[3] / B[3];
  C[4] = A[4] / B[4];
  C[5] = A[5] / B[5];
  C[6] = A[6] / B[6];
  C[7] = A[7] / B[7];

  u32_8x Result = U32_8X(C[0], C[1], C[2], C[3], C[4], C[5], C[6], C[7]);
  return Result;
}
link_inline u32_8x
operator%(u32_8x A, u32_8x B)
{
  u32 C[8];

  C[0] = A[0] % B[0];
  C[1] = A[1] % B[1];
  C[2] = A[2] % B[2];
  C[3] = A[3] % B[3];
  C[4] = A[4] % B[4];
  C[5] = A[5] % B[5];
  C[6] = A[6] % B[6];
  C[7] = A[7] % B[7];

  u32_8x Result = U32_8X(C[0], C[1], C[2], C[3], C[4], C[5], C[6], C[7]);
  return Result;
}
#endif

link_inline u32_8x
operator>>(u32_8x A, int B)
{
  u32_8x Result = {{ _mm256_srli_epi32(A.Sse, B) }};
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


link_inline u32_8x
operator^(u32_8x A, u32_8x B)
{
  u32_8x Result = {{ _mm256_xor_si256(A.Sse, B.Sse) }};
  return Result;
}


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

link_inline void
ShuffleToMemoryOrderAndStore(f32 *Dest, f32_8x Data)
{
  f32_8x MemoryOrdered = StaticShuffle_avx(Data, 7, 6, 5, 4, 3, 2, 1, 0);
  _mm256_store_ps(Dest, MemoryOrdered.Sse);
}
