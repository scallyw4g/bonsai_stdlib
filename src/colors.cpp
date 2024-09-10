inline v3
GetPaletteData(v3_cursor *Palette, u32 ColorIndex)
{
  v3 Result = {};
  if (ColorIndex < AtElements(Palette))
  {
    Result = Palette->Start[ColorIndex];
  }
  return Result;
}

inline v3
GetMagicaVoxelRGBColor(v3_cursor *Palette, u32 ColorIndex)
{
  v3 Result = GetPaletteData(Palette, ColorIndex);
  return Result;
}
