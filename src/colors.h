/* Note that the default MagicaVoxel format is AA GG BB RR, so something will
 * have to be done if we continue to load this format.  I manually rearranged
 * the columns in this lookup table to match what OpenGL expects: RR GG BB AA
 */

// NOTE(Jesse): Colors I named from magicavoxels default palette.  Mostly deprecated
#define              MCV_BLACK 0
#define              MCV_WHITE 1

#define          MCV_LIGHT_RED 104
#define                MCV_RED 36
#define           MCV_DARK_RED 144
#define      MCV_DARK_DARK_RED 180

#define        MCV_LIGHT_GREEN 75
#define              MCV_GREEN 186


#define   MCV_LIGHT_MOSS_GREEN 168
#define         MCV_MOSS_GREEN 174

#define        MCV_GRASS_GREEN 232
#define   MCV_DARK_GRASS_GREEN 235

#define               MCV_SNOW 1
#define           MCV_ICE_BLUE 207

#define              MCV_STONE MCV_GREY_6
#define         MCV_DARK_STONE MCV_GREY_8

#define             MCV_GRAVEL 3
#define               MCV_SAND 4

#define         MCV_LIGHT_DIRT 102
#define               MCV_DIRT 138

#define MCV_LONG_YELLOW_GRASS0 5
#define MCV_LONG_YELLOW_GRASS1 6

#define  MCV_LONG_GREEN_GRASS0 119
#define  MCV_LONG_GREEN_GRASS1 120

#define               MCV_BLUE 211
#define               MCV_PINK 27
#define               MCV_TEAL 181 // actually cyan

#define MCV_LIGHT_LIGHT_YELLOW 4
#define       MCV_LIGHT_YELLOW 5
#define             MCV_YELLOW 6


#define MCV_LIGHT_LIGHT_ORANGE 6
#define       MCV_LIGHT_ORANGE 12
#define             MCV_ORANGE 18
#define        MCV_DARK_ORANGE 24

#define          MCV_DARK_GREY 254
#define     MCV_DARK_DARK_GREY 255

#define             MCV_GREY_0 246
#define             MCV_GREY_1 247
#define             MCV_GREY_2 248
#define             MCV_GREY_3 249
#define             MCV_GREY_4 250
#define             MCV_GREY_5 251
#define             MCV_GREY_6 252
#define             MCV_GREY_7 253
#define             MCV_GREY_8 254
#define             MCV_GREY_9 255


#define MCV_STANDING_SPOT_DEFAULT_COLOR 128
#define STANDING_SPOT_DEFAULT_COLOR (MagicaVoxelDefaultPaletteToRGB(MCV_STANDING_SPOT_DEFAULT_COLOR))

// A small value
#define smol 0.001f

// HSV

#define HSV_WHITE       (V3(0.0f,  0.0f, 1.0f))
#define HSV_BLACK       (V3(0.0f,  0.0f, 0.0f))

#define HSV_GRASS_GREEN (V3(0.33f, 0.96f, 0.61f))
#define HSV_ICE_BLUE    (V3(0.0f,  0.5f, 0.75f))
#define HSV_DARK_GREY   (V3(0.0f,  0.5f, 0.75f))

#define HSV_RED         (RGBtoHSV(RGB_RED))
#define HSV_GREEN       (RGBtoHSV(RGB_GREEN))
#define HSV_BLUE        (RGBtoHSV(RGB_BLUE))

#define HSV_YELLOW      (V3(0.16f,  1.0f, 1.0f))


#define HSV_PINK        (RGBtoHSV(RGB_PINK))


// RGB


#define RGB_RED         (V3(1.0f,  smol, smol))
#define RGB_GREEN       (V3(smol,  1.0f, smol))
#define RGB_BLUE        (V3(smol,  smol, 1.0f))

#define RGB_YELLOW      (V3(1.0f,  1.0f, smol))
#define RGB_CYAN        (V3(0.0f,  1.0f, 1.0f))
#define RGB_MAGENTA     (V3(1.0f,  1.0f, smol))

#define RGB_PINK        (V3(1.0f, smol,  0.5f))

#define RGB_WHITE       (V3(1.0f,  1.0f, 1.0f))
#define RGB_BLACK       (V3(0.0f,  0.0f, 0.0f))

#define          RGB_LIGHT_RED MagicaVoxelDefaultPaletteToRGB(104)
#define           RGB_DARK_RED MagicaVoxelDefaultPaletteToRGB(144)
#define      RGB_DARK_DARK_RED MagicaVoxelDefaultPaletteToRGB(180)

#define        RGB_LIGHT_GREEN MagicaVoxelDefaultPaletteToRGB(75)


#define   RGB_LIGHT_MOSS_GREEN MagicaVoxelDefaultPaletteToRGB(168)
#define         RGB_MOSS_GREEN MagicaVoxelDefaultPaletteToRGB(174)

#define        RGB_GRASS_GREEN MagicaVoxelDefaultPaletteToRGB(232)
#define   RGB_DARK_GRASS_GREEN MagicaVoxelDefaultPaletteToRGB(235)

#define               RGB_SNOW MagicaVoxelDefaultPaletteToRGB(1)
#define           RGB_ICE_BLUE MagicaVoxelDefaultPaletteToRGB(207)

#define              RGB_STONE MagicaVoxelDefaultPaletteToRGB(MCV_GREY_6)
#define         RGB_DARK_STONE MagicaVoxelDefaultPaletteToRGB(MCV_GREY_8)

#define             RGB_GRAVEL MagicaVoxelDefaultPaletteToRGB(3)
#define               RGB_SAND MagicaVoxelDefaultPaletteToRGB(4)

#define         RGB_LIGHT_DIRT MagicaVoxelDefaultPaletteToRGB(102)
#define               RGB_DIRT MagicaVoxelDefaultPaletteToRGB(138)

#define RGB_LONG_YELLOW_GRASS0 MagicaVoxelDefaultPaletteToRGB(5)
#define RGB_LONG_YELLOW_GRASS1 MagicaVoxelDefaultPaletteToRGB(6)

#define  RGB_LONG_GREEN_GRASS0 MagicaVoxelDefaultPaletteToRGB(119)
#define  RGB_LONG_GREEN_GRASS1 MagicaVoxelDefaultPaletteToRGB(120)

#define               RGB_TEAL MagicaVoxelDefaultPaletteToRGB(181)

#define RGB_LIGHT_LIGHT_YELLOW MagicaVoxelDefaultPaletteToRGB(4)
#define       RGB_LIGHT_YELLOW MagicaVoxelDefaultPaletteToRGB(5)


#define RGB_LIGHT_LIGHT_ORANGE MagicaVoxelDefaultPaletteToRGB(6)
#define       RGB_LIGHT_ORANGE MagicaVoxelDefaultPaletteToRGB(12)
#define             RGB_ORANGE MagicaVoxelDefaultPaletteToRGB(18)
#define        RGB_DARK_ORANGE MagicaVoxelDefaultPaletteToRGB(24)

#define          RGB_DARK_GREY MagicaVoxelDefaultPaletteToRGB(254)
#define     RGB_DARK_DARK_GREY MagicaVoxelDefaultPaletteToRGB(255)

#define             RGB_GREY_0 MagicaVoxelDefaultPaletteToRGB(246)
#define             RGB_GREY_1 MagicaVoxelDefaultPaletteToRGB(247)
#define             RGB_GREY_2 MagicaVoxelDefaultPaletteToRGB(248)
#define             RGB_GREY_3 MagicaVoxelDefaultPaletteToRGB(249)
#define             RGB_GREY_4 MagicaVoxelDefaultPaletteToRGB(250)
#define             RGB_GREY_5 MagicaVoxelDefaultPaletteToRGB(251)
#define             RGB_GREY_6 MagicaVoxelDefaultPaletteToRGB(252)
#define             RGB_GREY_7 MagicaVoxelDefaultPaletteToRGB(253)
#define             RGB_GREY_8 MagicaVoxelDefaultPaletteToRGB(254)
#define             RGB_GREY_9 MagicaVoxelDefaultPaletteToRGB(255)





//
// https://github.com/Inseckto/HSV-to-RGB/blob/master/HSV2RGB.c
//
link_internal v3
HSVtoRGB(f32 H, f32 S, f32 V)
{
  f32 r = 0, g = 0, b = 0;

  f32 h = H;
  f32 s = S;
  f32 v = V;

  int i = s32(Floorf(h * 6));
  f32 f = h * 6 - i;
  f32 p = v * (1 - s);
  f32 q = v * (1 - f * s);
  f32 t = v * (1 - (1 - f) * s);

  switch (i)
  {
    case 6:
    case 0: {  r = v; g = t; b = p; break; }

    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;

    InvalidDefaultCase;
  }

  v3 Result = V3(r,g,b);
  return Result;
}

link_internal v3
HSVtoRGB(v3 HSV)
{
  return HSVtoRGB(HSV.h, HSV.s, HSV.v);
}

link_internal v3
RGBtoHSV(f32 r, f32 g, f32 b)
{
  f32 h, s, v; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

  f32 max = Max(r, Max(g, b));
  f32 min = Min(r, Min(g, b));

  v = max;

  if (max == 0.0f) {
      s = 0;
      h = 0;
  }
  else if (max - min == 0.0f) {
      s = 0;
      h = 0;
  }
  else {
      s = (max - min) / max;

      if (max == r) {
          h = 60 * ((g - b) / (max - min)) + 0;
      }
      else if (max == g) {
          h = 60 * ((b - r) / (max - min)) + 120;
      }
      else {
          h = 60 * ((r - g) / (max - min)) + 240;
      }
  }

  if (h < 0) h += 360.0f;

  h = (h / 360.f);

  v3 result = V3(h,s,v);
  return result;
}

link_internal v3
RGBtoHSV(v3 RGB)
{
  v3 Result = RGBtoHSV(RGB.r, RGB.g, RGB.b);
  return Result;
}

link_internal v3
UnpackV3_15b(u16 Packed)
{
  s32 FiveBits = 31;

  r32 x = ((Packed >> 10) & FiveBits) / r32(FiveBits);
  r32 y = ((Packed >> 5) & FiveBits) / r32(FiveBits);
  r32 z =  (Packed & FiveBits) / r32(FiveBits);
  v3 Result = V3(x, y, z);
  /* Assert(Length(Result) >= 1.f); */
  return Result;
}

link_internal v3
UnpackHSVColor(u16 Packed)
{
  /* u8 FourBits   = 0b1111; */
  s32 FiveBits = 31;
  s32 SixBits   = 63;
  /* u8 EightBits  = 0b11111111; */

  r32 H = ((Packed >> 10) & SixBits) / r32(SixBits);
  r32 S = ((Packed >> 5) & FiveBits) / r32(FiveBits);
  r32 V =  (Packed & FiveBits) / r32(FiveBits);
  v3 Result = V3(H, S, V);
  return Result;
}

link_internal v3
UnpackHSVColorToRGB(u16 Packed)
{
  v3 HSV = UnpackHSVColor(Packed);
  v3 Result = HSVtoRGB(HSV);
  return Result;
}

link_internal u16
PackV3_16b(v3 Color)
{
  /* u8 FourBits   = 0b1111; */
  u8 FiveBits   = 0b11111;
  u8 SixBits    = 0b111111;
  /* u8 EightBits  = 0b11111111; */

  u16 H = u16(Color.h * r32(SixBits));
  u16 S = u16(Color.s * r32(FiveBits));
  u16 V = u16(Color.v * r32(FiveBits));

  /* Assert(H <= SixBits); */
  /* Assert(S <= FiveBits); */
  /* Assert(V <= FiveBits); */


  u16 Result = u16((H << 10) | (S << 5) | V);

  /* v3 Check = UnpackHSVColor(Result); */
  /* Assert(Check == Color); */
  return Result;
}

link_internal u16
RGBtoPackedHSV(v3 RGB)
{
  v3 HSV = RGBtoHSV(RGB);
  u16 Packed = PackV3_16b(HSV);
  return Packed;
}

link_internal v3
PackedHSVtoRGB(u16 PackedHSV)
{
  v3 HSV = UnpackHSVColor(PackedHSV);
  v3 Result = HSVtoRGB(HSV);
  return Result;
}








link_internal v3
HSVSaturate(v3 HSV, r32 Percentage = 0.1f)
{
  v3 Result = V3(HSV.h, HSV.s * (1.f+Percentage), HSV.v);
  return Result;
}
link_internal v3
HSVDesaturate(v3 HSV, r32 Percentage = 0.1f)
{
  v3 Result = V3(HSV.h, HSV.s * (1.f-Percentage), HSV.v);
  return Result;
}
link_internal v3
HSVLighten(v3 HSV, r32 Percentage = 0.1f)
{
  v3 Result = V3(HSV.h, HSV.s, HSV.v * (1.f+Percentage));
  return Result;
}
link_internal v3
HSVDarken(v3 HSV, r32 Percentage = 0.1f)
{
  v3 Result = V3(HSV.h, HSV.s, HSV.v * (1.f-Percentage));
  return Result;
}


link_internal v3
RGBSaturate(v3 RGB, r32 Percentage = 0.1f)
{
  v3 Result = HSVtoRGB(HSVSaturate(RGBtoHSV(RGB), Percentage));
  return Result;
}
link_internal v3
RGBDesaturate(v3 RGB, r32 Percentage = 0.1f)
{
  v3 Result = HSVtoRGB(HSVDesaturate(RGBtoHSV(RGB), Percentage));
  return Result;
}
link_internal v3
RGBLighten(v3 RGB, r32 Percentage = 0.1f)
{
  v3 Result = HSVtoRGB(HSVLighten(RGBtoHSV(RGB), Percentage));
  return Result;
}
link_internal v3
RGBDarken(v3 RGB, r32 Percentage = 0.1f)
{
  v3 Result = HSVtoRGB(HSVDarken(RGBtoHSV(RGB), Percentage));
  return Result;
}










struct v3_cursor;
link_internal v3 GetMagicaVoxelRGBColor(v3_cursor *Palette, u32 ColorIndex);

link_internal void
FillArray(v2 Element, v2 *Dest, s32 Count)
{
  for (s32 Index = 0; Index < Count; ++Index) { Dest[Index] = Element; }
}

link_internal void
FillArray(v3 Element, v3_u8 *Dest, s32 Count)
{
  for (s32 Index = 0; Index < Count; ++Index) { Dest[Index] = V3U8(Element*127.f); }
}

link_internal void
FillArray(v3 Element, v3 *Dest, s32 Count)
{
  for (s32 Index = 0; Index < Count; ++Index) { Dest[Index] = Element; }
}

link_internal void
FillColorArray(v3_cursor *Palette, u32 ColorIndex, v3 *Dest, s32 Count)
{
  v3 ColorData = RGBtoHSV(GetMagicaVoxelRGBColor(Palette, ColorIndex));
  FillArray(ColorData, Dest, Count);
}

link_internal void
FillColorArray(v3 Color, v3 *Dest, s32 Count)
{
  FillArray(Color, Dest, Count);
}

