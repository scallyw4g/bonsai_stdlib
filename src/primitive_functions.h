#define CSz(NullTerminatedCString) \
  CS(NullTerminatedCString, sizeof(NullTerminatedCString)-1)

// TODO(Jesse): Remove this and add Flags to strings such that we just know if
// a string is static, null terminated, or a slice.
#define CSzs(NullTerminatedCString) \
  CS(NullTerminatedCString, sizeof(NullTerminatedCString))

link_inline cs CS(cs *Cs) { return *Cs; }
link_inline cs CS(cs Cs) { return Cs; }
link_inline cs CS(const char *S, umm Count);


inline umm
SafeTruncateToUMM(s32 Size)
{
  return umm(Size);
}

inline umm
SafeTruncateToUMM(u32 Size)
{
  return umm(Size);
}


inline umm
SafeTruncateToUMM(s64 Size)
{
  Assert(Size >= 0);
  Assert(umm(Size) <= umm_MAX);
  umm Result = (umm)Size;
  return Result;
}

inline umm
SafeTruncateToUMM(u64 Size)
{
  Assert(Size <= umm_MAX);
  umm Result = (umm)Size;
  return Result;
}

inline f32
SafeTruncateTo_f32(f64 Size)
{
  Assert(Size >= r64(f32_MIN));
  Assert(Size <= r64(f32_MAX));
  return f32(Size);
}

inline s32
SafeTruncateToS32(r64 Size)
{
  Assert(Size > r64(s32_MIN) && Size < r64(s32_MAX));
  s32 Result = Cast(s32, Cast(s64, Size));
  return Result;
}

inline s32
SafeTruncateToS32(u64 Size)
{
  Assert(Size <= s32_MAX);
  s32 Result = (s32)Size;
  return Result;
}

#if BONSAI_EMCC
inline s32
SafeTruncateToS32(umm Size)
{
  Assert(Size <= s32_MAX);
  s32 Result = (s32)Size;
  return Result;
}
#endif

inline u32
SafeTruncateToU32(u64 Size)
{
  Assert(Size <= u32_MAX);
  u32 Result = (u32)Size;
  return Result;
}

inline u16
SafeTruncateToU16(umm Size)
{
  Assert(Size <= u16_MAX);
  u16 Result = (u16)Size;
  return Result;
}

link_internal u8
SafeTruncateU8(u32 Size)
{
  Assert(Size <= u8_MAX);
  u8 Result = (u8)Size;
  return Result;
}

link_internal u8
SafeTruncateU8(s32 Size)
{
  Assert(Size >= 0);
  Assert(Size <= s32(u8_MAX));
  u8 Result = (u8)Size;
  return Result;
}

link_internal s8
SafeTruncateS8(u32 Size)
{
  Assert(s32(Size) <= s32(s8_MAX));
  s8 Result = (s8)Size;
  return Result;
}

link_internal s8
SafeTruncateS8(s32 Size)
{
  Assert(Size >= s32(s8_MIN));
  Assert(Size <= s32(s8_MAX));
  s8 Result = (s8)Size;
  return Result;
}


