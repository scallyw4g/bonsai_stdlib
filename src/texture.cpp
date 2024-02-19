
/* poof(block_array_c(texture, {8})) */
/* #include <generated/block_array_c_texture_688856411.h> */

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
GenTexture(v2i Dim, cs DebugName, u32 TextureDimensionality = GL_TEXTURE_2D)
{
  texture Result = {};

  Result.Dim = Dim;
  Result.DebugName = DebugName;

  GL.GenTextures(1, &Result.ID);
  GL.BindTexture(TextureDimensionality, Result.ID);

  // @texture_block_array
  /* if (GetStdlib) { Push(&GetStdlib()->AllTextures, &Result); } */

  // Note(Jesse): This is required to be set if mipmapping is off.  The default
  // behavior is to lerp between the two closest mipmap levels, and when there
  // is only one level that fails, at least on my GL implementation.
  GL.TexParameteri(TextureDimensionality, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //

  GL.TexParameteri(TextureDimensionality, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  GL.TexParameteri(TextureDimensionality, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GL.TexParameteri(TextureDimensionality, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GL.TexParameteri(TextureDimensionality, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  /* glTexParameteri(TextureDimensionality, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE); */
  GL.TexParameteri(TextureDimensionality, GL_TEXTURE_COMPARE_MODE, GL_NONE);

  AssertNoGlErrors;

  return Result;
}

link_internal texture
MakeTexture_RGBA(v2i Dim, u32 *Data, cs DebugName, u32 MaxTextureSlices = 1)
{
  Assert(MaxTextureSlices);

  b32 Multidimensional = MaxTextureSlices > 1;

  u32 TextureDimensionality = Multidimensional ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
  texture Result = GenTexture(Dim, DebugName, TextureDimensionality);
  Result.Channels = 4;

  u32 InternalFormat = GL_RGBA8;
  u32 TextureFormat = GL_RGBA;
  u32 ElementType = GL_UNSIGNED_BYTE;
  if (MaxTextureSlices == 1)
  {
    Assert(Data == 0); // Unsupported
    GL.TexImage2D(GL_TEXTURE_2D, 0, (s32)InternalFormat,
        Result.Dim.x, Result.Dim.y, 0, TextureFormat, ElementType, Data);
  }
  else
  {
    Result.Slices = MaxTextureSlices;
    // TODO(Jesse, id: 137, tags: robustness, open_question): This _should_ be
    // able to be glTexImage3D, but the driver is throwing an error .. why?!
    //
#if 1
    GL.TexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        s32(InternalFormat),
        Result.Dim.x, Result.Dim.y, s32(MaxTextureSlices),
        0,
        TextureFormat,
        ElementType,
        0);
#else

    s32 Mips = 0; //(s32)MaxTextureSlices;
    GL.TexStorage3D(GL_TEXTURE_2D_ARRAY, Mips, InternalFormat,
                    Dim.x, Dim.x, (s32)MaxTextureSlices);

#endif

    s32 xOffset = 0;
    s32 yOffset = 0;
    s32 zOffset = 0;

    if (Data)
    {
      s32 TextureDepth = 1;
      GL.TexSubImage3D( GL_TEXTURE_2D_ARRAY, 0,
                        xOffset, yOffset, zOffset,
                        Result.Dim.x, Result.Dim.y, TextureDepth,
                        TextureFormat, ElementType, Data );
    }
  }

/*   GL.BindTexture(TextureDimensionality, 0); */
  AssertNoGlErrors;

  return Result;
}

link_internal texture
MakeTexture_RGBA(v2i Dim, v4 *Data, cs DebugName)
{
  texture Texture = GenTexture(Dim, DebugName);
  Texture.Channels = 4;

  u32 TextureFormat = GL_RGBA;
  s32 InternalFormat = GL_RGBA32F;
  u32 ElementType = GL_FLOAT;
  GL.TexImage2D(GL_TEXTURE_2D, 0, InternalFormat,
      Texture.Dim.x, Texture.Dim.y, 0,  TextureFormat, ElementType, Data);

  GL.BindTexture(GL_TEXTURE_2D, 0);

  return Texture;
}

link_internal texture
MakeTexture_SingleChannel(v2i Dim, cs DebugName)
{
  texture Texture = GenTexture(Dim, DebugName);
  Texture.Channels = 1;

  GL.TexImage2D(GL_TEXTURE_2D, 0, GL_R32F,
      Texture.Dim.x, Texture.Dim.y, 0,  GL_RED, GL_FLOAT, 0);

  GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GL.BindTexture(GL_TEXTURE_2D, 0);

  return Texture;
}

link_internal texture
MakeTexture_RGB(v2i Dim, const v3 *Data, cs DebugName)
{
  texture Texture = GenTexture(Dim, DebugName);
  Texture.Channels = 3;

  /* TODO(Jesse, id: 138, tags: opengl, memory_consumption): 32F is only
   * necessary for reprojection of Position for calculating AO.  Consider
   * passing this in when creating a Texture?
   */

  GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
      Texture.Dim.x, Texture.Dim.y, 0,  GL_RGB, GL_FLOAT, Data);

  GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GL.BindTexture(GL_TEXTURE_2D, 0);

  AssertNoGlErrors;

  return Texture;
}

link_internal texture
MakeDepthTexture(v2i Dim, cs DebugName)
{
  texture Texture = GenTexture(Dim, DebugName);
  Texture.Channels = 1;

  GL.TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
    Texture.Dim.x, Texture.Dim.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  GL.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  r32 BorderColors[4] = {1, 1, 1, 1};
  GL.TexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColors);

  /* GL.BindTexture(GL_TEXTURE_2D, 0); */
  Texture.IsDepthTexture = True;

  return Texture;
}

link_internal void
FramebufferDepthTexture(texture *Tex)
{
  GL.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Tex->ID, 0);
}

link_internal void
FramebufferTexture(framebuffer *FBO, texture *Tex)
{
  u32 Attachment = FBO->Attachments++;
  GL.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + Attachment, GL_TEXTURE_2D, Tex->ID, 0);
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

link_internal bitmap
LoadBitmap(file_traversal_node *Node, memory_arena *Arena)
{
  cs Path = Concat(Node->Dir, CSz("/"), Node->Name, GetTranArena(), 1);
  bitmap Result = ReadBitmapFromDisk((const char*)Path.Start, Arena);
  return Result;
}

link_internal maybe_file_traversal_node
LoadBitmapFileHelper(file_traversal_node Node, u64 UserData)
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
LoadBitmapsFromFolder(cs FilePath, bitmap_block_array *Bitmaps)
{
  PlatformTraverseDirectoryTree(FilePath, LoadBitmapFileHelper, u64(Bitmaps));
}

link_internal texture
CreateTextureArrayFromBitmapArray(bitmap_block_array *Bitmaps, v2i TextureArrayXY)
{
  umm BitmapCount = TotalElements(Bitmaps);
  texture Result = MakeTexture_RGBA(TextureArrayXY, 0, CSz("TODO: DebugName"), u32(BitmapCount));

  s32 TextureDepth = 1;

  u32 InternalFormat = GL_RGBA8;
  u32 TextureFormat = GL_RGBA;
  u32 ElementType = GL_UNSIGNED_BYTE;

  IterateOver(Bitmaps, Bitmap, BitmapIndex)
  {
    s32 zOffset = s32(GetIndex(&BitmapIndex));

    s32 xOffset = (TextureArrayXY.x - Bitmap->Dim.x) / 2;
    s32 yOffset = (TextureArrayXY.y - Bitmap->Dim.y) / 2;
    GL.TexSubImage3D( GL_TEXTURE_2D_ARRAY, 0,
                      xOffset, yOffset, zOffset,
                      Bitmap->Dim.x, Bitmap->Dim.y,
                      TextureDepth,
                      TextureFormat,
                      ElementType,
                      Cast(void*, Bitmap->Pixels.Start) );
  }

  return Result;
}
