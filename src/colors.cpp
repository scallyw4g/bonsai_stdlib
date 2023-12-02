inline v3
GetColorData(v3_cursor *Palette, u32 ColorIndex)
{
  /* Assert(ColorIndex < u32(Global_ColorPaletteAt)); */
  /* v3_cursor *Palette = GetEngineResources()->World.ColorPalette; */
  v3 Result = Palette->Start[ColorIndex] / 255.f;
  return Result;
}
