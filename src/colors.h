/* Note that the default MagicaVoxel format is AA GG BB RR, so something will
 * have to be done if we continue to load this format.  I manually rearranged
 * the columns in this lookup table to match what OpenGL expects: RR GG BB AA
 */

#define BLACK    0
#define WHITE    1

#define     LIGHT_RED 104
#define           RED 36
#define      DARK_RED 144
#define DARK_DARK_RED 180

#define LIGHT_GREEN 75
#define       GREEN 186


#define      LIGHT_MOSS_GREEN  168
#define            MOSS_GREEN  174

#define      GRASS_GREEN 232
#define DARK_GRASS_GREEN 235

#define SNOW        1
#define ICE_BLUE    207

#define      STONE       GREY_6
#define DARK_STONE       GREY_8

#define GRAVEL      3
#define SAND        4

#define LIGHT_DIRT        102
#define       DIRT        138

#define LONG_YELLOW_GRASS0 5
#define LONG_YELLOW_GRASS1 6

#define LONG_GREEN_GRASS0 119
#define LONG_GREEN_GRASS1 120

#define BLUE 211
#define PINK 27
#define TEAL 181 // actually cyan

#define LIGHT_LIGHT_YELLOW 4
#define       LIGHT_YELLOW 5
#define             YELLOW 6


#define LIGHT_LIGHT_ORANGE 6
#define       LIGHT_ORANGE 12
#define             ORANGE 18
#define        DARK_ORANGE 24



#define      DARK_GREY 254
#define DARK_DARK_GREY 255

#define GREY_0   246
#define GREY_1   247
#define GREY_2   248
#define GREY_3   249
#define GREY_4   250
#define GREY_5   251
#define GREY_6   252
#define GREY_7   253
#define GREY_8   254
#define GREY_9   255


#define STANDING_SPOT_DEFAULT_COLOR 128


#define HSV_GRASS_GREEN (V3(0.31f, 0.8f, 0.74f))
#define HSV_ICE_BLUE    (V3(0.0f, 0.5f, 0.75f))
#define HSV_DARK_GREY   (V3(0.0f, 0.5f, 0.75f))
#define HSV_YELLOW      (V3(0.0f, 0.5f, 0.75f))
#define HSV_WHITE       (V3(0.0f, 0.5f, 0.75f))







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

  switch (i % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
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
RGBtoHSV(v3 RGB)
{
  NotImplemented;
  v3 Result = {};
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
PackHSVColor(v3 Color)
{
  /* u8 FourBits   = 0b1111; */
  u8 FiveBits   = 0b11111;
  u8 SixBits    = 0b111111;
  /* u8 EightBits  = 0b11111111; */

  u16 H = u16(Color.h * r32(SixBits));
  u16 S = u16(Color.s * r32(FiveBits));
  u16 V = u16(Color.v * r32(FiveBits));

  Assert(H <= SixBits);
  Assert(S <= FiveBits);
  Assert(V <= FiveBits);


  u16 Result = u16((H << 10) | (S << 5) | V);

  /* v3 Check = UnpackHSVColor(Result); */
  /* Assert(Check == Color); */
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

