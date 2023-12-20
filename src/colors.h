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

#define ICE_BLUE    207
#define GRASS_GREEN 232
#define STONE       GREY_6
#define GRAVEL      3
#define SAND        4
#define DIRT        138

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


struct v3_cursor;
link_internal v3 GetColorData(v3_cursor *Palette, u32 ColorIndex);

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
  v3 ColorData = GetColorData(Palette, ColorIndex);
  FillArray(ColorData, Dest, Count);
}

link_internal void
FillColorArray(v3 Color, v3 *Dest, s32 Count)
{
  FillArray(Color, Dest, Count);
}

