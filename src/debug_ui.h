

struct cycle_range
{
  u64 StartCycle;
  u64 TotalCycles;
};


link_internal u8
GetByte(u32 ByteIndex, u32 Source)
{
  u32 ShiftWidth = ByteIndex * 8;
  Assert(ShiftWidth < (sizeof(Source)*8));

  u32 Mask = 0xff;
  u8 Result = Cast(u8, (Source >> ShiftWidth) & Mask);
  return Result;
}

link_internal v3
ColorFromHash(u32 HashValue)
{
  u32 TenBits = 0b1111111111;

  u32 Rb = HashValue & TenBits;
  u32 Gb = (HashValue >> 10) & TenBits;
  u32 Bb = (HashValue >> 20) & TenBits;

  r32 R = (r32(Rb)-1.f) / (r32(TenBits)-2.f);
  r32 G = (r32(Bb)-1.f) / (r32(TenBits)-2.f);
  r32 B = (r32(Gb)-1.f) / (r32(TenBits)-2.f);

  v3 Color = V3(R,G,B);
  return Color;
}


