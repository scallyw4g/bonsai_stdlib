
/* poof(block_array_c(texture, {8})) */
/* #include <generated/block_array_c_texture_688856411.h> */

poof(block_array_c(texture, {8}))
#include <generated/block_array_c_texture_688856411.h>

poof(block_array_c(texture_ptr, {8}))
#include <generated/block_array_texture_ptr_688856411.h>

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

/* // Note(Jesse): Must match shader define in header.glsl */
/* #define DEBUG_TEXTURE_DIM 512 */

#if 0
texture
LoadDDS(const char * FilePath, memory_arena *Arena)
{
  texture Result = {};

  FILE *TextureFile = fopen(FilePath, "rb");

  if (!TextureFile){
    Error("Couldn't open shader - Shit!");
    Assert(False);
    return Result;
  }

  const u32 FileCodeLength = 4;
  char filecode[FileCodeLength];

  ReadBytes((u8*)filecode, FileCodeLength, TextureFile);
  if (!StringsMatch(CS(filecode), CS("DDS ")) )
  {
    Error("Invalid File format opening DDS file");
    Assert(False);
    fclose(TextureFile);
    return Result;
  }

  const u32 HeaderLength = 124;
  u32 Header[HeaderLength];
  ReadBytes((u8*)Header, HeaderLength, TextureFile);

  s32 height      = (s32)Header[2];
  s32 width       = (s32)Header[3];
  u32 linearSize  = Header[4];
  s32 mipMapCount = (s32)Header[6];
  u32 fourCC      = Header[20];

  Result.Dim.x = (s32)width;
  Result.Dim.y = (s32)height;

  u32 BufferSize = mipMapCount > 1 ? linearSize * 2 : linearSize;
  u8* Buffer = Allocate(u8, Arena, BufferSize);

  ReadBytes(Buffer, BufferSize, TextureFile);

  s32 Closed = fclose(TextureFile);
  Assert(Closed == 0);

  unsigned int format;

  switch(fourCC)
  {
  case FOURCC_DXT1:
    format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    break;
  case FOURCC_DXT3:
    format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    break;
  case FOURCC_DXT5:
    format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    break;
  default:
    return Result;
  }

  s32 offset = 0;

  glGenTextures(1, &Result.ID);

  glBindTexture(GL_TEXTURE_2D, Result.ID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


  s32 blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
  for (s32 level = 0; level < mipMapCount && (width || height); ++level)
  {
    s32 size = ((width+3)/4)*((height+3)/4)*blockSize;
    glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
      0, size, Buffer + offset);

    offset += size;
    width  /= 2;
    height /= 2;

    // Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
    if(width < 1) width = 1;
    if(height < 1) height = 1;

  }

  return Result;
}
#endif

link_internal texture
InitTexture(v2i Dim, cs DebugName, texture_storage_format Format, u32 Channels, u32 Slices, b32 IsDepthTexture )
{
  texture Result = {};

  Result.Dim = Dim;
  Result.DebugName = DebugName;
  Result.Channels = Channels;
  Result.Slices = Slices;
  Result.IsDepthTexture = IsDepthTexture;
  Result.Format = Format;

  return Result;
}

link_internal texture
GenTexture(v2i Dim, cs DebugName, texture_storage_format StorageFormat, u32 Channels, u32 Slices, b32 IsDepthTexture = False)
{
  Assert(Slices);
  if (IsDepthTexture) { Assert(Channels == 1); }

  u32 TextureDimensionality = Slices > 1 ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;

  texture Result = InitTexture(Dim, DebugName, StorageFormat, Channels, Slices, IsDepthTexture);

  GetGL()->GenTextures(1, &Result.ID);
  GetGL()->BindTexture(TextureDimensionality, Result.ID);

  bonsai_stdlib *Stdlib = GetStdlib();
  Stdlib->AllTextures.Memory = &Global_PermMemory;
  Push(&GetStdlib()->AllTextures, &Result);

  // Note(Jesse): This is required to be set if mipmapping is off.  The default
  // behavior is to lerp between the two closest mipmap levels, and when there
  // is only one level that fails, at least on my GL implementation.
  GetGL()->TexParameteri(TextureDimensionality, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //

  GetGL()->TexParameteri(TextureDimensionality, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  GetGL()->TexParameteri(TextureDimensionality, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GetGL()->TexParameteri(TextureDimensionality, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GetGL()->TexParameteri(TextureDimensionality, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  /* glTexParameteri(TextureDimensionality, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE); */
  GetGL()->TexParameteri(TextureDimensionality, GL_TEXTURE_COMPARE_MODE, GL_NONE);

  AssertNoGlErrors;

  return Result;
}

link_internal void
DeleteTexture(texture *Texture)
{
  /* if (GetStdlib) { RemoveUnordered(&GetStdlib()->AllTextures, Texture); } */

  GetGL()->DeleteTextures(1, &Texture->ID);
  Clear(Texture);
}

link_internal texture
MakeTexture_RGBA(    v2i  Dim,
                     u32 *Data,
                      cs  DebugName,
                     u32  Slices = 1,
  texture_storage_format  StorageFormat = TextureStorageFormat_RGBA8)
{
  Assert(Slices);

  u32 Channels = 4;
  texture Result = GenTexture(Dim, DebugName, StorageFormat, Channels, Slices);

  u32 InternalFormat = TextureStorageFormat_RGBA8;
  u32 TextureFormat = GL_RGBA;
  u32 ElementType = GL_UNSIGNED_BYTE;
  if (Slices == 1)
  {
    Assert(Data == 0); // Unsupported ..??  Shouldn't this just work ..?
    GetGL()->TexImage2D(GL_TEXTURE_2D, 0, (s32)InternalFormat,
        Result.Dim.x, Result.Dim.y, 0, TextureFormat, ElementType, Data);
  }
  else
  {
#if 1
    // TODO(Jesse, id: 137, tags: robustness, open_question): This _should_ be
    // able to be glTexImage3D, but the driver is throwing an error .. why?!
    //
    // UPDATE(Jesse): Looks like this is working on my INTEL/NVIDIA laptop, so
    // I'm going to switch back for now..
     GetGL()->TexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        s32(InternalFormat),
        Result.Dim.x, Result.Dim.y, s32(Slices),
        0,
        TextureFormat,
        ElementType,
        0);
#else

    s32 Mips = 0; //(s32)Slices;
    GetGL()->TexStorage3D(GL_TEXTURE_2D_ARRAY, Mips, InternalFormat,
                    Dim.x, Dim.x, (s32)Slices);

#endif

    s32 xOffset = 0;
    s32 yOffset = 0;
    s32 zOffset = 0;

    if (Data)
    {
      s32 TextureDepth = 1;
      GetGL()->TexSubImage3D( GL_TEXTURE_2D_ARRAY, 0,
                        xOffset, yOffset, zOffset,
                        Result.Dim.x, Result.Dim.y, TextureDepth,
                        TextureFormat, ElementType, Data);
    }
  }

  AssertNoGlErrors;

  return Result;
}

link_internal texture
MakeTexture_RGBA(    v2i  Dim,
                      v4 *Data,
                      cs  DebugName,
                     u32  Slices = 1,
  texture_storage_format  StorageFormat = TextureStorageFormat_RGBA32F)
{
  // @only_support_one_slice_allocate_texture
  Assert(Slices == 1);

  u32 Channels = 4;
  texture Result = GenTexture(Dim, DebugName, StorageFormat, Channels, Slices);

  s32 InternalFormat = StorageFormat;
  u32 TextureFormat = GL_RGBA;
  GetGL()->TexImage2D(GL_TEXTURE_2D, 0, InternalFormat,
      Result.Dim.x, Result.Dim.y, 0,  TextureFormat, GL_FLOAT, Data);

  GetGL()->BindTexture(GL_TEXTURE_2D, 0);

  return Result;
}

link_internal texture
MakeTexture_SingleChannel( v2i  Dim,
                     const f32 *Data,
                            cs  DebugName,
                           b32  IsDepthTexture, 
        texture_storage_format  StorageFormat = TextureStorageFormat_R32F)
{
  u32 Channels = 1;
  u32 Slices = 1;
  texture Result = GenTexture(Dim, DebugName, StorageFormat, Channels, Slices, IsDepthTexture);

  GetGL()->TexImage2D(GL_TEXTURE_2D, 0, StorageFormat,
      Result.Dim.x, Result.Dim.y, 0,  GL_RED, GL_FLOAT, Data);

  GetGL()->BindTexture(GL_TEXTURE_2D, 0);

  return Result;
}

link_internal texture
MakeTexture_RGB(     v2i  Dim,
                const v3 *Data,
                      cs  DebugName,
                     u32  Slices = 1,
  texture_storage_format  StorageFormat = TextureStorageFormat_RGB32F)
{
  // NOTE(Jesse): Only support a single slice through this path at the moment
  // @only_support_one_slice_allocate_texture
  Assert(Slices == 1);

  u32 Channels = 3;
  texture Result = GenTexture(Dim, DebugName, StorageFormat, Channels, Slices);

  GetGL()->TexImage2D(GL_TEXTURE_2D, 0, StorageFormat,
      Result.Dim.x, Result.Dim.y, 0,  GL_RGB, GL_FLOAT, Data);

  GetGL()->BindTexture(GL_TEXTURE_2D, 0);

  AssertNoGlErrors;

  return Result;
}

link_internal texture
MakeDepthTexture(v2i Dim, cs DebugName)
{
  u32 Channels = 1;
  u32 Slices = 1;
  b32 IsDepthTexture = True;
  texture_storage_format StorageFormat = TextureStorageFormat_Depth32;
  texture Result = GenTexture(Dim, DebugName, StorageFormat, Channels, Slices, IsDepthTexture);

  GetGL()->TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
    Result.Dim.x, Result.Dim.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  GetGL()->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  GetGL()->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  r32 BorderColors[4] = {1, 1, 1, 1};
  GetGL()->TexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColors);

  Result.IsDepthTexture = True;

  return Result;
}


















link_internal void
FramebufferDepthTexture(texture *Tex)
{
  GetGL()->FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Tex->ID, 0);
}

link_internal void
FramebufferTexture(framebuffer *FBO, texture *Tex)
{
  Assert(Tex->ID != INVALID_TEXTURE_HANDLE);
  u32 Attachment = FBO->Attachments++;
  GetGL()->FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, GL_TEXTURE_2D, Tex->ID, 0);
}

link_internal texture
LoadBitmap(const char* FilePath, u32 SliceCount, memory_arena *Arena)
{
  bitmap TexBitmap = ReadBitmapFromDisk(FilePath, Arena);
  texture Result = MakeTexture_RGBA(TexBitmap.Dim, TexBitmap.Pixels.Start, CS(FilePath), SliceCount);
  return Result;
}

link_internal texture
LoadBitmap(const char* FilePath, memory_arena *Arena)
{
  bitmap TexBitmap = ReadBitmapFromDisk(FilePath, Arena);
  texture Result = MakeTexture_RGBA(TexBitmap.Dim, TexBitmap.Pixels.Start, CS(FilePath));
  return Result;
}

link_internal b32
LoadBitmap(const char* FilePath, memory_arena *Arena, texture* Dest, s32 Slice)
{
  bitmap TexBitmap = ReadBitmapFromDisk(FilePath, Arena);
  Assert(TexBitmap.Dim == Dest->Dim);

  b32 Result = False;
  if (TexBitmap.Pixels.Start)
  {
    Result = True;
    u32 TextureFormat = GL_RGBA;
    u32 ElementType = GL_UNSIGNED_BYTE;
    s32 TextureDepth = 1;

    GetGL()->TexSubImage3D(
        GL_TEXTURE_2D_ARRAY,
        0, // mip level

        0,     // x offset
        0,     // y offset
        Slice, // z offset

        Dest->Dim.x, Dest->Dim.y, TextureDepth,

        TextureFormat, ElementType, TexBitmap.Pixels.Start);
  }

  return Result;
}

link_internal bitmap
LoadBitmap(file_traversal_node *Node, memory_arena *Arena)
{
  cs Path = EndsWith(Node->Dir, CSz("/")) ?
             Concat(Node->Dir, Node->Name, GetTranArena(), 1) :
             Concat(Node->Dir, CSz("/"), Node->Name, GetTranArena(), 1);

  bitmap Result = ReadBitmapFromDisk((const char*)Path.Start, Arena);
  return Result;
}

link_internal maybe_file_traversal_node
LoadBitmapFileTraversalHelper(file_traversal_node Node, u64 UserData)
{
  if (Node.Type == FileTraversalType_File)
  {
    bitmap_block_array *Bitmaps = Cast(bitmap_block_array*, UserData);

    bitmap B = LoadBitmap(&Node, GetTranArena());
    Push(Bitmaps, &B);
  }

  maybe_file_traversal_node Result = {};
  return Result;
}

link_internal void
LoadBitmapsFromFolderUnordered(cs FilePath, bitmap_block_array *Bitmaps)
{
  PlatformTraverseDirectoryTreeUnordered(FilePath, LoadBitmapFileTraversalHelper, u64(Bitmaps));
}

link_internal void
LoadBitmapsFromFolderOrdered(cs FilePath, bitmap_block_array *Bitmaps, memory_arena *BitmapMemory, memory_arena *DatastructureMemory)
{
  file_traversal_node_block_array FileNodes =
    GetLexicographicallySortedListOfFilesInDirectory(FilePath, DatastructureMemory);

  IterateOver(&FileNodes, FileNode, FileNodeIndex)
  {
    bitmap B = LoadBitmap(FileNode, BitmapMemory);
    Push(Bitmaps, &B);
  }
}

link_internal texture
CreateTextureArrayFromBitmapBlockArray(bitmap_block_array *Bitmaps, v2i TextureArrayXY)
{
  umm BitmapCount = TotalElements(Bitmaps);
  texture Result = MakeTexture_RGBA(TextureArrayXY, Cast(u32*, 0), CSz("TODO: DebugName"), u32(BitmapCount));

  s32 TextureDepth = 1;

  u32 InternalFormat = GL_RGBA8;
  u32 TextureFormat = GL_RGBA;
  u32 ElementType = GL_UNSIGNED_BYTE;

  IterateOver(Bitmaps, Bitmap, BitmapIndex)
  {
    s32 zOffset = s32(GetIndex(&BitmapIndex));

    s32 xOffset = (TextureArrayXY.x - Bitmap->Dim.x) / 2;
    s32 yOffset = (TextureArrayXY.y - Bitmap->Dim.y) / 2;
    GetGL()->TexSubImage3D( GL_TEXTURE_2D_ARRAY, 0,
                      xOffset, yOffset, zOffset,
                      Bitmap->Dim.x, Bitmap->Dim.y,
                      TextureDepth,
                      TextureFormat,
                      ElementType,
                      Cast(void*, Bitmap->Pixels.Start) );
  }

  return Result;
}

link_internal texture
CreateTextureArrayFromBitmapBufferArray(bitmap_buffer *BitmapBuffers, u32 BufferCount, v2i TextureArrayXY)
{
  umm BitmapCount = 0;
  RangeIterator_t(u32, BufferIndex, BufferCount)
  {
    bitmap_buffer *Bitmaps = BitmapBuffers + BufferIndex;
    BitmapCount += TotalElements(Bitmaps);
  }


  texture Result = MakeTexture_RGBA(TextureArrayXY, Cast(u32*, 0), CSz("TODO: DebugName"), u32(BitmapCount));

  s32 zOffset = 0;
  RangeIterator_t(u32, BufferIndex, BufferCount)
  {
    bitmap_buffer *Bitmaps = BitmapBuffers + BufferIndex;

    s32 TextureDepth = 1;

    u32 InternalFormat = GL_RGBA8;
    u32 TextureFormat = GL_RGBA;
    u32 ElementType = GL_UNSIGNED_BYTE;

    IterateOver(Bitmaps, Bitmap, BitmapIndex)
    {

      s32 xOffset = (TextureArrayXY.x - Bitmap->Dim.x) / 2;
      s32 yOffset = (TextureArrayXY.y - Bitmap->Dim.y) / 2;
      GetGL()->TexSubImage3D( GL_TEXTURE_2D_ARRAY, 0,
                        xOffset, yOffset, zOffset,
                        Bitmap->Dim.x, Bitmap->Dim.y,
                        TextureDepth,
                        TextureFormat,
                        ElementType,
                        Cast(void*, Bitmap->Pixels.Start) );
      zOffset += 1;
    }
  }

  return Result;
}
