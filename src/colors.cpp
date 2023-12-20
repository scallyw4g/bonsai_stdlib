inline v3
GetColorData(v3_cursor *Palette, u32 ColorIndex)
{
  v3 Result = Palette->Start[ColorIndex] / 255.f;
  return Result;
}
