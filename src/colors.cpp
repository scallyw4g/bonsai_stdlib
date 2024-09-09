inline v3
GetPaletteData(v3_cursor *Palette, u32 ColorIndex)
{
  Assert(ColorIndex < AtElements(Palette));
  v3 Result = Palette->Start[ColorIndex];
  /* Assert(LengthSq(Result) < 1.1f); */
  return Result;
}

inline v3
GetMagicaVoxelRGBColor(v3_cursor *Palette, u32 ColorIndex)
{
  v3 Result = GetPaletteData(Palette, ColorIndex);
  return Result;
}
