

struct cycle_range
{
  u64 StartCycle;
  u64 TotalCycles;
};


link_internal u8
GetByte(u32 ByteIndex, u64 Source)
{
  u32 ShiftWidth = ByteIndex * 8;
  Assert(ShiftWidth < (sizeof(Source)*8));

  u64 Mask = (u64)(0xff << ShiftWidth);
  u8 Result = (u8)((Source & Mask) >> ShiftWidth);
  return Result;
}

link_internal v3
ColorFromHash(u64 HashValue)
{
  u8 R = GetByte(0, HashValue);
  u8 G = GetByte(1, HashValue);
  u8 B = GetByte(2, HashValue);

  v3 Color = V3( R / 255.0f, G / 255.0f, B / 255.0f );
  return Color;
}


