#pragma pack(push, 1)
struct bitmap_image_header
{
  u32 SizeOfImageHeader;
  s32 WidthInPixels;
  s32 HeightInPixels;
  u16 ColorPlanes = 1;      // Must be 1
  u16 BPP;                  // Bits per pixel
  u32 CompressionType = 3;  // 0 == uncompressed
  u32 SizeInBytes = 0;      // 0 for uncompressed images
  u32 xPixelsPerMeter;      // Preferred Resolution in Pixels/Meter
  u32 yPixelsPerMeter;      // Preferred Resolution in Pixels/Meter
  u32 ColorMapsUsed;        // ??
  u32 NumSignificantColors; // ??
  u32 RedMask;
  u32 GreenMask;
  u32 BlueMask;
  u32 AlphaMask; // This is not technically defined in the spec, but I observed GIMP requiring it
};

struct bitmap_header
{
  u16 Type;
  u32 FileSizeInBytes;
  u32 Ignored;
  u32 OffsetToPixelData;
  bitmap_image_header Image;
};

struct bitmap
{
  v2i Dim;
  u32_cursor Pixels;
};
#pragma pack(pop)

link_internal void
SwizzleRedToBlueChannel(u32_cursor *PixelCursor)
{
  for (umm PixelIndex = 0; PixelIndex < TotalElements(PixelCursor); ++PixelIndex)
  {
    u32 *Pixel = PixelCursor->Start + PixelIndex;

    u32 Alpha = (*Pixel & 0xFF000000 ) >> 24;
    u32 Red   = (*Pixel & 0xFF0000   ) >> 16;
    u32 Green = (*Pixel & 0xFF00     ) >> 8;
    u32 Blue  = (*Pixel & 0xFF       );

    Assert( Alpha <= 0xFF );
    Assert( Red   <= 0xFF );
    Assert( Green <= 0xFF );
    Assert( Blue  <= 0xFF );

    *Pixel    = Red | (Green<<8) | (Blue<<16) | (Alpha<<24);
  }
}

// https://en.wikipedia.org/wiki/BMP_file_format
//
enum bitmap_compression_type
{
  BitmapCompressionType_RGB            = 0,  // none Most common
  BitmapCompressionType_RLE8           = 1,  // RLE 8-bit/pixel Can be used only with 8-bit/pixel bitmaps
  BitmapCompressionType_RLE4           = 2,  // RLE 4-bit/pixel Can be used only with 4-bit/pixel bitmaps
  BitmapCompressionType_BITFIELDS      = 3,  // OS22XBITMAPHEADER: Huffman 1D BITMAPV2INFOHEADER: RGB bit field masks, BITMAPV3INFOHEADER+: RGBA
  BitmapCompressionType_JPEG           = 4,  // OS22XBITMAPHEADER: RLE-24 BITMAPV4INFOHEADER+: JPEG image for printing[14]
  BitmapCompressionType_PNG            = 5,  // BITMAPV4INFOHEADER+: PNG image for printing[14]
  BitmapCompressionType_ALPHABITFIELDS = 6,  // RGBA bit field masks only Windows CE 5.0 with .NET 4.0 or later
  BitmapCompressionType_CMYK           = 11, // none only Windows Metafile CMYK[4]
  BitmapCompressionType_CMYKRLE8       = 12, // RLE-8 only Windows Metafile CMYK
  BitmapCompressionType_CMYKRLE4       = 13, // ?
};
poof(string_and_value_tables(bitmap_compression_type))
#include <generated/string_and_value_tables_bitmap_compression_type.h>

link_internal bitmap
ReadBitmapFromDisk(const char *Filename, memory_arena *Arena)
{
  bitmap_header Header = {};
  u32 *DestPixels = 0;
  u32  PixelCount = 0;

  u8_stream Buf = U8_StreamFromFile(Filename, Arena);
  if (Buf.Start)
  {
    Ensure(Read_struct(&Buf, &Header));

    // For now, we only support reading bitmaps that are bottom-up ie. Origin in top-left corner
    PixelCount = (u32)Header.Image.WidthInPixels * (u32)Header.Image.HeightInPixels;
  }
  else { Error("Opening %s for reading", Filename); }

  Assert( Header.OffsetToPixelData < TotalElements(&Buf));
  Buf.At = Buf.Start+Header.OffsetToPixelData;
  /* fseek(File.Handle, s32(Header.OffsetToPixelData), SEEK_SET); */

  u8 *SrcPixels = Buf.At;
  switch (bitmap_compression_type(Header.Image.CompressionType))
  {
    case BitmapCompressionType_RGB:
    {
      u32 TmpPixelsSize = PixelCount*3;
      Assert(TmpPixelsSize == Header.Image.SizeInBytes);

      DestPixels = Allocate(u32, Arena, PixelCount);

      u8 A = 0xFF;
      RangeIterator_t(u32, PixelIndex, PixelCount)
      {
        u32 rOffset = 0 + (PixelIndex*3);
        u32 gOffset = 1 + (PixelIndex*3);
        u32 bOffset = 2 + (PixelIndex*3);
        Assert(rOffset < TmpPixelsSize);
        Assert(gOffset < TmpPixelsSize);
        Assert(bOffset < TmpPixelsSize);

        u8 R = SrcPixels[rOffset];
        u8 G = SrcPixels[gOffset];
        u8 B = SrcPixels[bOffset];
        DestPixels[PixelIndex] = u32(R | (G<<8) | (B<<16) | (A<<24));
      }
    } break;

    case BitmapCompressionType_BITFIELDS:
    {
      DestPixels = Allocate(u32, Arena, PixelCount);
      CopyMemory(SrcPixels, Cast(u8*, DestPixels), PixelCount*4);
    } break;

    case BitmapCompressionType_RLE8:
    case BitmapCompressionType_RLE4:
    case BitmapCompressionType_JPEG:
    case BitmapCompressionType_PNG:
    case BitmapCompressionType_ALPHABITFIELDS:
    case BitmapCompressionType_CMYK:
    case BitmapCompressionType_CMYKRLE8:
    case BitmapCompressionType_CMYKRLE4:
    {
      SoftError("Unsupported Bitmap compression format encountered (%S)", ToString(bitmap_compression_type(Header.Image.CompressionType)));
    } break;
  }
  /* CloseFile(&File); */

  /* Assert(Header.Image.CompressionType == 3); */
  if (Buf.At != Buf.End)
  {
    Warn("(%s) loaded successfully, but we detected unsupported metadata during loading.", Filename);
  }


  bitmap Result = {};
  Result.Dim = V2i(Header.Image.WidthInPixels, Header.Image.HeightInPixels);
  Result.Pixels = U32Cursor(DestPixels, DestPixels+PixelCount);


  if (Header.Image.RedMask == 0xFF)
  {
    Assert(Header.Image.GreenMask == 0xFF00    );
    Assert(Header.Image.BlueMask  == 0xFF0000  );
    /* Assert(Header.Image.AlphaMask == 0xFF000000); */
  }
  else if (Header.Image.BlueMask == 0xFF)
  {
    Assert(Header.Image.GreenMask == 0xFF00    );
    Assert(Header.Image.RedMask   == 0xFF0000  );
    /* Assert(Header.Image.AlphaMask == 0xFF000000); */

    SwizzleRedToBlueChannel(&Result.Pixels);
  }
  else
  {
    Warn("(%s) loaded successfully, but detected unsupported pixel masking during loading.", Filename);
  }


  return Result;
}

poof(block_array(bitmap, {32}))
#include <generated/block_array_bitmap_688853862.h>

b32
WriteBitmapToDisk(bitmap *Bitmap, const char *Filename)
{
  bitmap_header Header = {};
  Header.Type = 0x4D42;
  Header.FileSizeInBytes = SafeTruncateToU32(sizeof(Header) + TotalSize(&Bitmap->Pixels));
  Header.OffsetToPixelData = sizeof(Header);

  Header.Image.SizeOfImageHeader    = sizeof(bitmap_image_header);
  Header.Image.WidthInPixels        = Bitmap->Dim.x;
  Header.Image.HeightInPixels       = Bitmap->Dim.y;
  Header.Image.ColorPlanes          = 1;
  Header.Image.BPP                  = 32;
  Header.Image.CompressionType      = 3;
  Header.Image.SizeInBytes          = SafeTruncateToU32(TotalSize(&Bitmap->Pixels));
  Header.Image.xPixelsPerMeter      = 2835;
  Header.Image.yPixelsPerMeter      = 2835;
  Header.Image.ColorMapsUsed        = 0;
  Header.Image.NumSignificantColors = 0;
  Header.Image.RedMask              = 0x000000FF;
  Header.Image.GreenMask            = 0x0000FF00;
  Header.Image.BlueMask             = 0x00FF0000;

  u32 SizeWritten = 0;
  native_file File = PlatformOpenFile(Filename, FilePermission_Write);
  if (File.Handle)
  {
    SizeWritten += PlatformWriteToFile(&File,              Cast(u8*, &Header), sizeof(Header));
    SizeWritten += PlatformWriteToFile(&File, Cast(u8*, Bitmap->Pixels.Start), TotalSize(&Bitmap->Pixels));
    CloseFile(&File);
  }
  else { Error("Opening %s for writing", Filename); }

  Assert(SizeWritten == Header.FileSizeInBytes);

  b32 Result = SizeWritten == Header.FileSizeInBytes;
  return Result;
}

bitmap
AllocateBitmap(v2i Dim, memory_arena *Arena)
{
  u32 Size = (u32)(Dim.x*Dim.y);
  u32* PixelPtr = Allocate(u32, Arena, Size);

  u32_cursor Pixels = U32Cursor(PixelPtr, PixelPtr+Size);

  bitmap Bitmap = {};
  Bitmap.Dim = Dim;
  Bitmap.Pixels = Pixels;

  return Bitmap;
}

v4
Unpack255RGBAToLinear(u32 C)
{
  u8 R =  0xFF & C;
  u8 G =  0xFF & (C>>8);
  u8 B =  0xFF & (C>>16);
  u8 A =  0xFF & (C>>24);

  v4 Color = {};
  Color.r = (r32)R/255.0f;
  Color.g = (r32)G/255.0f;
  Color.b = (r32)B/255.0f;
  Color.a = (r32)A/255.0f;

  return Color;
}

u32
PackRGBALinearTo255(v4 Color)
{
  u8 R = (u8)(Color.r*255);
  u8 G = (u8)(Color.g*255);
  u8 B = (u8)(Color.b*255);
  u8 A = (u8)(Color.a*255);

  u32 C =  (u32)( A<<24 | B<<16 | G<<8 | R);
  return C;
}

inline void
FillBitmap(u32 C, bitmap *Bitmap)
{
  Bitmap->Pixels.At = Bitmap->Pixels.Start;
  while (Bitmap->Pixels.At < Bitmap->Pixels.End)
  {
    *Bitmap->Pixels.At = C;
    Bitmap->Pixels.At++;
  }
  return;
}

inline s32
PixelCount(bitmap *Bitmap)
{
  s32 Result = Bitmap->Dim.x * Bitmap->Dim.y;
  return Result;
}

inline v2
GetUVForCharCode(u8 Char)
{
  v2 Result = V2( (Char%16)/16.0f, (Char/16)/16.0f );
  return Result;
}

